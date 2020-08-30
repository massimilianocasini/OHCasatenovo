// WiFiCloudDHTMqtt.ino
// Company: KMP Electronics Ltd, Bulgaria
// Web: http://kmpelectronics.eu/
// Supported boards:
//    KMP ProDino WiFi-ESP WROOM-02 (http://www.kmpelectronics.eu/en-us/products/prodinowifi-esp.aspx)
// Description:
//    Cloud MQTT example with DHT support. In this example we show how to connect KMP ProDino WiFi-ESP WROOM-02 with Amazon cloudmqtt.com service 
//      and measure humidity and temperature with DHT22 sensor. For connect with WiFi we use WiFiManager.
// Example link: http://www.kmpelectronics.eu/en-us/examples/prodinowifi-esp/wifiwebrelayserverap.aspx
// Version: 1.0.0
// Date: 04.06.2017
// Author: Plamen Kovandjiev 
//
// =========================================================================================
//
// Date: 12.12.2017 - Andrea Montefusco - experiments with MQTT server on the local network
//
// Before try to compile, check an issue I discovered:
//
// https://github.com/kmpelectronics/Arduino/issues/3
//
// This example try first to connect to a statically defined MQTT server defined as per following constants values:
// const char* MQTT_SERVER = "xxx.cloudmqtt.com"; 
// const char* MQTT_PORT   = 1833; 
//
// Preferably the MQTT server cna be searched even dynamically (if present and publisched) on the local broadcast directed 
// segment, using mDNS:
//
// const char* MQTT_SERVER_MDNS = "xxx.cloudmqtt.com"; 
// const char* MQTT_PORT_MNDS   = 1833; 
//
// the above assumes that MNQTT server has been published with the following command (to be executed where the mosquitto daemon is running:
//
//   avahi-publish -s "Mosquitto MQTT server on cm3home" _mqtt-server._tcp 1833 "info=Publish, Publish! Read all about it! mqtt.org"
//
//
// Examples for local mqtt server:
//
// in order to receive info and input statuses, subscribe at mosquitto_sub  -h localhost -t 'ProDinoWiFi/nInfo'
// You will receive status of optoisolated inputs:
// optoIn:0:1
// optoIn:0:0
// optoIn:0:1
//
// and even the status of relay (if commanded):
//  
// rel:1:1
// rel:3:0
// rel:1:0
// rel:1:1
//
// The relay can be driven as follows:
//
// # turn on the last relay
// mosquitto_pub -h localhost -t 'ProDinoWiFi/Cmd' -m 'rel:3:1'
//
// # turn off the last relay
// mosquitto_pub -h localhost -t 'ProDinoWiFi/Cmd' -m 'rel:3:9'
//

#include <KMPDinoWiFiESP.h>
#include <KMPCommon.h>
#include <DHT.h> //https://github.com/adafruit/DHT-sensor-library
#include <ESP8266WiFi.h>  //https://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <PubSubClient.h> //https://pubsubclient.knolleary.net/
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include  <ESP8266mDNS.h>     //mDSN library (needed to detect local mqtt transparently


// MQTT server settings static/global DNS definitions.
const char* MQTT_SERVER = "cm3home"; // Test  mosquitto server (remove xxxx.).
// const char* MQTT_SERVER = "192.168.0.31"; // Your local server (statically defined)
const int MQTT_PORT = 1883;                   // Your server port.
const char* MQTT_CLIENT_ID = "ESP8266Client";  // Default server name.
//const char* MQTT_USER = "xxxxxxxx";            // Your xxxxxxxx MQTT server user.
//const char* MQTT_PASS = "xxxxxxxxxxxx";        // Your xxxxxxxxxxxx MQTT server user password.


// MQTT server settings dynamic (mDNS) local definitions.
const char* MQTT_SERVER_MNDS = "mqtt-server"; // Please note that the trailing ".local" suffix is implicitly defined !!! --Nome del server MQTT pubblicato dal server AVAHI


// MQTT topics
const char* TOPIC_INFO_OPTOIN = "ProDinoWiFi/optoin";
const char* TOPIC_INFO_RELE = "ProDinoWiFi/rele";
const char* TOPIC_INFO_DHT_T = "ProDinoWiFi/dhtt";
const char* TOPIC_INFO_DHT_H = "ProDinoWiFi/dhth";
const char* TOPIC_COMMAND = "ProDinoWiFi/Cmd";
const char* CMD_ALL = "all";
const char* CMD_REL = "rel";
const char* CMD_OPTOIN = "optoIn";
const char* CMD_DHT_H = "dht";
const char* CMD_DHT_T = "dht";
const char CMD_SEP = '-';

DHT _dhtSensor(EXT_GROVE_D0, DHT22, 11);
// Contains last measured humidity from sensor.
float _humidity;
// Contains last measured temperature from sensor.
float _temperature;

// Check sensor data, interval in milliseconds.
const long CHECK_HT_INTERVAL_MS = 60000;
// Store last measure time.
unsigned long _mesureTimeout;

// Declares a ESP8266WiFi client.
WiFiClient _wifiClient;
// Declare a MQTT client.
PubSubClient _mqttClient(_wifiClient);

// There arrays store last states by relay and optical isolated inputs.
bool _lastRelayStatus[4] = { false };
bool _lastOptoInStatus[4] = { false };

// Buffer by send output state.
char _payload[16];
bool _sendAllData;

/**
* @brief Execute first after start device. Initialize hardware.
*
* @return void
*/
void setup(void)
            {
                // You can open the Arduino IDE Serial Monitor window to see what the code is doing
                // Serial connection from ESP-01 via 3.3v console cable
                delay(2000); //Ritardo per inizializzare seriale;
                Serial.begin(115200);
                Serial.println();
                KMPDinoWiFiESP.init();
                Serial.println("AVVIO \r\n");
                Serial.println("Gestione via MQTT IN e OUT.\r\n");
                
                //WiFiManager
                //Local initialization. Once its business is done, there is no need to keep it around
                WiFiManager wifiManager;
              
                 //wifiManager.resetSettings();
                // Is OptoIn 4 is On the board is resetting WiFi configuration.
                if (KMPDinoWiFiESP.GetOptoInState(OptoIn4))
                {
                    Serial.println("Resetting WiFi configuration...\r\n");
                    //reset saved settings
                    wifiManager.resetSettings();
                    Serial.println("WiFi configuration was reset.\r\n");
                }
            
                //fetches ssid and pass from eeprom and tries to connect
                //if it does not connect it starts an access point with the specified name
                //auto generated name ESP + ChipID
                wifiManager.setDebugOutput(true); //Disabilita il debug se FALSE
                wifiManager.autoConnect();
            
                // Initialize MQTT.
                _mqttClient.setCallback(callback);
                _sendAllData = true;
            
              Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
            
              ///////////////////////////////////////////////////////////
              //
              // mDNS client and responder
              //
              ///////////////////////////////////////////////////////////
              if (!MDNS.begin("ESP-136ED7-Prodino")) {
                Serial.println("Error setting up MDNS responder!");
              }
              Serial.println("mDNS responder started");
             // MDNS.addService(("ESP-136ED7-Prodino-"+WiFi.macAddress()).c_str(), "tcp", 80); // Announce prodino tcp service on port 80 (in realtà non c'è nessun server sulla porta 80)
              MDNS.addService("ESP-136ED7-Prodino", "tcp", 80);
              Serial.println("END OF SETUP");
            }

/**
* @brief Callback method. It is fire when has information in subscribed topic.
*
* @return void
*/
void callback(char* topic, byte* payload, unsigned int length)
          {
              Serial.print("Subscribed topic [");
              Serial.print(topic);
              Serial.print("]");
          
              // Check topic.
              if (strncmp(TOPIC_COMMAND, topic, strlen(TOPIC_COMMAND)) != 0)
              {
                  Serial.println("Is not valid.");
                  return;
              }
          
              Serial.print(" payload [");
          
              for (uint i = 0; i < length; i++)
              {
                  Serial.print((char)payload[i]);
              }
              Serial.println("]");
          
              // Command send all data.
              if (strncmp((const char*)payload, CMD_ALL, strlen(CMD_ALL)) == 0)
              {
                  _sendAllData = true;
                  return;
              }
          
              // Relay command.
              // Command structure: [command (rel):relay number (0..3):relay state (0 - Off, 1 - On)]. Example: rel:0:0 
              size_t cmdRelLen = strlen(CMD_REL);
          
              if (strncmp(CMD_REL, (const char*)payload, cmdRelLen) == 0 && length >= cmdRelLen + 4)
              {
                  KMPDinoWiFiESP.SetRelayState(CharToInt(payload[4]), CharToInt(payload[6]) == 1);
              }
          }

/**
* @brief Main method.
*
* @return void
*/
void loop(void)
          {
              // By the normal device work need connected with WiFi and MQTT server.
              if (!ConnectWiFi() || !ConnectMqtt())
              {
                  return;
              }
          
              _mqttClient.loop();
          
              // Publish information in MQTT.
              PublishInformation();
          }

/**
* @brief Publish information in the MQTT server.
*
* @return void
*/
void PublishInformation()
              {
                  char state[2];
                  state[1] = '\0';
                  // Get current Opto input and relay statuses.
                  for (byte i = 0; i < RELAY_COUNT; i++)
                  {
                      bool rState = KMPDinoWiFiESP.GetRelayState(i);
                      if (_lastRelayStatus[i] != rState || _sendAllData)
                      {
                          _lastRelayStatus[i] = rState;
                          state[0] = rState ? '1' : '0';
                          buildPayload(_payload, CMD_REL, CMD_SEP, i, state);
                          Publish(TOPIC_INFO_RELE, _payload);
                      }
                  }
              
                  for (byte i = 0; i < OPTOIN_COUNT; i++)
                  {
                      bool oiState = KMPDinoWiFiESP.GetOptoInState(i);
                      if (_lastOptoInStatus[i] != oiState || _sendAllData)
                      {
                          _lastOptoInStatus[i] = oiState;
                          state[0] = oiState ? '1' : '0';
                          buildPayload(_payload, CMD_OPTOIN, CMD_SEP, i, state);
              
                          Publish(TOPIC_INFO_OPTOIN, _payload);
                      }
                  }
              
                  GetDHTSensorData();
              
                  _sendAllData = false;
              }

/**
* @brief Read data from sensors a specified time.
*
* @return void
*/
void GetDHTSensorData()
            {
                if (millis() > _mesureTimeout || _sendAllData)
                {
                    _dhtSensor.read(true);
                    float humidity = _dhtSensor.readHumidity();
                    float temperature = _dhtSensor.readTemperature();
            
                    if (_humidity != humidity || _sendAllData)
                    {
                        FloatToChars(humidity, 1, _payload);
                        _humidity = humidity;
                        Publish(TOPIC_INFO_DHT_H, _payload);
                    }
            
                    if (_temperature != temperature || _sendAllData)
                    {
                        FloatToChars(temperature, 1, _payload);
                        _temperature = temperature;
                        Publish(TOPIC_INFO_DHT_T, _payload);
                    }
            
                    // Set next time to read data.
                    _mesureTimeout = millis() + CHECK_HT_INTERVAL_MS;
                }
            }

/**
* @brief Build publish payload.
* @param buffer where fill payload.
* @param command description
* @param number device number
* @param state device state
*
* @return void
*/
void buildPayload(char* buffer, const char* command, char separator, byte number, const char* state)
              {
                  int cmdLen = strlen(command);
                  memcpy(buffer, command, cmdLen);
                  buffer[cmdLen++] = separator;
                  buffer[cmdLen++] = IntToChar(number);
                  buffer[cmdLen++] = separator;
                  buffer += cmdLen;
                  int stLen = strlen(state);
                  memcpy(buffer, state, stLen);
                  buffer[stLen] = '\0';
              }

/**
* @brief Publish topic.
* @param topic title.
* @param payload data to send
*
* @return void
*/
void Publish(const char* topic, char* payload)
            {
                Serial.print("Publish topic [");
                Serial.print(topic);
                Serial.print("] payload [");
                Serial.print(_payload);
                Serial.println("]");
            
                _mqttClient.publish(topic, (const char*)_payload);
            }

/**
* @brief Connect to WiFi access point.
*
* @return bool true - success.
*/
bool ConnectWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
//    Serial.print("Stato WIFI ")
 //   Serial.println(WiFi.status()) //Stampa stato WIFI
    {
        Serial.print("Riconnessione WIFI [");
        Serial.print(WiFi.SSID());
        Serial.println("]...");

       WiFi.begin();
     //WiFi.begin(SSID, SSID_PASSWORD);
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
          Serial.println("Non connesso ...");
         {
           return false;
         }
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    return true;
    }
/**
* @brief Connect to MQTT server.
*
* @return bool true - success.
*/
bool ConnectMqtt()
{
    if (!_mqttClient.connected())
    {
    // reset ther server IP address as it could be changed by mDNS procedure (see below)
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  
        Serial.print("Tentativo connessione a  MQTT server ");
    Serial.println(MQTT_SERVER);
    Serial.print("Port: ");
    Serial.println(MQTT_PORT);
    
        //if (_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
    if (_mqttClient.connect(MQTT_CLIENT_ID))
    {
            Serial.println("Connesso al server MQTT !!");
            _mqttClient.subscribe(TOPIC_COMMAND);
        }
        else
        {
            Serial.print("Connessione fallita, Causa=");
            Serial.print(_mqttClient.state());
        Serial.println(" try again after 5 seconds");
        Serial.print("Ricerca via mDNS del servizio ");
        Serial.println(MQTT_SERVER_MNDS);

      // search for MQTT server on the local segment
      // assume that MNQTT server published with the following command
      // avahi-publish -s "Mosquitto MQTT server on cm3home" _mqtt-server._tcp 1833 "info=Publish, Publish! Read all about it! mqtt.org"

      int n = MDNS.queryService(MQTT_SERVER_MNDS, "tcp"); // Send out query for esp tcp services -- MQTT_SERVER_MNDS definito sopra
      Serial.println("mDNS query done");
      if (n == 0) {
         Serial.println("no services found");
      } else {
         Serial.print(n);
         Serial.println(" service(s) found");
         for (int i = 0; i < n; ++i) {
           // Print details for each service found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(MDNS.hostname(i));
            Serial.print(" (");
            Serial.print(MDNS.IP(i));
            Serial.print(":");
            Serial.print(MDNS.port(i));
            Serial.println(")");
            //mqtt_server = MDNS.IP(i);
            //mqtt_server_found = true;
            //mdns_f = 1;
            
            _mqttClient.setServer(MDNS.IP(i), MDNS.port(i));

            if (_mqttClient.connect(MQTT_CLIENT_ID)) {
               Serial.println("Connesso al server MQTT !!!");
               _mqttClient.subscribe(TOPIC_COMMAND);
            } else {
               Serial.print("Connessione fallita, Causa=");
               Serial.print(_mqttClient.state());
               Serial.println(" try again after 5 seconds");
            }
         }
    }
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }

    return _mqttClient.connected();
}
