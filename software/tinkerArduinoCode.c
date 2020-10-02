#include <Servo.h>

#define LIVE 0
#define PLAYBACK 1
#define RECORD 2

double note2freq[] = {110, 116.5, 123.5, 130.8, 138.6, 146.8, 155.6, 164.8, 174.6, 185.0, 196.0, 207.7}; 

byte recordBuffer[1000];
int bufferIndex = 0;

int currentState;

// Servos
Servo servo_DOLL;
Servo servo_SPHERE;


// PERIPHERAL MECHANICS //

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

// Routine to add to the recording buffer, wipe if we exceed the end
void addToBuffer(int bin){
  recordBuffer[bufferIndex] = (byte) bin;
  bufferIndex++;
  if (bufferIndex >= 100){
  	bufferIndex = 0;
    for (int i = 0; i < 100; i++){
    recordBuffer[i] = 0;
  	}
  }
}

// STATE MECHANICS //

bool acquireState(){
  
  int buttonState = 0;
  int thisState = -1;
  
  // Which push button is pressed, if any?
  if (digitalRead(13) == HIGH){
    thisState = LIVE;
  } else if (digitalRead(12) == HIGH){
    thisState = PLAYBACK;
  } else if (digitalRead(11) == HIGH){
    thisState = RECORD;
  } else if (digitalRead(2) == HIGH){
    thisState = LIVE;
    bufferIndex = 0;
    for (int i = 0; i < 100; i++){
      recordBuffer[i] = 0;
  	}
  }
  
  // If state changed, report, and return true
  if (thisState != currentState && thisState != -1){
    currentState = thisState;
    Serial.print("State changed: ");
    Serial.println(currentState);
    return true;
    
    // ALSO will change the LED here
    // adjustLED();
  }
  
  return false;
}

void liveState(){
 
  
  if (digitalRead(10) == HIGH){  
  
      // grab reading from TOF
  
    long reading = readUltrasonicDistance(7, 7);

    // bin the TOF

    reading = constrain(reading, 1080, 19090); // TOF returns value from 1080 to 19090... map 
    int bin = map(reading, 1080, 19090, 0, 11);
    
    // grab reading from IMU
    // filter the reading
	// add value to note offset
    double whammy = 0;

    // grab reading from SOUND pushbutton
    
    // print note to serial
    Serial.print("Playing Note: ");
    Serial.println(bin);
    
    // execute note to speaker
    tone(8, note2freq[bin] + whammy, 100);
   
    // execute note to doll (bang bang)
    servo_DOLL.write(180);
    // execute note to hoberman (use note mapping)
    servo_SPHERE.write(map(bin, 0, 11, 0, 180));
    
  	// if recording, add to record circle buffer
    if (currentState == RECORD) {
 	   addToBuffer(bin);
    } 
  }else {
    servo_DOLL.write(0);
    servo_SPHERE.write(0);
    if (currentState == RECORD) {
 	   //addToBuffer(13);
    } 
  }
   delay(100);
   noTone(8);
}

void playbackState(){
 
  for (int i = 0; i < bufferIndex; i++){
    Serial.print(recordBuffer[i]);
  	// iterate through array and play back notes
  } 
  Serial.println("");
  
  delay(1000);
}

// DRIVER //

void setup()
{
  // Prepare serial to write to PC
  Serial.begin(9600);
  Serial.println("Hello from Hobermin");
  
  // Prepare intitial conditions
  currentState = LIVE;
  
  // Prepare pins
  pinMode(13, INPUT); // LIVE
  pinMode(12, INPUT); // PLAYBACK
  pinMode(11, INPUT); // RECORD
  
  pinMode(10, INPUT); // HIT NOTE
  pinMode(2, INPUT); // RESET
  
  for (int i = 0; i < 100; i++){
    recordBuffer[i] = 0;
  }
  
  servo_DOLL.attach(9);
  servo_SPHERE.attach(6);

}

void loop()
{
  // acquire state if changed, change state LED accordingly
  acquireState();
  
  // switch routine based on state
  switch (currentState){
    case LIVE:
    	liveState();
    	break;
    
    case RECORD:
    	liveState();
    	break;
    
    case PLAYBACK:
    	playbackState();
    	break;
         
  }
}
