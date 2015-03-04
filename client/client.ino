const static int start_pin = 2;
const static int num = 11;
const static int each = 128/num;
const static int samples_max = 4;

int samples[samples_max];
int samples_p = 0;

void setup() {
  Serial.begin(115200);
  for(int i = 0; i < num; i++) {
    pinMode(i + start_pin, OUTPUT);
  }
  for(int i = 0; i < samples_max; i++) {
    samples[i] = 0;
  }
}

void loop() {
  if (Serial.available() > 0) {
    int val = Serial.read();
    val -= 128;
    samples[samples_p++] = val;
    if(samples_p == samples_max) {
      samples_p = 0;
    }
    val = 0;
    for(int i = 0; i < samples_max; i++) {
      val += samples[i];
    }
    val /= samples_max;
    for(int i = 0; i < num; i++) {
      int cur = max(min(8, val - each), 0) * 6;
      analogWrite(i + start_pin, cur);
      val -= each;
    }
  }
}
