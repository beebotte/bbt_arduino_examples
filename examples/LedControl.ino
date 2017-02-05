#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define LEDPIN            3         // Pin which is connected to LED.

#define BBT "mqtt.beebotte.com"     // Domain name of Beebotte MQTT service
#define TOKEN "token:XXXXXXXXXX"    // Set your channel token here

#define CHANNEL "mydevice"          // Replace with your device name
#define LED_RESOURCE "led"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
// Feel free to change this according to your network settings
IPAddress ip(192, 168, 0, 177);

const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

EthernetClient ethClient;
PubSubClient client(ethClient);

char id[17];

long lastReconnectAttempt = 0;

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

// will be called every time a message is received
void onMessage(char* topic, byte* payload, unsigned int length) {

  StaticJsonBuffer<128> jsonInBuffer;

  JsonObject& root = jsonInBuffer.parseObject(payload);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  bool data = root["data"];

  Serial.print("Received message of length ");
  Serial.print(length);
  Serial.println();
  Serial.print("data ");
  Serial.print(data);
  Serial.println();

  // Set the led pin to high or low
  digitalWrite(LEDPIN, data ? HIGH : LOW);
}

// reconnects to Beebotte MQTT server
boolean reconnect() {
  if (client.connect(generateID(), TOKEN, "")) {
    char topic[64];
    sprintf(topic, "%s/%s", CHANNEL, LED_RESOURCE);
    client.subscribe(topic);

    Serial.println("Connected to Beebotte MQTT");
  }
  return client.connected();
}

void setup()
{
  pinMode(LEDPIN, OUTPUT);

  client.setServer(BBT, 1883);
  client.setCallback(onMessage);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  lastReconnectAttempt = 0;
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
    delay(500);
    client.loop();
  }
}
