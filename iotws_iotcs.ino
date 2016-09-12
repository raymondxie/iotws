/*
 * Sample code #3 - IoT Workshop
 * Press button to send your name to Oracle IoT Cloud Service with a hello message.
 * 
 * Author: Raymond Xie
 * Date: 9/6/2016
 * 
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>

// HINT:  provide the following information
//
// WiFi connection: SSID and Password
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_passwd";

// MQTT server params: server, port, user, password, and unique clientid
const char* mqtt_server = "m12.cloudmqtt.com";
const int mqtt_port = 11565;  
const char *mqtt_user = "ask_your_instructor";
const char *mqtt_pass = "ask_your_instructor";
const char *mqtt_clientid = "CID-J1-Table01";   // check with instructor, following format: CID-J1-Table01, CID-OOW-Table02
const char *mqtt_topic = "iotcs";               // to be sent over to Oracle IoT CS
const char *myname = "Raymond Xie";         // To indicate the message is from "Raymond Xie", replace it with your own name

WiFiClient espClient;
PubSubClient client(espClient, mqtt_server, mqtt_port);

// your button press input
const int buttonPin = D2; 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


// Initial one-time setup
void setup() {
  pinMode(buttonPin, INPUT);      
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
    
  setup_wifi();
}

// Loop forever
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // check user input
  checkButtonPress();

  // pace it
  delay(500);
}


void checkButtonPress() {
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      client.publish(mqtt_topic, myname);
    }
  }
  
  // keep tracking buttonstate
  lastButtonState = buttonState;
}


// Callback function upon receiving message from MQTT
const int BUFFER_SIZE = 100;
void callback(const MQTT::Publish& pub) {
  // Just print out for debug
  Serial.print(pub.topic());
  Serial.print(" => ");
  String payload = pub.payload_string();
  Serial.println(payload);

  // blink once to confirm to user that your message has sent
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  
  delay(1);
}

// Connect to WiFi network
void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print(mqtt_clientid);
    Serial.print("  ");
    Serial.print(mqtt_user);
    
    // Attempt to connect
    if (client.connect(MQTT::Connect(mqtt_clientid).set_auth(mqtt_user, mqtt_pass))) {
      Serial.println("MQTT connected");
      
      // subscribe to a topic channel
      client.subscribe(mqtt_topic);
      client.set_callback(callback);
    } 
    else {
      Serial.print("failed, rc=");
      // Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

