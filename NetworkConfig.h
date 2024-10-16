#define GET_OBJ_IP(id) NetworkConfig::object_ips[id]

namespace NetworkConfig {

    const char* ssid     = "TrychtyrLOM";
    const char* password = "LomLomLom";

    const char* ap_ip = "192.168.0.201";
    
    const char* gateway = "192.168.0.1";
    const char* subnet = "255.255.255.0";

    const char* const object_ips[] = {
        "192.168.0.210",
        "192.168.0.211",
        "192.168.0.212",
        "192.168.0.213",
        "192.168.0.214"
    };

    const int osc_from_ctl = 54345;
    const int osc_from_ap = 54350;
}
