#include <WiFiClientSecure.h>
#include <PubSubClient.h>


// Configura los datos de HiveMQ Cloud
const char* mqtt_server = "b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "hivemq.webclient.1749844320500";
const char* mqtt_password = "bcDm5j>2lRvUKA,%6.3Q";
const char* mqtt_topic = "esp32/status";


// Certificado raíz de HiveMQ Cloud
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdzCCAl+gAwIBAgIEbGz3kDANBgkqhkiG9w0BAQsFADBvMQswCQYDVQQGEwJV\n" \
"UzELMAkGA1UECBMCQ0ExEDAOBgNVBAcTB0lydmluZzEZMBcGA1UEChMQRGlnaUNl\n" \
"cnQgSW5jMRwwGgYDVQQLExNEaWdpQ2VydCBHbG9iYWwgUm9vdDENMAsGA1UEAxME\n" \
"R0NEMB4XDTIxMDYxMDAwMDAwMFoXDTMxMDYxMDAwMDAwMFowbzELMAkGA1UEBhMC\n" \
"VVMxCzAJBgNVBAgTAkNBMRAwDgYDVQQHEwdJcnZpbmcxGTAXBgNVBAoTEERpZ2lD\n" \
"ZXJ0IEluYzEcMBoGA1UECxMTRElHSUNlcnQgR2xvYmFsIFJvb3QxDTALBgNVBAMT\n" \
"BEdDRDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALeJtG6z1U4z8a9T\n" \
"zFv+3Z7z1z5+6z5Zz5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5z5\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure secureClient;

void mqtt_setup(PubSubClient& client) {
  secureClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
}

void mqtt_reconnect(PubSubClient& client) {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      client.publish(mqtt_topic, "Estoy conectado ESP32A");
    } else {
      delay(5000);
    }
  }
}

void mqtt_loop(PubSubClient& client) {
  if (!client.connected()) {
    mqtt_reconnect(client);
  }
  client.loop();
}
