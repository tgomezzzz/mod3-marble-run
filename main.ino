#include <WiFi.h>
#include <HTTPClient.h>
#include <Stepper.h>
#include <ESP32Servo.h>

#define USE_SERIAL Serial

const char *ssid_Router     = "Columbia University";
const char *password_Router = "";

String address= "http://134.122.113.13/tjk2132/running";

bool run = false;
unsigned long i = 0;

Servo serv;
int posVal = 0;
int servoPin = 26;

int outPorts[] = {35, 32, 33, 25};

unsigned long t;

void moveOneStep(bool dir) {
  static byte out = 0x01;
  if (dir) {
    out != 0x08 ? out = out << 1 : out = 0x01;
  } else {
    out != 0x01 ? out = out >> 1 : out = 0x08;
  }
  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts[i], (out & (0x01 << i)) ? HIGH : LOW);
  }
}

void moveSteps(bool dir, int steps, byte ms) {
  for (unsigned long i = 0; i < steps; i++) {
    moveOneStep(dir);
    delay(constrain(ms, 3, 20));
  }
}

void moveServo(int dir) {
  if (dir > 0) {
    for (int j = 0; j < dir; j++) {
      posVal += 1;
      serv.write(posVal);
      delay(15);
    }
  } else {
    for (int j = -dir; j > 0; j--) {
      posVal -= 1;
      serv.write(posVal);
      delay(15);
    }
  }
}

void setup(){
  
  USE_SERIAL.begin(115200);

  WiFi.begin(ssid_Router, password_Router);
  USE_SERIAL.println(String("Connecting to ")+ssid_Router);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println("\nConnected, IP address: ");
  USE_SERIAL.println(WiFi.localIP());
  USE_SERIAL.println("Setup End");

  for (int i = 0; i < 4; i++) {
    pinMode(outPorts[i], OUTPUT);
  }

  serv.setPeriodHertz(50);
  serv.attach(servoPin, 500, 2500);
  moveServo(30);
} 
 
void loop() {

  if (run) {

    // Servo code.
    if (i == 1) {
      moveServo(90);
      Serial.println("moving right");
    } else if (i >= 2) {
      moveServo(- 90);
      i = 0;
      Serial.println("moving left");
    }
    i++;
    
    // Stepper code.
    moveSteps(false, 32 * 64, 3);
    Serial.println(i);
  } else if((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(address);
 
    int httpCode = http.GET(); // start connection and send HTTP header
    if (httpCode == HTTP_CODE_OK) { 
        String response = http.getString();
        if (response.equals("false")) {
            // Do not run sculpture, perhaps sleep for a couple seconds
        }
        else if(response.equals("true")) {
            run = true;
            t = millis();
        }
        USE_SERIAL.println("Response was: " + response);
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(500); // sleep for half of a second
  }
}
