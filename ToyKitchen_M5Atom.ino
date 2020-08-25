#include <M5Atom.h>
#include <math.h>

const int VOLUME_PIN = 33;
const int VOLUME_SW = 23;
const int LED_NUM = 4;
const int LED_PIN[LED_NUM] = {25, 21, 19, 22};
const int LED_CH[LED_NUM] = {0, 1, 2, 3};

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
  if (digitalRead(VOLUME_SW) == LOW) {
    int vol = analogRead(VOLUME_PIN);
    Serial.println(vol);
    for (int i = 0; i < LED_NUM; i++) {
      float rate = randomRate();
      int v = vol * rate / 2; // PWM MAXだと明るすぎるので半分に落とした
      if (v > 4095) {
        v = 4095;
      }
      ledcWrite(LED_CH[i], v);
    }
    float vol_linear = sqrt(vol); // 0~63.99
    int power = 25.0 * vol_linear / 64.0 + 0.5; // 0 - 25
    Serial.println(power);
    for (int i = 0; i < 25; i++) {
      if(i < power) {
        M5.dis.drawpix(i, 0x00ff00);  // GRB
      } else {
        M5.dis.drawpix(i, 0x000000);
      }
    }
  } else {
    for (int i = 0; i < LED_NUM; i++) {
      ledcWrite(LED_CH[i], 0);
      M5.dis.clear();
    }
  }
  delay(100);
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
