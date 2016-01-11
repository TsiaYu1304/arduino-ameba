#include "libtest.h"

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // prints title with ending line break
  Serial.println("Fabonacci");
}

static int a=2;
static int b=3;
void loop() {
  int c;
  c = mysum(a, b);
  Serial.print(c);
  Serial.print(" ");

  a=b;
  b=c;
  delay(500);
}
