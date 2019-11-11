int threshold = 40;
bool touch1detected = false;
bool touch2detected = false;

void gotTouch1(){
 touch1detected = true;
}

void gotTouch2(){
 touch2detected = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Interrupt Test");
  // PINO 32
  touchAttachInterrupt(T2, gotTouch1, threshold);
  // PINO 31
  touchAttachInterrupt(T3, gotTouch2, threshold);
}

void loop(){
  if(touch1detected){
    touch1detected = false;
    Serial.println("Touch 1 detected");
  }
  if(touch2detected){
    touch2detected = false;
    Serial.println("Touch 2 detected");
  }
}
