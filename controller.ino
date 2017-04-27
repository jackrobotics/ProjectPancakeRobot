#include <Servo.h>
#include <TimerOne.h>

#define DEBUG 1

#define MAGNET_PIN 14
#define limitY digitalRead(53)
#define limitA digitalRead(52)
#define limitB digitalRead(50)

Servo GRIPPER;

uint8_t *dataReceive;

struct STEPMOTOR
{
  int pin = 0;      //step pin
  int dir = 0;      //dir pin
  int en = 0;       //en pin
};

struct CONTROL
{
  int Y = 0;        //Step Y
  int A = 0;        //Step A
  int B = 0;        //Step B
  int S = 0;        //Servo
  bool M = false;   //Magnet
};

CONTROL Target;
CONTROL Position;

STEPMOTOR STEPPER_Y;
STEPMOTOR STEPPER_A;
STEPMOTOR STEPPER_B;

bool gotoHomePosition = false;


void setup()
{
  Serial.begin(115200);
  pinMode(MAGNET_PIN,OUTPUT);

  Timer1.initialize(100000);
  Timer1.attachInterrupt(serialRead);
  noInterrupts();

  STEPPER_Y.pin = 25;
  STEPPER_Y.dir = 27;
  STEPPER_Y.en = 23;
  pinMode(STEPPER_Y.pin,OUTPUT);
  pinMode(STEPPER_Y.dir,OUTPUT);
  pinMode(STEPPER_Y.en,OUTPUT);
  STEPPER_A.pin = 37;
  STEPPER_A.dir = 39;
  STEPPER_A.en = 35;
  pinMode(STEPPER_A.pin,OUTPUT);
  pinMode(STEPPER_A.dir,OUTPUT);
  pinMode(STEPPER_A.en,OUTPUT);
  STEPPER_B.pin = 31;
  STEPPER_B.dir = 33;
  STEPPER_B.en = 29;
  pinMode(STEPPER_B.pin,OUTPUT);
  pinMode(STEPPER_B.dir,OUTPUT);
  pinMode(STEPPER_B.en,OUTPUT);
  GRIPPER.attach(13);

  pinMode(9,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(7,INPUT_PULLUP);

  interrupts();
}

void serialRead()
{
  if(Serial.available()) protocol();
}

void loop()
{
  digitalWrite(STEPPER_Y.en,digitalRead(9));
  digitalWrite(STEPPER_A.en,digitalRead(8));
  digitalWrite(STEPPER_B.en,digitalRead(7));
  
  GRIPPER.write(Target.S);
  digitalWrite(MAGNET_PIN,Target.M? HIGH:LOW);

  
  if(gotoHomePosition)
  {
    if(limitY)
    {
      digitalWrite(STEPPER_Y.dir,HIGH);
      digitalWrite(STEPPER_Y.pin,HIGH);
    }
    if(limitA)
    {
      digitalWrite(STEPPER_A.dir,HIGH);
      digitalWrite(STEPPER_A.pin,HIGH);
    }
    if(limitB)
    {
      digitalWrite(STEPPER_B.dir,HIGH);
      digitalWrite(STEPPER_B.pin,HIGH);
    }
    delayMicroseconds(10000);
    if(limitY)digitalWrite(STEPPER_Y.pin,LOW);
    if(limitA)digitalWrite(STEPPER_A.pin,LOW);
    if(limitB)digitalWrite(STEPPER_B.pin,LOW);
    delayMicroseconds(1000);
    if(!limitY && !limitA && !limitB)
    {
      gotoHomePosition = false;
      Position.Y=0;
      Position.A=0;
      Position.B=0;
    }
    return;
  }

  if(!limitY)Position.Y=0;
  if(!limitA)Position.A=0;
  if(!limitB)Position.B=0;

  if(Target.Y-Position.Y != 0)
  {
    if(Target.Y-Position.Y >= 0 ){
      digitalWrite(STEPPER_Y.dir,LOW);
      Position.Y++;
    }else if(limitY){
      digitalWrite(STEPPER_Y.dir,HIGH);
      Position.Y--;
    }
    digitalWrite(STEPPER_Y.pin,HIGH);
  }else digitalWrite(STEPPER_Y.pin,LOW);


  if(Target.A-Position.A != 0)
  {
    if(Target.A-Position.A >= 0 ){
      digitalWrite(STEPPER_A.dir,HIGH);
      Position.A++;
    }else if(limitA){
      digitalWrite(STEPPER_A.dir,LOW);
      Position.A--;
    }
    digitalWrite(STEPPER_A.pin,HIGH);
  }else digitalWrite(STEPPER_A.pin,LOW);


  if(Target.B-Position.B != 0)
  {
    if(Target.B-Position.B >= 0 ){
      digitalWrite(STEPPER_B.dir,HIGH);
      Position.B++;
    }else if(limitB){
      digitalWrite(STEPPER_B.dir,LOW);
      Position.B--;
    }
    digitalWrite(STEPPER_B.pin,HIGH);
  }else digitalWrite(STEPPER_B.pin,LOW);
  delayMicroseconds(1500);
  if(Target.Y-Position.Y != 0)digitalWrite(STEPPER_Y.pin,LOW);
  if(Target.A-Position.A != 0)digitalWrite(STEPPER_A.pin,LOW);
  if(Target.B-Position.B != 0)digitalWrite(STEPPER_B.pin,LOW);
  delayMicroseconds(1000);
}


void protocol(){
  int count = 0;
  int sizeBuffer = Serial.available();
  dataReceive = malloc(sizeBuffer);

  while (Serial.available()){         // read all
    uint8_t dataraw = Serial.read();
    dataReceive[count++] = dataraw;
  }
  //FF FF 02 04 F9
  //FF FF 05 01 00 00 00 __
  int checksum = 0;
  for(int i=0;i<dataReceive[2]-1;i++) checksum+= dataReceive[3+i];
  checksum = (~checksum)&0xFF;
  
  // for(int i=0;i<count;i++)Serial.write(dataReceive[i]);
  // Serial.write(checksum);
  // Serial.write(dataReceive[dataReceive[2]+2]);
  // if(checksum&0xFF == dataReceive[dataReceive[2]+2]&0xFF)Serial.write(0x66);
  if((dataReceive[0] == 0xFF) && (dataReceive[1] == 0xFF) && (checksum == dataReceive[count-1]))  //check header package
  {
   
      switch(dataReceive[3])
      {
        case 0x01:  //y_position ,a_angle ,b_angle
          gotoHomePosition = false;
          Target.Y = (dataReceive[4]<<8)|(dataReceive[5]&0xFF);
          Target.A = (dataReceive[6]<<8)|(dataReceive[7]&0xFF);
          Target.B = (dataReceive[8]<<8)|(dataReceive[9]&0xFF);
          break;
        case 0x02:  //servo,magnat
          gotoHomePosition = false;
          Target.S = (int)dataReceive[4];
          ( dataReceive[5] == 0x01 )? Target.M = true : Target.M = false;
          break;
        case 0x03:  //report all
          byte dataSend[13];
          dataSend[0] = 0xFF;
          dataSend[1] = 0xFF;
          dataSend[2] = (byte)10;  // length
          dataSend[3] = (byte)3;  // instruction
          dataSend[4] = (byte)((Position.Y>>8)&0xFF);
          dataSend[5] = (byte)(Position.Y&0xFF);
          dataSend[6] = (byte)((Position.A>>8)&0xFF);
          dataSend[7] = (byte)(Position.A&0xFF);
          dataSend[8] = (byte)((Position.B>>8)&0xFF);
          dataSend[9] = (byte)(Position.B&0xFF);
          dataSend[10] = (byte)Target.S;
          dataSend[11] = Target.M? 0x01:0x00;
          dataSend[12] = ~(dataSend[3] + dataSend[4] + dataSend[5] + dataSend[6] + dataSend[7] + dataSend[8] + dataSend[9] + dataSend[10] + dataSend[11])&0xFF;
          for(int i = 0; i < 13 ; i++){
            Serial.write(dataSend[i]);
          }
          // Serial.write('\n'); Serial.write('\r');
          break;
        case 0x04:  //home
          gotoHomePosition = true;
          break;
        default:
          break;
      
    }
  }

  free(dataReceive);
}
