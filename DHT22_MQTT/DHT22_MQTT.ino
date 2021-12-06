/*
    Xiaoling La
    Plant Monitor
    
*/
// Libraries needed
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>

// define sensor variables
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Sensors - DHT22 and Nails
uint8_t DHTPin = 12;        // on Pin 2 of the Huzzah
uint8_t soilPin = 0;      // ADC or A0 pin on Huzzah
float Temperature;
float Humidity;
int Moisture = 1; // initial value just in case web page is loaded before readMoisture called
int sensorVCC = 13;
int blueLED = 2;
DHT dht(DHTPin, DHTTYPE);   // Initialize DHT sensor.

// New variables for new functions added

int IAMHOT;
int IAMGOOD;     
int INEEDWATER;
int IAMFULL;
int IAMCOLD;
  


// Wifi and MQTT
#include "arduino_secrets.h" 

/*
**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below
* 
**** CE-hub connected
* 
#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
 */
  // CE-hub connected
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

ESP8266WebServer server(80);
const char* mqtt_server = "mqtt.cetools.org";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Date and time
Timezone GB;



void setup() {
  // Set up LED to be controllable via broker
  // Initialize the BUILTIN_LED pin as an output
  // Turn the LED off by making the voltage HIGH
  pinMode(BUILTIN_LED, OUTPUT);     
  digitalWrite(BUILTIN_LED, HIGH);  

  // Set up the outputs to control the soil sensor
  // switch and the blue LED for status indicator
  pinMode(sensorVCC, OUTPUT); 
  digitalWrite(sensorVCC, LOW);
  pinMode(blueLED, OUTPUT); 
  digitalWrite(blueLED, HIGH);

  // open a serial connection for debugging
  Serial.begin(115200);
  delay(100);

  // setup the pin for DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();

  // run initialisation functions
  startWifi();
  startWebserver();
  syncDate();

  // start MQTT server
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);

}
  // main loop 
void loop() {
  // handler for receiving requests to webserver
  server.handleClient();
  
  // Set the frequency of reading data;
  // Call sensor functions;
  if (minuteChanged()) {
    readMoisture();
    sendMQTT();
    Serial.println(GB.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")
  }
  
  client.loop();
}
  // readMoisture function turns on the soil senser so that voltage is applied to the sensor and a resistance can be measured
void readMoisture(){
  
  // power the sensor
  digitalWrite(sensorVCC, HIGH);
  digitalWrite(blueLED, LOW);
  // delay to slow down and enable the current to stabilise
  delay(100);
  // read the value from the sensor:
  Moisture = analogRead(soilPin);         
  //Moisture = map(analogRead(soilPin), 0,320, 0, 100);    // note: if mapping work out max value by dipping in water
  // 320 is the max value; 0 is the min value; can be changed depend on the actual value.    
  //stop power 
  digitalWrite(sensorVCC, LOW);  
  digitalWrite(blueLED, HIGH);
  delay(100);
  Serial.print("Wet ");
  Serial.println(Moisture);   // read the value from the nails
}

void startWifi() {
  // start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // check to see if connected and wait until you are
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void syncDate() {
  // get real date and time
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  GB.setLocation("Europe/London");
  Serial.println("London time: " + GB.dateTime());

}
  // start the webserver 
void startWebserver() {
  // when connected and IP address obtained start HTTP server
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void sendMQTT() {
 // try to reconnect if not
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Temperature = dht.readTemperature(); // Gets the values of the temperature
  snprintf (msg, 50, "%.1f", Temperature);// senser values are stored in a msg character array
  Serial.print("Publish message for t: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnxla/temperature", msg);

  Humidity = dht.readHumidity(); // Gets the values of the humidity
  snprintf (msg, 50, "%.0f", Humidity);
  Serial.print("Publish message for h: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnxla/humidity", msg);

  //Moisture = analogRead(soilPin);   // moisture read by readMoisture function
  snprintf (msg, 50, "%.0i", Moisture);
  Serial.print("Publish message for m: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnxla/moisture", msg);
  


  // Determine whether the soil data exceeds the limit; show the warning;

if (Temperature>28){
    
    Serial.print("I am hot");
    IAMHOT = 1;
    snprintf (msg, 50, "%.0i", IAMHOT);
    Serial.print("Publish message for IAMHOT: ");
    Serial.println(msg);
    client.publish("student/CASA0014/plant/ucfnxla/IAMHOT", msg);}
    
  if (Temperature<16){
    
    Serial.print("I am cold");
    IAMCOLD = 1;
    snprintf (msg, 50, "%.0i", IAMCOLD);
    Serial.print("Publish message for IAMCOLD: ");
    Serial.println(msg);
    client.publish("student/CASA0014/plant/ucfnxla/IAMCOLD", msg);}



if (Moisture<15){
    
    Serial.print("I need water!");
    INEEDWATER = 1;
    snprintf (msg, 50, "%.0i", INEEDWATER);
    Serial.print("Publish message for INEEDWATER: ");
    Serial.println(msg);
    client.publish("student/CASA0014/plant/ucfnxla/INEEDWATER", msg);}


 if ( 16<Temperature<28 && Moisture>15  )
 {   IAMGOOD = 1;
    Serial.print("I am good");
    snprintf (msg, 50, "%.0i", IAMGOOD);
    Serial.print("Publish message for IAMGOOD: ");
    Serial.println(msg);
    client.publish("student/CASA0014/plant/ucfnxla/IAMGOOD", msg);
   }
    



}
  // Execute code when a message is recieved from the MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect with clientID, username and password
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("student/CASA0014/plant/ucfnxla/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "text/html", SendHTML(Temperature, Humidity, Moisture));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

// web page sent 

String SendHTML(float Temperaturestat, float Humiditystat, int Moisturestat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP8266 DHT22 Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Huzzah DHT22 Report</h1>\n";

  ptr += "<p>Temperature: ";
  ptr += (int)Temperaturestat;
  ptr += " C</p>";
  ptr += "<p>Humidity: ";
  ptr += (int)Humiditystat;
  ptr += "%</p>";
  ptr += "<p>Moisture: ";
  ptr += Moisturestat;
  ptr += "</p>";
  ptr += "<p>Sampled on: ";
  ptr += GB.dateTime("l,");
  ptr += "<br>";
  ptr += GB.dateTime("d-M-y H:i:s T");
  ptr += "</p>";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
