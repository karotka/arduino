#include <EEPROM.h>
#include <CRC32.h>
#include <config.wifi.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <esp.wifi.setting.h>


ESP8266WebServer server(80);
ConfigWifi_t configWifi;
ESPWifiSetting setting(&configWifi, &server);

void setup() {

    /* Serial for logging
     */
    Serial.begin(115200);
    delay(10);

    /* Start filesystem
     */
    SPIFFS.begin();

    /* EEPROM for store wifi configuration
     */
    EEPROM.begin(EEPROM_SIZE);

    /* Register methods for settings
     */
    uint16_t lastAddress = setting.begin();

    /* Show last EEPROM address
     */
    SLOGF("Last EEPROM address = %d", lastAddress);

    /* Start web server for settings
     */
    server.begin();

}

void loop() {
    server.handleClient();
}