/*
 * Sample code #3 - IoT Workshop
 * Press button to send your name to Oracle IoT Cloud Service with a hello message.
 * 
 * Author: Raymond Xie
 * Date: 9/7/2016
 * Update: 1/24/2017
 * 
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>

// TODO:  provide the following connection information
//
// WiFi connection: SSID and Password
const char* ssid = "oc-iotws";
const char* password = "IoTWorkshop";

// MQTT server params: server, port, user, password, and unique clientid
const char* mqtt_server = "192.168.20.251";
const int mqtt_port = 1883; 
const char *mqtt_user = "iotuser";
const char *mqtt_pass = "iotpass";
// MQTT topic: to be sent over to Oracle IoT CS
const char *mqtt_topic = "iotcs-oc";           
String mqtt_clientid = "";
// To indicate the message is from "Raymond Xie", replace it with your own name
const char *myname = "Raymond Xie";         

WiFiClient espClient;
PubSubClient client(espClient, mqtt_server, mqtt_port);

// your button press input
const int buttonPin = D2; 
int buttonState = 0;         // current state of the button

// Initial one-time setup
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);      
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
  
  // if the state has changed, increment the counter
  if (buttonState == LOW) {
    client.publish(mqtt_topic, myname);
  }
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
    mqtt_clientid = String(ESP.getChipId());
    
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


