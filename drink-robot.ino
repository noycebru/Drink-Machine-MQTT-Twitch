/*******************************************************************
  Connect to local MQTT server with a Bot

  ESP8266 library from https://github.com/esp8266/Arduino

  Created for noycebru www.twitch.tv/noycebru
 *******************************************************************/
#include "robot.h"
#include "robot_wifi.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

//------------------------------
WiFiClient wiFiClient;
PubSubClient client(wiFiClient); // MQTT client

// LED
CRGB leds[NUM_LEDS];

// Put your setup code here, to run once:
void setup() {

  setupSerial();

  setupPins();

  setupWIFI();

  setupMQTT();

  setupDrinkLEDs();
}

void setupSerial() {
  Serial.begin(115200);
  Serial.println();
}

void setupPins() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
}

void setupWIFI() {
  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");

  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}

void setupMQTT() {
  client.setServer(MQTT_BROKER.c_str(), MQTT_PORT);
  client.setCallback(callback);// Initialize the callback routine
}

void setupDrinkLEDs() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  FastLED.clear(true);
}

void loop() {
  // Check to make sure we are connected to the mqtt server
  reconnectClient();

  // Tell the mqtt client to process its loop
  client.loop();
}

// Reconnect to client
void reconnectClient() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if(client.connect(MQTT_ID.c_str())) {

      Serial.println("Connected!");

      for(int i=0;i < MQTT_TOPIC_COUNT;i++){
        client.subscribe(MQTT_TOPIC[i].c_str());
        Serial.print("Subcribed to: ");
        Serial.println(MQTT_TOPIC[i]);
      }
    } else {
      Serial.println(" try again in 5 seconds");
      // Wait before retrying
      delay(MQTT_RECONNECT_DELAY);
    }
    Serial.println('\n');
  }
}

// Handle incomming messages from the broker
void callback(char* topic, byte* payload, unsigned int length) {
  String response;
  String msgTopic = String(topic);

  Serial.println("topic received message:");
  Serial.println(msgTopic);

  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.println(response);

  // We need to set the default value for the older message format
  long activateValue = ACTIVATE_DEFAULT;

  // This is quick and dirty with minimal input checking
  // We are the only ones sending this data so we shouldn't have to worry
  if (response.indexOf(",") != -1) {
    // It looks like we are receiving the new format so try and parse the activation time
    int delimiterLocation = response.indexOf(",");
    activateValue = response.substring(delimiterLocation + 1, response.length()).toFloat();
  }

  // We need to turn the robot on
  activateRobot(activateValue);
}

void activateRobot(const long activateValue) {

  Serial.print("activateRobot called: ");
  Serial.println(activateValue);

  // Tell the drink bot to start filling the glass
  drinkBotFill(activateValue);

  Serial.println("activateRobot completed!");
  Serial.println();
}

void drinkBotFill(const int activateValue) {
  // Turn on the LEDs
  ledControl(LED_ON);

  // Turn on the pump
  digitalWrite(PUMP_PIN, HIGH);
  delay(activateValue); // This is the amount of time to turn the pump on for
  // Turn off the pump
  digitalWrite(PUMP_PIN, LOW);
  delay(25);
  // Now turn off the LEDs
  ledControl(LED_OFF);
}

void ledControl(const int state) {
  // LED's Off
  if (state == LED_OFF) {
    // Loop over the LEDs and turn them off aka black
    for(int i = NUM_LEDS;i >= 0;i--) {
      leds[i] = CRGB::Black;
      FastLED.show();
      Serial.print(i);
      Serial.println("Black");
      delay(LED_LOOP_DELAY);
    }
  }
  else if (state == LED_ON) {
    // Loop over the LEDs and turn the on with blue color
    for(int i = 0;i <= NUM_LEDS;i++) {
      leds[i] = CRGB::Blue;
      FastLED.show();
      Serial.print(i);
      Serial.println("Blue");
      delay(LED_LOOP_DELAY);
    }
  }
}