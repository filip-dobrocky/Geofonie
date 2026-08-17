#pragma once

namespace NetworkConfig {

    // Mesh SSID: every node advertises this as its own softAP, it is not an
    // infrastructure access point to associate with.
    const char* ssid     = "TrychtyrLOM";
    const char* password = "LomLomLom";

    const int mesh_port = 5555;
    const int max_conn = 10;
    const int mesh_channel = 1;

    const int osc_from_ctl = 54345;  // nodes listen here
    const int osc_info = 54355;      // telemetry out to the controller
}
