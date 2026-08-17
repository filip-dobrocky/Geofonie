#!/usr/bin/env python3
"""Build a generic OTA image and distribute it over the Geofonie mesh.

Builds the given PlatformIO env of the chosen project (roto or acid), then
uploads .pio/build/<env>/firmware.bin to the OTA distributor of whichever
node's softAP this machine is joined to (the gateway 10.x.y.1, auto-detected).
That node stores the image, offers it to the mesh, and self-flashes last.

Role matching ("roto" / "acid") means a Roto image can never flash an Acid
node, even though both live on the same mesh -- but you do have to run this
once per project.

Verify the rollout by watching the fw version in /fromRoto/ping (Max
read_info.maxpat), or with --watch (polls the distributor's /status).

Examples:
  python tools/ota_update.py --project roto              # build deploy image + upload
  python tools/ota_update.py --project acid --env xiao_ota --watch
  python tools/ota_update.py --project roto --skip-build --ip 10.226.71.1
"""

import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import time
import urllib.error
import urllib.request
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PROJECTS = {"roto": "ESPRoto", "acid": "ESPAcid"}


def find_pio():
    pio = shutil.which("pio")
    if pio:
        return pio
    suffix = "Scripts/pio.exe" if sys.platform == "win32" else "bin/pio"
    candidate = Path.home() / ".platformio" / "penv" / suffix
    if candidate.exists():
        return str(candidate)
    sys.exit("error: pio not found (PATH or ~/.platformio/penv)")


def find_node_ip():
    """The node is always .1 of the softAP subnet (painlessMesh gateway)."""
    if sys.platform == "win32":
        cmd = [
            "powershell", "-NoProfile", "-Command",
            "(Get-NetIPConfiguration | Where-Object "
            "{ $_.IPv4DefaultGateway.NextHop -like '10.*' } | "
            "Select-Object -First 1).IPv4DefaultGateway.NextHop",
        ]
    elif sys.platform == "darwin":
        cmd = ["route", "-n", "get", "default"]
    else:
        cmd = ["ip", "route", "show", "default"]
    try:
        out = subprocess.run(cmd, capture_output=True, text=True, timeout=10).stdout
    except OSError:
        out = ""
    match = re.search(r"\b(10\.\d+\.\d+\.\d+)\b", out)
    if not match:
        sys.exit("error: no 10.x.y.1 gateway found -- is Wi-Fi joined to a "
                 "node's softAP? (or pass --ip)")
    return match.group(1)


def upload(ip, bin_path, timeout=120):
    """POST firmware.bin as multipart/form-data (stdlib only)."""
    boundary = uuid.uuid4().hex
    head = (f"--{boundary}\r\n"
            f'Content-Disposition: form-data; name="fw"; filename="firmware.bin"\r\n'
            f"Content-Type: application/octet-stream\r\n\r\n").encode()
    tail = f"\r\n--{boundary}--\r\n".encode()
    body = head + bin_path.read_bytes() + tail
    req = urllib.request.Request(
        f"http://{ip}/fw", data=body,
        headers={"Content-Type": f"multipart/form-data; boundary={boundary}"})
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return resp.read().decode(errors="replace")


def get_status(ip):
    with urllib.request.urlopen(f"http://{ip}/status", timeout=5) as resp:
        return json.load(resp)


def watch(ip, md5):
    print(f">> watching http://{ip}/status (Ctrl+C to stop; "
          f"'abort' via http://{ip}/abort)")
    down = 0
    while True:
        time.sleep(15)
        try:
            s = get_status(ip)
        except (urllib.error.URLError, OSError, json.JSONDecodeError):
            down += 1
            print("   node unreachable (rebooting into the new image?)")
            # Self-flash reboots the AP; if it stays down, this machine has to
            # rejoin the AP anyway before /status is reachable again.
            if down >= 8:
                print(">> giving up polling; rejoin the AP and check /status")
                return
            continue
        down = 0
        print(f"   offering={s['offering']} done={s['nodes_done']}/"
              f"{s['mesh_nodes']} elapsed={s['elapsed_s']}s "
              f"running_md5={s['running_md5']}")
        if not s["offering"]:
            if s["running_md5"] == md5:
                print(">> distributor is running the new image -- rollout complete")
            else:
                print(">> offer ended (self-flash pending reboot, or aborted)")
            return


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--project", required=True, choices=sorted(PROJECTS),
                    help="which firmware to distribute")
    ap.add_argument("--env", default="xiao_deploy",
                    help="PlatformIO env to build/upload (default: %(default)s)")
    ap.add_argument("--ip", help="node AP IP (default: auto-detect 10.x.y.1 gateway)")
    ap.add_argument("--skip-build", action="store_true",
                    help="upload the existing firmware.bin without rebuilding")
    ap.add_argument("--watch", action="store_true",
                    help="poll /status until the distributor self-flashes")
    args = ap.parse_args()

    if not re.search(r"ota|deploy", args.env):
        sys.exit(f"error: '{args.env}' is not a generic image env. Per-node "
                 "(dev_N) or bench images must NOT be distributed via OTA -- "
                 "they would overwrite every node's id with the same one.")

    project_dir = ROOT / PROJECTS[args.project]

    if not args.skip_build:
        pio = find_pio()
        print(f">> pio run -e {args.env} -d {project_dir}")
        if subprocess.run([pio, "run", "-e", args.env, "-d", str(project_dir)]).returncode:
            sys.exit("error: build failed")

    bin_path = project_dir / ".pio" / "build" / args.env / "firmware.bin"
    if not bin_path.exists():
        sys.exit(f"error: missing {bin_path}")

    md5 = hashlib.md5(bin_path.read_bytes()).hexdigest()
    print(f">> image: {bin_path} ({bin_path.stat().st_size:,} bytes, md5 {md5})")

    ip = args.ip or find_node_ip()
    print(f">> uploading to http://{ip}/fw")
    try:
        reply = upload(ip, bin_path)
    except (urllib.error.URLError, OSError) as e:
        sys.exit(f"error: upload failed: {e}")
    print(reply.strip())

    if md5 not in reply:
        sys.exit(f"error: distributor md5 does not match local image md5 "
                 f"({md5}) -- check the upload")
    # The distributor only offers to nodes running its own role, so uploading a
    # roto image to an acid node's AP silently reaches nobody.
    if f"role={args.project}" not in reply:
        sys.exit(f"error: this node distributes a different role than "
                 f"'{args.project}' -- join a {args.project} node's softAP")
    print(f">> md5 confirmed; mesh distribution started (role={args.project})")
    print(">> track per-node progress via the fw version in ping telemetry")

    if args.watch:
        watch(ip, md5)


if __name__ == "__main__":
    main()
