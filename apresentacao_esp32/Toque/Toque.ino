#define TOUTCH_PIN T0 // ESP32 Pin D4
#define LED_PIN LED_BUILTIN
int touch_value = 100;

void setup()
{
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);
}

void loop()
{
  touch_value = touchRead(TOUTCH_PIN);
  Serial.println(touch_value);  // get value using T0/D1
  if (touch_value < 50)
  {
    digitalWrite (LED_PIN, HIGH);
  }
  else
  {
    digitalWrite (LED_PIN, LOW);
  }
  delay(100);
}
