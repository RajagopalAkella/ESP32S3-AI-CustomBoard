#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Router-SSID-Name";
const char* password = "Router-Password";
const char* mqtt_server = "192.168.0.178";

const uint16_t  onChip = 2;	      //OnChip
const uint16_t  Relay10A1 = 16;   //Inverse output
const uint16_t  Relay10A2 = 5;

const bool enableSerial = true;
void srlPrintLn(String str){ if(enableSerial) Serial.println(str); }
void srlPrint(String str){ if(enableSerial) Serial.print(str); }

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  srlPrintLn("");
  srlPrint("Connecting to ");
  srlPrintLn(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    srlPrint(".");
  }
  randomSeed(micros());
  srlPrintLn("");
  srlPrintLn("WiFi connected");
  srlPrintLn("IP address: ");
  srlPrintLn(String(WiFi.localIP()));
}

void callback(String topic, byte* payload, unsigned int length) {
String messageTemp; int firstSlash; int secondSlash; String deviceName;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  srlPrint("Message arrived on topic: ");
  srlPrint(topic);
  srlPrint(" Message received is: ");
  srlPrintLn(messageTemp);
  firstSlash = topic.indexOf('/');
  secondSlash = topic.indexOf('/',firstSlash+1);
  deviceName = topic.substring(firstSlash+1, secondSlash);
  srlPrint("Device: ");
  srlPrint(deviceName);
  srlPrint(" Status: ");
  srlPrintLn(messageTemp);
  if (deviceName == "HallBalconyRelay10A1") { if (messageTemp == "ON") { digitalWrite(Relay10A1, HIGH); } else if (messageTemp == "OFF") { digitalWrite(Relay10A1, LOW); } }
  if (deviceName == "Hall2LightRelay10A2") { if (messageTemp == "ON") { digitalWrite(Relay10A2, HIGH); } else if (messageTemp == "OFF") { digitalWrite(Relay10A2, LOW); } }
}

void reconnect() {
  while (!client.connected()) {
    srlPrint("Attempting MQTT connection...");
    String clientId = "ESP8266-Hall1"; //clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      srlPrintLn("connected");
      client.subscribe("cmnd/HallBalconyRelay10A1/#");
      client.subscribe("cmnd/Hall2LightRelay10A2/#");
    } else {
      srlPrint("failed, rc=");
      srlPrint(String(client.state()));
      srlPrintLn(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  if(enableSerial) { Serial.begin(115200);  setup_wifi(); }

  pinMode(onChip, OUTPUT); digitalWrite(onChip, HIGH);
  pinMode(Relay10A1, OUTPUT); digitalWrite(Relay10A1, LOW);
  pinMode(Relay10A2, OUTPUT); digitalWrite(Relay10A2, LOW);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(5000);
}

void loop() {
  if (!client.connected()) { reconnect(); }
  client.loop();
}
