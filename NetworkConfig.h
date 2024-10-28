#define GET_ROTO_IP(id) NetworkConfig::roto_object_ips[id]
#define GET_ACID_IP(id) NetworkConfig::acid_object_ips[id]

namespace NetworkConfig {

    const char* ssid     = "TrychtyrLOM";
    const char* password = "LomLomLom";

    const char* ap_ip = "192.168.0.201";
    
    const char* gateway = "192.168.0.1";
    const char* subnet = "255.255.255.0";

    const char* const roto_object_ips[] = {
        "192.168.0.210",
        "192.168.0.211",
        "192.168.0.212",
        "192.168.0.213",
        "192.168.0.214"
    };

    const char* const acid_object_ips[] = {
        "192.168.0.215",
        "192.168.0.216",
        "192.168.0.217",
        "192.168.0.218",
        "192.168.0.219"
    };

    const int osc_from_ctl = 54345;
    const int osc_from_ap = 54350;
}
