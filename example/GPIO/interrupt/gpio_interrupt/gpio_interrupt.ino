int event_pin = 13;
int int_pin = 17;
int count=0;

void test_isr(void)
{
  count++;
  Serial.print("Interrupt! count=");
  Serial.println(count);
}

void setup() {
    Serial.print("int_pin=");
    Serial.println(int_pin);
    pinMode(event_pin, OUTPUT);
    pinMode(int_pin, INPUT_PULLUP);
    attachInterrupt(int_pin, test_isr, FALLING);
}

void loop() {
  if ( count >= 10 ) {
    detachInterrupt(int_pin);
  }
  digitalWrite(event_pin, HIGH);
  delay(500);
  digitalWrite(event_pin, LOW);
  delay(500);
}


