#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

WebServer server(80);

int redLED = 23;
int yellowLED = 22;
int greenLED = 21;
int buttonPin = 19;
int buzzerPin = 18;

String currentLight = "RED";
bool pedestrianRequest = false;

unsigned long previousMillis = 0;
int countdown = 5;

void setLight(String color) {
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);

  if (color == "RED") digitalWrite(redLED, HIGH);
  if (color == "YELLOW") digitalWrite(yellowLED, HIGH);
  if (color == "GREEN") digitalWrite(greenLED, HIGH);

  currentLight = color;
}

void beep() {
  digitalWrite(buzzerPin, HIGH);
  delay(200);
  digitalWrite(buzzerPin, LOW);
}

void pedestrianMode() {
  setLight("RED");

  for (int i = 5; i > 0; i--) {
    countdown = i;
    beep();
    delay(1000);
  }

  pedestrianRequest = false;
  setLight("GREEN");
  countdown = 5;
}

void nextLight() {
  if (pedestrianRequest) {
    pedestrianMode();
    return;
  }

  if (currentLight == "RED") {
    setLight("GREEN");
    countdown = 5;
  } else if (currentLight == "GREEN") {
    setLight("YELLOW");
    countdown = 2;
  } else {
    setLight("RED");
    countdown = 5;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS failed");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println(WiFi.localIP());

  setLight("RED");

  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/status", []() {
    String json = "{";
    json += "\"light\":\"" + currentLight + "\",";
    json += "\"countdown\":" + String(countdown);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/pedestrian", []() {
    pedestrianRequest = true;
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (digitalRead(buttonPin) == LOW) {
    pedestrianRequest = true;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    countdown--;

    if (countdown <= 0) {
      nextLight();
    }
  }
}
