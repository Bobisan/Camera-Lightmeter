#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <SparkFun_VEML7700_Arduino_Library.h>
#include <Adafruit_VEML7700.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define ISO_BUTTON    13
#define RE1a 10
#define RE1b 21
#define RE2a 11
#define RE2b 36

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//VEML7700 veml;
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// ── Lookup tables ──────────────────────────────────────────────────────────────
const float arr_full_seconds_SP[]  = {30,25,20,15,13,10,8,6,5,4,3,2.5,2,1.6,1.3,1};
const float arr_parts_seconds_SP[] = {1.3,1.6,2,2.5,3,4,5,6,8,10,13,16,20,25,30,40,50,60,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000};
const float arr_Aperture[] = {1,1.1,1.2,1.4,1.6,1.8,2,2.2,2.4,2.8,3.2,3.5,4,4.5,5,5.6,6.3,7.1,8,9,10,11,13,14,16,18,20,22,25,29};

#define FULL_SP_COUNT   16
#define PARTS_SP_COUNT  36
#define TOTAL_SP_COUNT  (FULL_SP_COUNT + PARTS_SP_COUNT)  // 52
#define MAX_AP_INDEX    29

// ── State ──────────────────────────────────────────────────────────────────────
volatile int  enc1Delta  = 0;   // RE1: aperture / ISO adjust
volatile int  enc2Delta  = 0;   // RE2: select parameter
volatile uint8_t enc1Last = 0;
volatile uint8_t enc2Last = 0;

int  current_position = 0;      // 0 = aperture, 1 = shutter
int  DisplayVariant = 0;      // 0 = main, 1 = ISO
float  ISO = 100;
int  Aperature_Index = 9;
int  SP_Index = 32;
float SP_value = 60;
float lux = 0;
float Aperature_Value = 2.8;
float ev = 0;

bool  needsUpdate= true;
unsigned long startTimer= 0;
const unsigned long HOLD_DURATION = 10000;  // ms to hold RE1 before exiting ISO screen

// ── Quadrature ISR helper ──────────────────────────────────────────────────────
// Full-step table: only counts on clean AB transitions to reduce bounce.
static const int8_t QEM[16] = {
  0,-1, 1, 0,
  1, 0, 0,-1,
  -1, 0, 0, 1,
  0, 1,-1, 0
};

void IRAM_ATTR isr_enc1() {
  uint8_t cur = (digitalRead(RE1a) << 1) | digitalRead(RE1b);
  enc1Delta += QEM[(enc1Last << 2) | cur];
  enc1Last   = cur;
}

void IRAM_ATTR isr_enc2() {
  uint8_t cur = (digitalRead(RE2a) << 1) | digitalRead(RE2b);
  enc2Delta += QEM[(enc2Last << 2) | cur];
  enc2Last   = cur;
}

// ── SP helpers ────────────────────────────────────────────────────────────────
float spValueFromIndex(int idx) {
  if (idx < FULL_SP_COUNT)
    return arr_full_seconds_SP[idx];
  return arr_parts_seconds_SP[idx - FULL_SP_COUNT];
}

// ── Apreture helpers ────────────────────────────────────────────────────────────────
int findNearestApertureIndex(float value) {
  int bestIndex = 0;
  float bestDiff = fabs(value - arr_Aperture[0]);

  for (int i = 1; i <= MAX_AP_INDEX; i++) {
    float diff = fabs(value - arr_Aperture[i]);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestIndex = i;
    }
  }
  return bestIndex;
}
// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin(8,9);
  pinMode(RE1a, INPUT);
  pinMode(RE1b, INPUT);
  pinMode(RE2a, INPUT);
  pinMode(RE2b, INPUT);
  pinMode(ISO_BUTTON, INPUT_PULLUP);

  // Seed the "last" state so first edge doesn't produce a phantom step
  enc1Last = (digitalRead(RE1a) << 1) | digitalRead(RE1b);
  enc2Last = (digitalRead(RE2a) << 1) | digitalRead(RE2b);

  // Attach interrupts to both pins of each encoder
  attachInterrupt(digitalPinToInterrupt(RE1a), isr_enc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE1b), isr_enc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE2a), isr_enc2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE2b), isr_enc2, CHANGE);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  if (veml.begin() == false)
  {
    Serial.println("Unable to communicate with the VEML7700. Please check the wiring. Freezing...");
    while (1)
        ;
  }
  //veml.setGain(VEML7700_GAIN_1);
  //veml.setIntegrationTime(VEML7700_IT_100MS);

  display.clearDisplay();
  display.display();

  SP_value = spValueFromIndex(SP_Index);
}

// ── Draw OLED (only called when needsUpdate is true) ──────────────────────────
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);

  if (DisplayVariant == 0) {
    display.printf("AP: f/%.1f\n", Aperature_Value);
    if (SP_Index < FULL_SP_COUNT)
      display.printf("SP: %.1fs\n", SP_value);
    else
      display.printf("SP: 1/%.0f\n", SP_value);
  } else {
    display.println("ISO");
    display.printf("%d\n", ISO);
  }

  display.display();
  needsUpdate = false;
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  lux = veml.readLux();
  ev = log2(lux/2.5);
  Serial.printf("ev:%.1f\n", ev);
  // ── Atomically grab and clear encoder deltas ──────────────────────────────
  noInterrupts();
  int d1 = enc1Delta;  enc1Delta = 0;
  int d2 = enc2Delta;  enc2Delta = 0;
  interrupts();

  // ── RE2: cycle through parameters (aperture / shutter) ───────────────────
  if (d2 != 0 && DisplayVariant == 0) {
    // One detent = one step; clamp to number of selectable params
    current_position = constrain(current_position + (d2 > 0 ? 1 : -1), 0, 1);
    needsUpdate = true;
  }

  // ── RE1: adjust value ─────────────────────────────────────────────────────
  if (d1 != 0) {
    if (DisplayVariant == 0) {
      switch (current_position) {
        case 0:   // Aperture
          Aperature_Index = constrain(Aperature_Index + d1, 0, MAX_AP_INDEX);
          Aperature_Value = arr_Aperture[Aperature_Index];
          SP_value = ((250*Aperature_Value*Aperature_Value)/ISO*lux);
          needsUpdate = true;
          break;
        case 1:   // Shutter speed
          SP_Index = constrain(SP_Index + d1, 0, TOTAL_SP_COUNT - 1);
          SP_value = spValueFromIndex(SP_Index);

          float calculatedAperture = sqrt((lux * ISO * SP_value) / 250.0);

          Aperature_Index  = findNearestApertureIndex(calculatedAperture);
         // Aperature_Value = arr_Aperture[Aperature_Index];
          Aperature_Value = calculatedAperture;
          needsUpdate = true;
          break;
      }
    } else {
      // ISO screen: double/halve per detent
      if (d1 > 0) {
        for (int i = 0; i < abs(d1); i++) ISO = max(ISO * 2, ISO + 1); // guard against overflow
      } else {
        for (int i = 0; i < abs(d1); i++) ISO = max(1.0f, ISO / 2);
      }
      needsUpdate = true;
      startTimer  = 0;  // reset the hold-to-exit timer on any adjustment
    }
  }

  // ── ISO button: enter ISO screen ─────────────────────────────────────────
  // Button is INPUT_PULLUP → active LOW
  if (digitalRead(ISO_BUTTON) == LOW && DisplayVariant == 0) {
    DisplayVariant = 1;
    needsUpdate    = true;
    startTimer     = 0;
  }

  // ── Auto-exit ISO screen after HOLD_DURATION of no encoder activity ───────
  if (DisplayVariant == 1) {
    if (d1 == 0) {
      if (startTimer == 0) startTimer = millis();
      if (millis() - startTimer >= HOLD_DURATION) {
        DisplayVariant = 0;
        needsUpdate = true;
        startTimer = 0;
      }
    } else {
      startTimer = 0;   // activity → reset hold timer
    }
  }

  // ── Render ────────────────────────────────────────────────────────────────
  // Only push to OLED when something changed; this prevents I²C bus saturation.
  if (needsUpdate) {
    updateDisplay();
  }
}
