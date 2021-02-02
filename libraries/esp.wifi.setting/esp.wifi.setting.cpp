#include <esp.wifi.setting.h>


ESPWifiSetting::ESPWifiSetting(ConfigWifi_t *config,
                               ESP8266WebServer *server) {
    _config = config;
    _server = server;
}

void ESPWifiSetting::ap() {
    WiFi.disconnect();
    for (int i = 0; i < 4; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }

    String hostname("ESP-temp-" + String(ESP.getChipId()));
    WiFi.softAP(hostname);
    IPAddress IP = WiFi.softAPIP();
    SLOGF("Switch into AP mode: %s %s",
          hostname.c_str(), IP.toString().c_str());
}

void ESPWifiSetting::connect() {

    if (_config->dhcp) {
        SLOG("WiFi in DHCP mode");
    } else {
        IPAddress ip;
        ip.fromString(_config->ip);
        IPAddress gw;
        gw.fromString(_config->gateway);
        IPAddress sub;
        sub.fromString(_config->subnet);

        WiFi.config(ip, gw, sub);
        SLOGF("WiFi in static IP mode: %s", ip.toString().c_str());
    }
    WiFi.begin(_config->ssid.c_str(), _config->password.c_str());

    pinMode(LED_BUILTIN, OUTPUT);
    analogWrite(LED_BUILTIN, 1000);

    int retryCount = 0;
    bool st = true;
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        if (st) digitalWrite(LED_BUILTIN, HIGH);
        else analogWrite(LED_BUILTIN, 1000);
        st = !st;
        retryCount++;
        if (retryCount > 20) {
            ap();
            break;
        }
    }
    SLOGF("WiFi connected: http://%s/", WiFi.localIP().toString().c_str());

    analogWrite(LED_BUILTIN, 1000);
}

void ESPWifiSetting::reconnect() {
    WiFi.disconnect();
    connect();
}

void ESPWifiSetting::handleCss() {
    File dataFile = SPIFFS.open("/nstyle.css", "r");
    _server->streamFile(dataFile, "text/css");
    dataFile.close();
}

void ESPWifiSetting::handleSetup() {
    File dataFile = SPIFFS.open("/network_setup.html", "r");
    _server->streamFile(dataFile, "text/html");
    dataFile.close();
}

void ESPWifiSetting::handleData() {
    _config->load();

    String ret =
        "{\"ip\" : \""      + _config->ip + "\","
        "\"gateway\" : \""  + _config->gateway + "\","
        "\"subnet\" : \""   + _config->subnet + "\","
        "\"ssid\" : \""     + _config->ssid + "\","
        "\"password\" : \"" + _config->password + "\","
        "\"dhcp\" : \""     + _config->dhcp + "\","
        "\"localIp\" : \""  + WiFi.localIP().toString() + "\"}";

    _server->setContentLength(ret.length());
    _server->send(200, "text/json", ret);
}

void ESPWifiSetting::handleSaveData() {

    _config->ssid = _server->arg("ssid");
    _config->ssidSize = _config->ssid.length();

    _config->password = _server->arg("password");
    _config->passwordSize = _config->password.length();

    _config->ip = _server->arg("ip");
    _config->ipSize = _config->ip.length();

    _config->gateway = _server->arg("gateway");
    _config->gatewaySize = _config->gateway.length();

    _config->subnet = _server->arg("subnet");
    _config->subnetSize = _config->subnet.length();

    _config->dhcp = _server->arg("dhcp").equals("1") ? 1 : 0;

    _config->save();

    _server->sendHeader("Location", String("/networkSetup"), true);
    _server->send(302, "text/plain", "");
}

uint16_t ESPWifiSetting::begin() {

    _server->on("/nstyle.css",      std::bind(&ESPWifiSetting::handleCss, this));
    _server->on("/networkSetup",    std::bind(&ESPWifiSetting::handleSetup, this));
    _server->on("/networkData",     std::bind(&ESPWifiSetting::handleData, this));
    _server->on("/saveNetworkData", std::bind(&ESPWifiSetting::handleSaveData, this));
    _server->on("/connect",         std::bind(&ESPWifiSetting::reconnect, this));

    // load wifi config variables
    uint16_t lastAddress = _config->load();

    /* connect to wifi
     * if unsuccessful switch to AP mode
     */
    connect();

    return lastAddress;
}
