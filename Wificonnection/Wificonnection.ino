/* 
 *  
 *  Wifif connection test
 */
 
#include <ESP8266WiFi.h>


const char* ssid     = "CE-Hub";
const char* password = "";
const char* host = "iot.io";

void setup() {
  Serial.begin(115200);
  delay(100);

  //  Connection to a WiFi network
  //  Print out debug info via the Serial monitor
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
 // A while loop runs continuously when there is not a connection 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
 // Print "." to show the end of one loop
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// Already had a connection and then print data about the connection
int value = 0;
// Main loop ; Use Wificlient class to construct a client,define the port ,establish the connection
void loop() {
  delay(5000);

  Serial.println("-------------------------------");
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;

}

// Create a URI for the request
  String url = "/data/index.html";
  Serial.print("Requesting URL: ");
  Serial.println(host + url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
}
  
