#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Configuración MQTT
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const char* root_ca;

// Declaración de variables externas
extern WiFiClientSecure secureClient;

// Declaración de funciones
void mqtt_setup(PubSubClient& client);
bool mqtt_reconnect_safe(PubSubClient& client);
void mqtt_loop(PubSubClient& client);
void initializeMQTT();
void handleMQTT();
void sendMQTTStatus();

#endif