// ============================================================
// Garden Watering System — Final Deployment Sketch
// 2 moisture sensors (proxy configuration) -> 4 pumps
//   Sensor 1 (in a tomato plant) -> triggers Tomato 1, Tomato 2, Pepper
//   Sensor 2 (in garlic)         -> triggers Garlic
// ============================================================

// ---- TEST MODE TOGGLE ----
// true  = short interval for bench verification (fires every 10 sec)
// false = real deployment cadence (once per day)
bool TEST_MODE = false;

// ---- Relay trigger polarity (confirmed from your board) ----
const int RELAY_ON  = HIGH;
const int RELAY_OFF = LOW;

// ---- Sensor pins (ADC1 only -- required for WiFi compatibility) ----
const int SENSOR_TOMATO_PIN = 32;  // sensor 1, physically placed in a tomato plant
const int SENSOR_GARLIC_PIN = 33;  // sensor 2, placed in garlic

// ---- Calibration ----
// Raw ADC (12-bit, 0-4095). Lower raw = wetter, higher raw = drier (inverted sensor characteristic).
// "Damp" baseline: raw < 2000 = damp enough, skip watering.
// Threshold = the raw value at/above which a channel is considered dry and triggers its pump(s).
const int RAW_DRY = 3900;
const int RAW_WET = 1000;

// Garlic dislikes drying out -> triggers sooner (lower raw threshold = waters at a wetter reading)
const int THRESHOLD_GARLIC = 1800;
// Tomato/pepper tolerate (and benefit from) more dry-down between waterings -> triggers later
const int THRESHOLD_TOMATO_PEPPER = 2200;

// ---- Pump relay pins ----
const int PUMP_TOMATO1_PIN = 5;   // relay channel 2
const int PUMP_TOMATO2_PIN = 17;  // relay channel 4
const int PUMP_PEPPER_PIN  = 19;  // relay channel 6
const int PUMP_GARLIC_PIN  = 22;  // relay channel 8

const unsigned long PUMP_RUN_MS = 10000; // how long each pump runs when triggered -- tune based on observed flow

const unsigned long REAL_CHECK_INTERVAL_MS = 86400000UL; // 24 hours
const unsigned long TEST_CHECK_INTERVAL_MS = 10000;      // 10 sec, bench only

unsigned long lastCheckTime = 0;

int readRaw(int pin) {
  return analogRead(pin);
}

void runPump(int relayPin, const char* label) {
  Serial.print("  -> Watering: ");
  Serial.println(label);
  digitalWrite(relayPin, RELAY_ON);
  delay(PUMP_RUN_MS);
  digitalWrite(relayPin, RELAY_OFF);
  delay(500); // brief pause before next pump, keeps everything sequential -- never overlapping
}

void checkTomatoGroup() {
  int raw = readRaw(SENSOR_TOMATO_PIN);
  Serial.print("Tomato/Pepper sensor raw: ");
  Serial.print(raw);

  if (raw >= THRESHOLD_TOMATO_PEPPER) {
    Serial.println("  -- DRY, watering tomato1, tomato2, pepper");
    runPump(PUMP_TOMATO1_PIN, "Tomato 1");
    runPump(PUMP_TOMATO2_PIN, "Tomato 2");
    runPump(PUMP_PEPPER_PIN,  "Pepper");
    delay(1000);
    checkTomatoGroup();
  } else {
    Serial.println("  -- damp enough, skipping tomato1/tomato2/pepper");
  }
}

void checkGarlic() {
  int raw = readRaw(SENSOR_GARLIC_PIN);
  Serial.print("Garlic sensor raw: ");
  Serial.print(raw);

  if (raw >= THRESHOLD_GARLIC) {
    Serial.println("  -- DRY, watering garlic");
    runPump(PUMP_GARLIC_PIN, "Garlic"); 
    delay(10000);
    checkGarlic();
  } else {
    Serial.println("  -- damp enough, skipping garlic");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PUMP_TOMATO1_PIN, OUTPUT);
  pinMode(PUMP_TOMATO2_PIN, OUTPUT);
  pinMode(PUMP_PEPPER_PIN, OUTPUT);
  pinMode(PUMP_GARLIC_PIN, OUTPUT);

  digitalWrite(PUMP_TOMATO1_PIN, RELAY_OFF);
  digitalWrite(PUMP_TOMATO2_PIN, RELAY_OFF);
  digitalWrite(PUMP_PEPPER_PIN, RELAY_OFF);
  digitalWrite(PUMP_GARLIC_PIN, RELAY_OFF);

  Serial.println("=== Garden Watering System -- Live ===");
  Serial.print("Mode: ");
  Serial.println(TEST_MODE ? "TEST (10 sec interval)" : "REAL (24 hr interval)");
  Serial.println();

  // Run an immediate check on boot rather than waiting a full interval
  lastCheckTime = millis() - (TEST_MODE ? TEST_CHECK_INTERVAL_MS : REAL_CHECK_INTERVAL_MS);
}

void loop() {
  unsigned long interval = TEST_MODE ? TEST_CHECK_INTERVAL_MS : REAL_CHECK_INTERVAL_MS;

  if (millis() - lastCheckTime >= interval) {
    lastCheckTime = millis();

    Serial.println("---- Daily check ----");
    checkTomatoGroup();
    checkGarlic();
    Serial.println("---- Check complete ----");
    Serial.println();
  }
}


