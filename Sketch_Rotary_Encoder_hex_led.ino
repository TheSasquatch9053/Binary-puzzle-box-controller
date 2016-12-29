#include <Servo.h>


/*******Interrupt-based Rotary Encoder Sketch*******
by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence
modified by TheSasquatch to create a 8 position dial lock test
*/
 
Servo lockServo; // Define lockServo 

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2 - used for rotary encoder
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3 - used for rotary encoder

static byte Hex0 = 12; //Output pin for bit 1 of hex code output
static byte Hex1 = 11; //Output pin for bit 2 of hex code output
static byte Hex2 = 13; //Output pin for bit 3 of hex code output

static byte Hexin0 = 9; //Output pin for bit 1 of hex code input
static byte Hexin1 = 8; //Output pin for bit 1 of hex code input
static byte Hexin2 = 10; //Output pin for bit 1 of hex code input

static byte maxDial = 8; // highest number of dial
static byte minDial = 0; // lowest number on dial
static boolean val = HIGH; // Status of pinClick
static int masterCode[] = {6,6,2}; // lock combination
static int codeInput[3] = {0,0,0}; // initial array for combination input
static boolean passFail[5] = {false, false, false}; //array for collecting combination test results
static int count = 0; // starting array input location
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 1; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 1; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

void setup() {
  lockServo.attach(5,400,2400); // attach lock actuation servo to pin 5
  lockServo.write(95); // default to middle
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage used for rotary encoder
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage used for rotary encoder
  
  pinMode(6, INPUT_PULLUP); //set pinClick as in input, pulled HIGH to the logic voltage - used for rotary encoder click
  
  pinMode(Hex0,OUTPUT); //set hex output pin 1 as LED output
  pinMode(Hex1,OUTPUT); //set hex output pin 2 as LED output
  pinMode(Hex2,OUTPUT); //set hex output pin 3 as LED output

  pinMode(Hexin0,OUTPUT); //set hex output pin 1 as LED output
  pinMode(Hexin1,OUTPUT); //set hex output pin 2 as LED output
  pinMode(Hexin2,OUTPUT); //set hex output pin 3 as LED output
  
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(115200); // start the serial monitor link
 
}

void PinA(){  //I copied this except for the part that tests encoder position and limits it to dial values...
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
     if (encoderPos < minDial) // test if encoder position goes negative
    {
      encoderPos = maxDial;   // set encoder position to maximum dial number
    }
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){  //I copied this except for the part that tests encoder position and limits it to dial values...
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    if(encoderPos > maxDial) // test if encoder position goes negative
    {
      encoderPos = minDial;   // set encoder position to maximum dial number
    }
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

// I wrote the loop!

void loop(){
   //lockServo.write(100)
   Serial.println(lockServo.read(), DEC);
  if(oldEncPos != encoderPos) { //Test if dial has been turned and print new dial value
    Serial.println(encoderPos, DEC);
    digitalWrite(Hexin0,bitRead(encoderPos,0));
    digitalWrite(Hexin1,bitRead(encoderPos,1));
    digitalWrite(Hexin2,bitRead(encoderPos,2));
    oldEncPos = encoderPos;    
  }
  val = digitalRead(6);
  if(val == LOW) 
  {
    codeInput[count] = encoderPos;
    digitalWrite(Hex0,bitRead(encoderPos,0));
    digitalWrite(Hex1,bitRead(encoderPos,1));
    digitalWrite(Hex2,bitRead(encoderPos,2));
    count++;
    int i;
    for (i = 0; i < 3; i++) 
    {
       Serial.print(codeInput[i], DEC);
       Serial.println("+");
    }
    delay(1000);
    digitalWrite(Hex0,0);
    digitalWrite(Hex1,0);
    digitalWrite(Hex2,0);
    delay(200);
    if(count == 1)
    {
      digitalWrite(Hex0,1);
      delay(200);
    }
    else if (count == 2)
    {
      digitalWrite(Hex0,1);
      digitalWrite(Hex1,1);
      delay(200);
    }
    else if(count ==3)
    {
      digitalWrite(Hex0,1);
      digitalWrite(Hex1,1);
      digitalWrite(Hex2,1);
      delay(200);
    }
  }
  if(count == 3) 
  {
    int a;
    for (a = 0; a < 3; a++) 
    {
      if (codeInput[a]== masterCode[a]) 
      {
        passFail[a] = true;
      }
      else 
      {
        Serial.println("You have failed...");
        int i;
      for (i=0;i<3; i++)
        {
        digitalWrite(Hex0,1);
        digitalWrite(Hex1,1);
        digitalWrite(Hex2,1);
        delay(200);
        digitalWrite(Hex0,0);
        digitalWrite(Hex1,0);
        digitalWrite(Hex2,0);
        delay(200);
        }
      break;
      }
    Serial.println("Success!");
    lockServo.write(80);
    int i;
    for (i=0;i<3; i++)
    {
    digitalWrite(Hexin0,1);
    digitalWrite(Hexin1,1);
    digitalWrite(Hexin2,1);
    delay(200);
    digitalWrite(Hexin0,0);
    digitalWrite(Hexin1,0);
    digitalWrite(Hexin2,0);
    delay(200);
    }
    digitalWrite(Hex0,0);
    digitalWrite(Hex1,0);
    digitalWrite(Hex2,0); 
   }
    count=0;
    memset(passFail,false,sizeof(passFail)); 
    memset(codeInput,0,sizeof(codeInput));
  } 
  
}
