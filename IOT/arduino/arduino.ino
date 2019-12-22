/*
 WiFiEsp example: WebServerLed
 
 A simple web server that lets you turn on and of an LED via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

// Red Led pin number 11
int pin_LED_R = 11;
// Triger pin number 7 setup
int pin_UL_TRIG = 7;
// Echo pin number 8 setup
int pin_UL_OUT = 8;
int timer;

#include "WiFiEsp.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "iptime-----";            // your network SSID (name)
char pass[] = "ffffffff";        // your network password
int status = WL_IDLE_STATUS;

int ledStatus = LOW;
int led1 = 5;

WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
  pinMode(led1, OUTPUT);  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);   // initialize serial for debugging
  Serial1.begin(115200);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();

  // Red Led pin Output Setup
  pinMode(pin_LED_R, OUTPUT);
  // Out pin Input Setup
  pinMode(pin_UL_OUT, INPUT);
  // Trig pin Output Setup
  pinMode(pin_UL_TRIG, OUTPUT);
  digitalWrite(pin_UL_TRIG, 0);
  timer = 0;
}

void loop(){
  sensor();
  web();
}

long blink_previousMillis = 0;
long interval = 1000;
long timelimit = 5;
void sensor(){
  unsigned long currentMillis = millis();
  if(currentMillis - blink_previousMillis >= interval) {
    blink_previousMillis = currentMillis;
    unsigned long microseconds, distance_cm;
    // Ultrasonic wave transmission
    digitalWrite(pin_UL_TRIG, 0); // Output pin_ULTRASONIC_T to LOW
    delayMicroseconds(2);
    // pull the Trig pin to high level for more than 10us impulse 
    digitalWrite(pin_UL_TRIG, 1); // Output pin_ULTRASONIC_T to HIGH       
    delayMicroseconds(10);
    digitalWrite(pin_UL_TRIG, 0); // Output pin_ULTRASONIC_T to LOW
    
    // waits for the pin to go HIGH, and returns the length of the pulse in microseconds
    microseconds = pulseIn(pin_UL_OUT, 1, 24000);
    // time to dist
    distance_cm = microseconds * 17/1000;
  
    if(distance_cm < 50){
      timer++;
    }
    else{
      timer = 0;
    }
    // Red Led ON
    if(timer < timelimit){
      digitalWrite(pin_LED_R, LOW);
    }
    // Red Led OFF
    else{
      digitalWrite(pin_LED_R, HIGH);
    }
  }
}
void web()
{
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer

        // printing the stream to the serial monitor will slow down
        // the receiving of data from the ESP filling the serial buffer
        Serial.write(c);
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
          break;
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (buf.endsWith("GET /H")) {
          //Serial.println("Turn led ON");
          ledStatus = HIGH;
          digitalWrite(led1, HIGH);   // turn the LED on (HIGH is the voltage level)
        }
        else if (buf.endsWith("GET /L")) {
          //Serial.println("Turn led OFF");
          ledStatus = LOW;
          digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
        }
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}


void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  if(timer > timelimit){
    client.print("Someone is here for ");
    client.print(timer);
    client.print(" Second.");
  }
  else{
    client.print("No one is here.");
  }
  client.println("<br>");
  client.println("<br>");
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
