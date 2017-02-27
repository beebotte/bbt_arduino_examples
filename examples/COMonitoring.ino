#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


#define BBT "mqtt.beebotte.com" // Domain name of Beebotte MQTT service
#define Token "token:xxxxxxxxxxx" // Set your channel token here
#define Channel "Arduino" // Replace with your channel name
#define CoResource "co" //Replace with your resource name
#define OverLimitResource "overLimit" //Replace with your resource name
#define Write true 


EthernetClient ethClient;
PubSubClient client(ethClient);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
long lastReconnectAttempt = 0;

// interval for sending CO readings to Beebotte
const long interval = 10000; // 10 seconds

// last time CO sensor data were sent to Beebotte
unsigned long lastReadingMillis = 0;

const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
char id[17];

// Set the static IP address to use if the DHCP fails to assign
// Feel free to change this according to your network settings
IPAddress ip(192, 168, 1, 202);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

const int aOutPin=0; //the AOUT pin of the CO sensor goes into analog pin A0 of the arduino
const int dOutPin=8; //the DOUT pin of the CO sensor goes into digital pin D8 of the arduino 
const int ledPin=13; //the anode of the LED connects to digital pin D13 of the arduino

int rawValue;
int overLimit;

void setup() { 
  Serial.begin(9600);//Sets the data rate in bits per second (baud) for serial data transmission
  pinMode(dOutPin, INPUT);//sets the pin as an input to the arduino 
  pinMode(ledPin, OUTPUT);//sets the pin as an output of the arduino
  client.setServer(BBT, 1883);
  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // start the Ethernet connection:  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    
     Ethernet.begin(mac, ip, gateway, subnet);
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  lastReconnectAttempt = 0;
}

// publishes data to the specified resource
void publish(const char* resource, float data, bool persist)
{
  StaticJsonBuffer<128> jsonOutBuffer;
  JsonObject& root = jsonOutBuffer.createObject();
  root["channel"] = Channel;
  root["resource"] = resource;
  if (persist) {
    root["write"] = true;
  }
  root["data"] = data;

  // Now print the JSON into a char buffer
  char buffer[128];
  root.printTo(buffer, sizeof(buffer));

  // Create the topic to publish to
  char topic[64];
  sprintf(topic, "%s/%s", Channel, resource);

  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer);
}

void readSensorData()
{
  rawValue= analogRead(aOutPin);//reads the analog value from the CO sensor's AOUT pin  
  overLimit = digitalRead(dOutPin);//reads the digital value from the CO sensor's DOUT pin
  
  if (!isnan(rawValue )) {    
    publish(CoResource, rawValue, Write);    
  }
    
      
    if (overLimit== HIGH){
      digitalWrite(ledPin, HIGH);//if limit has been reached, LED turns on 
      publish(OverLimitResource, true, Write);  
    }
    else{
      digitalWrite(ledPin, LOW);//if threshold not reached, LED remains off
      publish(OverLimitResource, false, Write);  
      }
  }  


const char * generateID()
{
  randomSeed(analogRead(0));
  int i = 0;
  for(i = 0; i < sizeof(id) - 1; i++) {
    id[i] = chars[random(sizeof(chars))];
  }
  id[sizeof(id) -1] = '\0';
  return id;
}

// reconnects to Beebotte MQTT server
boolean reconnect() {
  if (client.connect(generateID(), Token, "")) {
    Serial.println("Connected to Beebotte MQTT");
  }
  return client.connected();
}

//Calculate co concentration
int getCOConcentration(double val){
    double ppm = 3.027*exp(1.0698*(val*3.3/4095));
    return ppm;
}

void loop()
{
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    // read sensor data every 10 seconds
    // and publish values to Beebotte
    unsigned long currentMillis = millis();
    if (currentMillis - lastReadingMillis >= interval) {
      // save the last time we read the sensor data
      lastReadingMillis = currentMillis;

      readSensorData();
    }

    client.loop();
  }
}


