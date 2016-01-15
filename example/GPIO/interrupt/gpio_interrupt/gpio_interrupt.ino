int event_pin = 2;
int int_pin = 13;
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
    pinMode(int_pin, INPUT);
    digitalWrite(event_pin, HIGH);
    attachInterrupt(int_pin, test_isr, RISING);
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


