#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Fuzzy.h>

#define DHTTYPE DHT11
#define RELAY_ON 0
#define RELAY_OFF 1

// Deklarasi PIN Sensor Hujan
int RAIN_PIN = A0;
int SOIL_PIN = A1;
int LIGHT_PIN = A2;
int DHT_PIN = 7;
int RELAY_PIN = 8;

SoftwareSerial Hujan(2, 3);  // RX, TX
DHT_Unified dht(DHT_PIN, DHTTYPE);

//Data Tampung
int DataHujan;
int DataSoil;
int DataLight;
float DataSuhu;
float DataHumidity;
float DataSprinkler;

boolean isSprinklerOn;

sensors_event_t event;

Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput
FuzzySet *dingin = new FuzzySet(-10, 0, 15, 20);
FuzzySet *sedang = new FuzzySet(15, 20, 25, 30);
FuzzySet *panas = new FuzzySet(25, 30, 30, 40);

// FuzzyInput
FuzzySet *basah = new FuzzySet(0, 200, 300, 409);
FuzzySet *normal = new FuzzySet(300, 370, 470, 511);
FuzzySet *kering = new FuzzySet(470, 600, 900, 1023);

// FuzzyInput
FuzzySet *lebat = new FuzzySet(0, 300, 500, 600);
FuzzySet *normalHujan = new FuzzySet(500, 600, 800, 900);
FuzzySet *tidakHujan = new FuzzySet(800, 1000, 1000, 1100);

// FuzzyInput
FuzzySet *terang = new FuzzySet(0, 300, 400, 500);
FuzzySet *normalCahaya = new FuzzySet(400, 500, 700, 900);
FuzzySet *gelap = new FuzzySet(800, 1000, 1000, 1100);

// FuzzyOutput
FuzzySet *stopedOutput = new FuzzySet(0, 0, 0, 100);
FuzzySet *startedOutput = new FuzzySet(0, 100, 100, 100);

void setup() {
  Serial.begin(9600);
  Hujan.begin(9600);
  dht.begin();
  pinMode(RAIN_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  // Set a random seed
  randomSeed(analogRead(0));

  FuzzyInput *suhu = new FuzzyInput(1);
  suhu->addFuzzySet(dingin);
  suhu->addFuzzySet(sedang);
  suhu->addFuzzySet(panas);
  fuzzy->addFuzzyInput(suhu);

  FuzzyInput *soil = new FuzzyInput(2);
  soil->addFuzzySet(basah);
  soil->addFuzzySet(normal);
  soil->addFuzzySet(kering);
  fuzzy->addFuzzyInput(soil);


  FuzzyInput *hujan = new FuzzyInput(3);
  hujan->addFuzzySet(lebat);
  hujan->addFuzzySet(normalHujan);
  hujan->addFuzzySet(tidakHujan);
  fuzzy->addFuzzyInput(hujan);

  FuzzyInput *cahaya = new FuzzyInput(4);
  cahaya->addFuzzySet(terang);
  cahaya->addFuzzySet(normalCahaya);
  cahaya->addFuzzySet(gelap);
  fuzzy->addFuzzyInput(cahaya);

  FuzzyOutput *sprinkler = new FuzzyOutput(1);
  sprinkler->addFuzzySet(stopedOutput);
  sprinkler->addFuzzySet(startedOutput);
  fuzzy->addFuzzyOutput(sprinkler);

  FuzzyRuleAntecedent *ifSuhuTinggiAndCahayaTerangAndKelembabanRendah = new FuzzyRuleAntecedent();
  ifSuhuTinggiAndCahayaTerangAndKelembabanRendah->joinWithAND(panas, terang);
  ifSuhuTinggiAndCahayaTerangAndKelembabanRendah->joinWithAND(kering, kering);
  FuzzyRuleConsequent *thenOutputAktif = new FuzzyRuleConsequent();
  thenOutputAktif->addOutput(startedOutput);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifSuhuTinggiAndCahayaTerangAndKelembabanRendah, thenOutputAktif);
  fuzzy->addFuzzyRule(fuzzyRule1);

  // Hanya potongan code yang relevan disajikan di sini, silakan sesuaikan dengan kebutuhan Anda.
  // ...

  FuzzyRuleAntecedent *ifSuhuRendah = new FuzzyRuleAntecedent();
  ifSuhuRendah->joinSingle(dingin);
  FuzzyRuleConsequent *thenOutputNonAktif = new FuzzyRuleConsequent();
  thenOutputNonAktif->addOutput(stopedOutput);
  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, ifSuhuRendah, thenOutputNonAktif);
  fuzzy->addFuzzyRule(fuzzyRule2);

  FuzzyRuleAntecedent *ifHujanSedangOrLebat = new FuzzyRuleAntecedent();
  ifHujanSedangOrLebat->joinWithOR(lebat, normalHujan);
  FuzzyRuleConsequent *thenOutputNonAktif1 = new FuzzyRuleConsequent();
  thenOutputNonAktif1->addOutput(stopedOutput);
  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, ifHujanSedangOrLebat, thenOutputNonAktif1);
  fuzzy->addFuzzyRule(fuzzyRule3);

  FuzzyRuleAntecedent *ifHujanAtauKelembabanBasah = new FuzzyRuleAntecedent();
  ifHujanAtauKelembabanBasah->joinWithOR(lebat, basah);

  FuzzyRuleConsequent *thenSprinklerMati = new FuzzyRuleConsequent();
  thenSprinklerMati->addOutput(stopedOutput);

  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, ifHujanAtauKelembabanBasah, thenSprinklerMati);
  fuzzy->addFuzzyRule(fuzzyRule4);

  // ...
}

void loop() {

  DataHujan = analogRead(RAIN_PIN);
  DataSoil = analogRead(SOIL_PIN);
  DataLight = analogRead(LIGHT_PIN);

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    DataSuhu = event.temperature;
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    DataHumidity = event.relative_humidity;
  }

  fuzzy->setInput(1, DataSuhu);
  fuzzy->setInput(2, DataSoil);
  fuzzy->setInput(3, DataHujan);
  fuzzy->setInput(4, DataLight);

  fuzzy->fuzzify();

  float output1 = fuzzy->defuzzify(1);

  Serial.println("Output: ");
  Serial.println(output1);

  if (output1 > 50) {
    isSprinklerOn = true;
  } else {
    isSprinklerOn = false;
  }

  Serial.print("Sprinkler: ");
  Serial.println(isSprinklerOn);

  Hujan.print(DataHujan);
  Hujan.print("A");
  Hujan.print(DataSoil);
  Hujan.print("B");
  Hujan.print(DataLight);
  Hujan.print("C");
  Hujan.print(DataSuhu);
  Hujan.print("D");
  Hujan.print(DataHumidity);
  Hujan.print("E");
  Hujan.print(isSprinklerOn);
  Hujan.print("F");
  Hujan.print('\n');

  delay(2000);
}
