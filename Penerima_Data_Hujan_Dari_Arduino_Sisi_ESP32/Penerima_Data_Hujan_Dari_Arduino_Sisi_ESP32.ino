#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define RELAY_ON 0
#define RELAY_OFF 1

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "*******";
char pass[] = "*******";
int RELAY_PIN = 5;
SoftwareSerial Hujan(D8, D7);  // RX, TX
char c;
String dataIn;
int8_t indexOfA, indexOfB, indexOfC, indexOfD, indexOfE, indexOfF;
String dataRain, dataSoil, dataLight, dataSuhu, dataHumidity, dataSprinkler;

#define BLYNK_TEMPLATE_ID "*************"
#define BLYNK_TEMPLATE_NAME "**********************"
#define BLYNK_AUTH_TOKEN "*************************"
#define BLYNK_PRINT Serial

boolean manualPomp = false;

BLYNK_WRITE(V6) {
  if (param.asInt() == 1) {
    Serial.println("on");
    digitalWrite(RELAY_PIN, RELAY_ON);
    Blynk.virtualWrite(V6, 1);

  } else {
    Serial.println("off");
    digitalWrite(RELAY_PIN, RELAY_OFF);
    Blynk.virtualWrite(V6, 0);
  }
}

BLYNK_WRITE(V7) {
  if (param.asInt() == 1) {
    manualPomp = true;

  } else {
    manualPomp = false;
  }
}

void setup() {
  Serial.begin(9600);
  Hujan.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V7, 0);
}

void loop() {
  Blynk.run();
  String data_hujan = "";
  while (Hujan.available() > 0) {
    c = Hujan.read();

    if (c == '\n') {
      break;
    } else {
      dataIn += c;
    }
  }

  if (c == '\n') {
    ParseData();

    Serial.println("Data Hujan = " + dataRain);
    Serial.println("Data Soil = " + dataSoil);
    Serial.println("Data Light = " + dataLight);
    Serial.println("Data Suhu = " + dataSuhu);
    Serial.println("Data Humidity = " + dataHumidity);
    Serial.println("Data Sprinkler = " + dataSprinkler);
    Serial.println("=========================");
    Blynk.virtualWrite(V0, dataSuhu);
    Blynk.virtualWrite(V1, dataSoil);
    Blynk.virtualWrite(V2, dataHumidity);
    Blynk.virtualWrite(V3, dataLight);
    Blynk.virtualWrite(V4, dataRain);

    if (!manualPomp) {
      if (dataSprinkler == "1") {
        Blynk.virtualWrite(V6, dataSprinkler);

        digitalWrite(RELAY_PIN, RELAY_ON);
      } else {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        Blynk.virtualWrite(V6, dataSprinkler);
      }
    }

    c = 0;
    dataIn = "";
  }

  delay(2000);
}

void ParseData() {
  indexOfA = dataIn.indexOf("A");
  indexOfB = dataIn.indexOf("B");
  indexOfC = dataIn.indexOf("C");
  indexOfD = dataIn.indexOf("D");
  indexOfE = dataIn.indexOf("E");
  indexOfF = dataIn.indexOf("F");

  dataRain = dataIn.substring(0, indexOfA);
  dataSoil = dataIn.substring(indexOfA + 1, indexOfB);
  dataLight = dataIn.substring(indexOfB + 1, indexOfC);
  dataSuhu = dataIn.substring(indexOfC + 1, indexOfD);
  dataHumidity = dataIn.substring(indexOfD + 1, indexOfE);
  dataSprinkler = dataIn.substring(indexOfE + 1, indexOfF);
}
