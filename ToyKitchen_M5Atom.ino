#include <M5Atom.h>
#include <math.h>
#include "image.h"

const int VOLUME_PIN = 33;
const int VOLUME_SW = 23;
const int LED_NUM = 4;
const int LED_PIN[LED_NUM] = {25, 21, 19, 22};
const int LED_CH[LED_NUM] = {0, 1, 2, 3};

int state = 0;

void setup() {
  M5.begin(true, false, true);
  delay(50);
  M5.dis.clear();
  pinMode(VOLUME_PIN, INPUT);
  pinMode(VOLUME_SW, INPUT_PULLUP);
  for (int i = 0; i < LED_NUM; i++) {
    pinMode(LED_PIN[i], OUTPUT);
    ledcSetup(LED_CH[i], 1000, 12);
    ledcAttachPin(LED_PIN[i], LED_CH[i]);
  }
  Serial.begin(115200);
  Serial.println("Hello!");
}

void loop() {
  M5.update();
  if (M5.Btn.wasPressed()) {
    state = (state + 1) % 2;
    const unsigned char *imageTable[] = {
      image_0,
      image_1,
      image_2,
      image_3,
    };
    M5.dis.displaybuff((uint8_t*)imageTable[state], 0, 0);
    delay(1000);
  }
  switch (state) {
    case 0:
      state0Loop();
      break;
    case 1:
      state1Loop();
      break;
    case 2:
      state2Loop();
      break;
    case 3:
      state3Loop();
      break;
    default:
      break;
  }
}

void state0Loop() {
  if (isVolumeSwOn()) {
    float power = readVolume();
    for (int i = 0; i < LED_NUM; i++) {
      float rate = randomRate();
      fireLED(i, power * rate / 2); // MAXだと明るすぎるので半分に落とした
    }
    displayPowerIndicator(power);
  } else {
    for (int i = 0; i < LED_NUM; i++) {
      fireLED(i, 0);
      displayClear();
    }
  }
  delay(100);
}
void state1Loop() {
  static unsigned long lastms = 0;
  static int count = 0;
  unsigned long ms = millis();
  if (!isVolumeSwOn()) {
    static const float power[LED_NUM] = {0, 0, 0, 0};
    fireLEDs(power);
    lastms = ms - 1000;
    return;
  }
  int vol = analogRead(VOLUME_PIN);
  float vol_linear = sqrt(vol); // 0~63.99
  float hz = vol_linear / 64 * 19 + 1; // 1~20
  if (ms - lastms > (1000.0 / hz + 0.5)) {
    lastms = ms;
    count = (count + 1) % 4;
    static const float powerTable[][LED_NUM] = {
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1},
    };
    fireLEDs(powerTable[count]);
  }
}
void state2Loop() {

}
void state3Loop() {

}

float randomRate() {
  // 1/fゆらぎを再現
  // 参考: https://rinie.hatenadiary.org/entry/20120917/1347821818
  float r = (float)random(1024) / 1024;
  float rate;
  if (r < 0.5) {
    rate =  r + 2 * r * r;
  } else {
    rate = r - 2 * (1 - r) * (1 - r);
  }
  if (rate < 0.8) {
    rate += 0.8;
  }
  return rate;
}

boolean isVolumeSwOn() {
  return digitalRead(VOLUME_SW) == LOW;
}

float readVolumeRaw() {
  // 0~1
  return (float)analogRead(VOLUME_PIN) / 4095.0;
}
float readVolume() {
  // 0~1 で線形に変換した値を返す
  float v = readVolumeRaw();
  // 変換テーブル（実測値）
  static const float linearTable[5][2] = {
    {0.00, 0.00},
    {0.08, 0.25},
    {0.18, 0.50},
    {0.52, 0.75},
    {1.00, 1.00}
  };
  for (int i = 1; i < 5; i++) {
    float x1 = linearTable[i - 1][0];
    float y1 = linearTable[i - 1][1];
    float x2 = linearTable[i][0];
    float y2 = linearTable[i][1];
    if (x1 <= v && v <= x2) {
      return (y2 - y1) / (x2 - x1) * (v - x1) + y1;
    }
  }
  return 1.0;
}

void fireLEDs(const float power[LED_NUM]) {
  // 炎のLEDを複数点灯 power: 0~1
  for (int i = 0; i < LED_NUM; i++) {
    fireLED(i, power[i]);
  }
}
void fireLED(int i, float power) {
  // 炎のLEDを点灯 power: 0~1
  if (power < 0) {
    power = 0.0;
  } else if (power > 1) {
    power = 1.0;
  }
  power = power * power; // 見た目の明るさがリニアに変わるように
  ledcWrite(LED_CH[i], 4095.0 * power + 0.5);
}

void displayPowerIndicator(float power) {
  // LEDマトリクスにpower(0~1)のインジケーター表示
  power = power * 25.0; // 0~25
  for (int i = 0; i < 25; i++) {
    if (i < power) {
      M5.dis.drawpix(i, 0x00ff00);  // GRB
    } else {
      M5.dis.drawpix(i, 0x000000);
    }
  }
}
void displayClear() {
  M5.dis.clear();
}
