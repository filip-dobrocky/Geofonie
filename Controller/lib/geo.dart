import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:network_info_plus/network_info_plus.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:wifi_iot/wifi_iot.dart';

import 'osc.dart';

// Ports and mesh credentials from PlatformIO/common/NetworkConfig.h.
const cmdPort = 54345;
const infoPort = 54355;
const staleAfter = Duration(seconds: 15);
const nodeIds = [0, 1, 2, 3, 4];

// Servo range from ESPRoto.cpp, to turn a reported center angle back into 0..1.
const _minAngle = 40, _maxAngle = 99;

/// One 0..1 parameter. Each node gets `path [id, v]`; the GLOBAL row sends
/// the `/global/` form `[v]`, except where noted.
class Param {
  const Param(this.label, this.path, [this.def = 0, this.hint]);
  final String label, path;
  final double def;
  final String? hint;
}

const speed = Param('SPEED', '/toRoto/rotation/speed');
// Fine speed offset (0..0.05) added to SPEED; never sent on its own.
const fine = Param('fine', '/toRoto/rotation/speed+');
const dir = Param('DIR', '/toRoto/rotation/direction');
const servo = Param('SERVO', '/toRoto/servo/center', 0.5); // no global form
const auto = Param('AUTO', '/toRoto/auto'); // global only: /toRoto/global/auto
const minDist = Param('MIN DIST', '/toRoto/calibration/minDist', 0);
const maxDist = Param('MAX DIST', '/toRoto/calibration/maxDist', 1);

// Defaults: misc_osc[] in ESPRoto.cpp.
const misc = [
  Param('VOL', '/toRoto/misc/1', 1),
  Param('JAS', '/toRoto/misc/2', 0, '0–.5 auto · .5–1 manual'),
  Param('ENGINE', '/toRoto/misc/3', 0, '0 granular · .5 sine · 1 reso'),
  Param('ROOT', '/toRoto/misc/4', 0, 'MIDI 36–84'),
  Param('SCALE', '/toRoto/misc/5'),
  Param('RESO', '/toRoto/misc/6', 0, '0 −fb · .5 off · 1 +fb'),
  Param('FM RATIO', '/toRoto/misc/7', 0.1875),
  Param('FM DEPTH', '/toRoto/misc/8', 0.1),
];

// Defaults: osc_params[] in ESPAcid.cpp.
const acid = [
  Param('PIEZO IN', '/toAcid/param1', 1),
  Param('par2', '/toAcid/param2'),
  Param('par3', '/toAcid/param3'),
  Param('par4', '/toAcid/param4'),
  Param('par5', '/toAcid/param5'),
  Param('par6', '/toAcid/param6'),
  Param('par7', '/toAcid/param7'),
];

final allParams = [speed, fine, dir, servo, auto, minDist, maxDist, ...misc, ...acid];

/// '/toRoto/misc/4' -> '/toRoto/global/misc/4'
String globalPath(String path) {
  final i = path.indexOf('/', 1);
  return '${path.substring(0, i)}/global${path.substring(i)}';
}

/// One Message line for PlatformIO/common/Score.h. id < 0 = global.
String scoreLine(String path, int id, double v, String label) =>
    '{"${id < 0 ? globalPath(path) : path}", $id, '
    'CONST(${v.clamp(0.0, 1.0).toStringAsFixed(3)}f), 100}, // ${label.toLowerCase()}';

/// MIDI note shown for ROOT: 0..1 maps to 36..84.
String midiNote(double v) {
  const names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
  final n = (36 + v * 48).round();
  return '${names[n % 12]}${n ~/ 12 - 1}';
}

class NodeInfo {
  DateTime? lastPing;
  int? fw;
  double? reading;

  bool get alive =>
      lastPing != null && DateTime.now().difference(lastPing!) < staleAfter;
}

class Geo extends ChangeNotifier {
  final roto = {for (final id in nodeIds) id: NodeInfo()};
  final acidInfo = {for (final id in nodeIds) id: NodeInfo()};
  String ip = '10.0.0.1';
  String status = 'starting…';
  bool busy = false;

  // Settings (persisted).
  String ssid = 'TrychtyrLOM', pass = 'LomLomLom', manualIp = '';
  int rampMs = 0; // Max msRamp: SPEED and SERVO glide over this

  late SharedPreferences _prefs;
  RawDatagramSocket? _sock;

  // --- parameter state --------------------------------------------------------
  // Keyed '<path> <id>', id -1 = the GLOBAL row.
  final _values = <String, double>{};
  final _dirty = <String>{}; // insertion-ordered
  final _lastSent = <String, double>{};
  final _ramps = <String, (double from, DateTime t0)>{};

  double get(Param p, int id) => _values['${p.path} $id'] ?? p.def;

  /// Value actually going out: SPEED carries its fine offset.
  double out(Param p, int id) =>
      p == speed ? get(speed, id) + get(fine, id) : get(p, id);

  bool get live => [...roto.values, ...acidInfo.values].any((n) => n.alive);

  void set(Param p, int id, double v) {
    if (id < 0) return _setGlobal(p, v);
    _values['${p.path} $id'] = v;
    _mark(p == fine ? speed : p, id);
    notifyListeners();
  }

  // Like Max: a global control goes as one /global/ packet and every node's
  // control follows silently. One packet can't carry per-node fine offsets,
  // so a global SPEED/fine move zeroes them.
  void _setGlobal(Param p, double v) {
    _values['${p.path} -1'] = v;
    final q = p == fine ? speed : p;
    final s = out(q, -1);
    for (final id in nodeIds) {
      if (q == speed) _values['${fine.path} $id'] = 0;
      _values['${q.path} $id'] = s;
      _lastSent['${q.path} $id'] = s;
    }
    _mark(q, -1);
    notifyListeners();
  }

  /// SPEED 0 everywhere, fine offsets included (else nodes keep creeping).
  void stop() {
    _values['${fine.path} -1'] = 0;
    _setGlobal(speed, 0);
  }

  void _mark(Param p, int id) {
    final k = '${p.path} $id';
    if ((p == speed || p == servo) && rampMs > 0) {
      _ramps[k] = (_lastSent[k] ?? p.def, DateTime.now()); // never sent: firmware default
    } else {
      _ramps.remove(k);
    }
    _dirty.add(k);
  }

  // ponytail: every node packet is rebroadcast over the whole mesh, so cap
  // packets per tick; raise if preset recalls feel slow.
  static const _maxPerTick = 8;

  void _flush() {
    final now = DateTime.now();
    for (final k in _dirty.take(_maxPerTick).toList()) {
      _dirty.remove(k);
      final sp = k.lastIndexOf(' ');
      final path = k.substring(0, sp);
      final id = int.parse(k.substring(sp + 1));
      final p = allParams.firstWhere((p) => p.path == path);
      var v = out(p, id);
      final r = _ramps[k];
      if (r != null) {
        final f = now.difference(r.$2).inMilliseconds / rampMs;
        if (f < 1) {
          v = r.$1 + (v - r.$1) * f;
          _dirty.add(k); // keep gliding, back of the queue
        } else {
          _ramps.remove(k);
        }
      }
      _lastSent[k] = v;
      if (id < 0) {
        send(globalPath(path), [v]);
      } else {
        send(path, [id.toDouble(), v]);
      }
    }
  }

  void calibrate(int id) => id < 0
      ? send('/toRoto/global/calibration/auto', [1.0])
      : send('/toRoto/calibration/auto', [id.toDouble(), 1.0]);

  // --- presets ------------------------------------------------------------------
  bool hasPreset(int slot) => _prefs.containsKey('preset_$slot');

  Future<void> savePreset(int slot) async {
    await _prefs.setString('preset_$slot', jsonEncode(_values));
    notifyListeners();
  }

  void recallPreset(int slot) {
    final s = _prefs.getString('preset_$slot');
    if (s == null) return;
    _values
      ..clear()
      ..addAll((jsonDecode(s) as Map).map((k, v) => MapEntry(k as String, (v as num).toDouble())));
    // Node values fully define the state; globals are only the UI row.
    for (final p in allParams) {
      if (p == fine || p == auto) continue;
      for (final id in nodeIds) {
        _mark(p, id);
      }
    }
    _mark(auto, -1);
    notifyListeners();
  }

  // --- lifecycle / settings -----------------------------------------------------
  Future<void> start() async {
    _prefs = await SharedPreferences.getInstance();
    ssid = _prefs.getString('ssid') ?? ssid;
    pass = _prefs.getString('pass') ?? pass;
    manualIp = _prefs.getString('manualIp') ?? '';
    rampMs = _prefs.getInt('rampMs') ?? 0;
    await _bind();
    findNode(); // slow on Windows (powershell); UI shows status meanwhile
    Timer.periodic(const Duration(milliseconds: 50), (_) => _flush());
    // Repaint ping freshness even when nothing arrives.
    Timer.periodic(const Duration(seconds: 1), (_) => notifyListeners());
  }

  void setRamp(int ms) {
    rampMs = ms;
    _prefs.setInt('rampMs', ms);
    notifyListeners();
  }

  Future<void> saveSettings(
      {required String ssid, required String pass, required String manualIp}) async {
    this.ssid = ssid;
    this.pass = pass;
    this.manualIp = manualIp;
    await _prefs.setString('ssid', ssid);
    await _prefs.setString('pass', pass);
    await _prefs.setString('manualIp', manualIp);
    await findNode();
  }

  // --- socket -------------------------------------------------------------------
  // One socket on the telemetry port: receives /fromRoto|/fromAcid broadcasts
  // and sends commands. Rebound after Wi-Fi changes or any socket error, so a
  // network drop never leaves the app silently deaf.
  // Shared future: overlapping callers must not each open (and leak) a socket.
  Future<void>? _binding;
  Future<void> _bind() => _binding ??= _doBind().whenComplete(() => _binding = null);

  Future<void> _doBind() async {
    _sock?.close();
    _sock = null;
    try {
      final s = await RawDatagramSocket.bind(InternetAddress.anyIPv4, infoPort,
          reuseAddress: true);
      s.listen((e) {
        if (e != RawSocketEvent.read) return;
        final dg = s.receive();
        if (dg != null) _onPacket(dg.data);
      }, onError: (Object e) {
        _setStatus('socket error: $e');
        if (_sock == s) _sock = null;
      }, onDone: () {
        if (_sock == s) _sock = null;
      });
      _sock = s;
    } catch (e) {
      _setStatus('cannot bind :$infoPort — $e');
    }
  }

  // The firmware sends every arg as a float.
  void _onPacket(Uint8List data) {
    final msg = decodeOsc(data);
    if (msg == null) return;
    final (addr, args) = msg;
    if (args.length < 2 || args.any((a) => a is! num)) return;
    final a = [for (final x in args) (x as num).toDouble()];
    final id = a[0].round();
    final n = (addr.startsWith('/fromAcid') ? acidInfo : roto)[id];
    if (n == null) return;
    switch (addr) {
      case '/fromRoto/ping' || '/fromAcid/ping':
        n.lastPing = DateTime.now();
        n.fw = a[1].round();
      case '/fromRoto/reading':
        n.reading = a[1];
      case '/fromRoto/calibration' when a.length >= 4:
        // A finished scan: show what the object measured, don't send it back.
        for (final (p, v) in [
          (minDist, a[1]),
          (maxDist, a[2]),
          (servo, ((a[3] - _minAngle) / (_maxAngle - _minAngle)).clamp(0.0, 1.0)),
        ]) {
          _values['${p.path} $id'] = v;
          _lastSent['${p.path} $id'] = v;
        }
      default:
        return;
    }
    notifyListeners();
  }

  Future<void> send(String path, List<Object> args) async {
    if (_sock == null) await _bind();
    final s = _sock;
    if (s == null) return;
    try {
      final sent = s.send(encodeOsc(path, args), InternetAddress(ip), cmdPort);
      if (sent == 0) _setStatus('send failed (network down?)');
    } catch (e) {
      // Usually just no route (not on the mesh); the socket itself is fine.
      _setStatus('send error: $e');
    }
  }

  // --- network ------------------------------------------------------------------
  /// The node we talk to is the gateway of the softAP we joined
  /// (painlessMesh hands out 10.x.y.1). Returns true when an address is known.
  Future<bool> findNode() async {
    if (manualIp.isNotEmpty) {
      ip = manualIp;
      _setStatus('using manual IP $ip');
      return true;
    }
    String? gw;
    try {
      if (Platform.isWindows) {
        final r = await Process.run('powershell', [
          '-NoProfile',
          '-Command',
          "(Get-NetIPConfiguration | Where-Object { \$_.IPv4DefaultGateway.NextHop -like '10.*' } | Select-Object -First 1).IPv4DefaultGateway.NextHop",
        ]);
        gw = (r.stdout as String).trim();
      } else if (Platform.isLinux) {
        final r = await Process.run('sh', ['-c', "ip route | awk '/^default via 10\\./{print \$3; exit}'"]);
        gw = (r.stdout as String).trim();
      } else {
        gw = await NetworkInfo().getWifiGatewayIP();
      }
    } catch (e) {
      _setStatus('findNode: $e');
    }
    if (gw != null && gw.startsWith('10.')) {
      ip = gw;
      _setStatus('node at $ip');
      return true;
    } else {
      ip = '10.0.0.1';
      _setStatus('no 10.x.y.1 gateway — is Wi-Fi on $ssid? (got ${gw ?? '-'})');
      return false;
    }
  }

  Future<void> reconnectWifi() async {
    if (ssid.isEmpty) return _setStatus('set the mesh SSID in settings first');
    busy = true;
    _setStatus('joining $ssid…');
    try {
      if (Platform.isWindows) {
        await _windowsJoin();
      } else if (Platform.isAndroid) {
        await WiFiForIoTPlugin.forceWifiUsage(false);
        final ok = await WiFiForIoTPlugin.connect(ssid,
            password: pass,
            security: pass.isEmpty ? NetworkSecurity.NONE : NetworkSecurity.WPA,
            withInternet: false);
        if (!ok) throw 'connect refused';
        // The AP has no internet: pin this process's sockets to it.
        await WiFiForIoTPlugin.forceWifiUsage(true);
      } else {
        throw 'not supported on this platform — join manually';
      }
      // Wait for DHCP to hand us the 10.x.y.1 gateway.
      for (var i = 0; i < 15; i++) {
        await Future<void>.delayed(const Duration(seconds: 1));
        if (await findNode()) break;
      }
      await _bind();
    } catch (e) {
      _setStatus('Wi-Fi: $e');
    } finally {
      busy = false;
      notifyListeners();
    }
  }

  // netsh can only connect to a saved profile, so (re)write one first.
  Future<void> _windowsJoin() async {
    if (pass.isNotEmpty) {
      String x(String s) => s
          .replaceAll('&', '&amp;')
          .replaceAll('<', '&lt;')
          .replaceAll('>', '&gt;');
      final f = File('${Directory.systemTemp.path}\\geo_wlan.xml');
      await f.writeAsString('''<?xml version="1.0"?>
<WLANProfile xmlns="http://www.microsoft.com/networking/WLAN/profile/v1">
<name>${x(ssid)}</name>
<SSIDConfig><SSID><name>${x(ssid)}</name></SSID></SSIDConfig>
<connectionType>ESS</connectionType><connectionMode>manual</connectionMode>
<MSM><security>
<authEncryption><authentication>WPA2PSK</authentication><encryption>AES</encryption><useOneX>false</useOneX></authEncryption>
<sharedKey><keyType>passPhrase</keyType><protected>false</protected><keyMaterial>${x(pass)}</keyMaterial></sharedKey>
</security></MSM>
</WLANProfile>''');
      final r = await Process.run(
          'netsh', ['wlan', 'add', 'profile', 'filename=${f.path}', 'user=current']);
      await f.delete();
      if (r.exitCode != 0) throw (r.stdout as String).trim();
    }
    final r = await Process.run('netsh', ['wlan', 'connect', 'name=$ssid']);
    if (r.exitCode != 0) throw (r.stdout as String).trim();
  }

  void _setStatus(String s) {
    status = s;
    notifyListeners();
  }
}
