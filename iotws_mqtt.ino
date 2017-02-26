/*
 * Sample code #2 - IoT Workshop
 * Send button press count to MQTT broker, and listen to the channel to get back the count to play a music
 * 
 * Author: Raymond Xie
 * Created on: 9/6/2016
 * Updated on: 1/24/2017
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
// prefix topic with your initials (RX for Raymond Xie), so you don't interference with your fellow participants
// if you choose to use the same topic with other people, that's fun too - as you can control other's board.
const char *mqtt_topic = "RX-music";          
String mqtt_clientid = "";


WiFiClient espClient;
PubSubClient client(espClient, mqtt_server, mqtt_port);

// your button press input
const int buttonPin = D2; 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

// play music
const int buzzerPin = D6;
byte names[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'};  
int tones[] = {1915, 1700, 1519, 1432, 1275, 1136, 1014, 956};
byte melody0[] = "4c4c4g4g5a5a8g4f4f3e3e3d3d8c4g4g4f4f4e4e8d8p8p8p";
byte melody1[] = "8e6g2g4c4d4e4c4d3d1d4d2e2d2c1c1d2e2c8d8p8p";

void playMelody(String payload) {
  byte melody[100];
  int MAX_COUNT = 24;
  int count = 0;
  int count2 = 0; 
  int count3 = 0;

  if( payload == "0" ) {
    MAX_COUNT = 24;  
    for(count=0; count < MAX_COUNT*2; count++) {
      melody[count] = melody0[count];  
    }
  }
  else if( payload == "1") {
    MAX_COUNT = 21;
    for(count=0; count < MAX_COUNT*2; count++) {
      melody[count] = melody1[count];  
    }
  }

  analogWrite(buzzerPin, 0);     
  analogWrite(buzzerPin, 0);     

  for (count = 0; count < MAX_COUNT; count++) {
    for (count3 = 0; count3 <= (melody[count*2] - 48) * 30; count3++) {
      for (count2=0;count2<8;count2++) {
        if (melody[count*2 + 1] == names[count2]) {       
          analogWrite(buzzerPin,1000);
          delayMicroseconds(tones[count2]);
          analogWrite(buzzerPin, 0);
          delayMicroseconds(tones[count2]);
        } 
        if (melody[count*2 + 1] == 'p') {
          // make a pause of a certain size
          analogWrite(buzzerPin, 0);
          delayMicroseconds(500);
        }
      }
    }
    delay(1);
  }  
}


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
  // Serial.print("checkButtonPress: ");
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == LOW) {
      // if the current state is HIGH then the button was pressed
      buttonPushCounter++;
      if( buttonPushCounter == 2 ) {
        // reset 
        buttonPushCounter = 0;
      }

      digitalWrite( BUILTIN_LED, LOW);
      
      Serial.print("number of button pushes:  ");
      Serial.println(buttonPushCounter);
      
      // publish button press count
      client.publish(mqtt_topic, String(buttonPushCounter));
    }
  }
  
  // keep tracking buttonstate
  lastButtonState = buttonState;
}


// Callback function upon receiving message from MQTT
const int BUFFER_SIZE = 100;
void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  String payload = pub.payload_string();
  Serial.println(payload);

  digitalWrite( BUILTIN_LED, HIGH);

  // play different music based on return value
  playMelody(payload);
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
  delay(500);
}



