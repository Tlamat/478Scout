/*******************************************************************************

Scout Code - Version 16

Working - starts up, takes commands
Not Working - Does not aquire sensor data

*******************************************************************************/


#include <18F4620.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP
#use delay(clock=20000000)

#include <math.h>
#include "angles.h"

#include "trigMulti.c"
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7)

#define angularVelocity    161 // mm/s

#define right 		0x03
#define left 		0x02
#define fwd			0x01
#define start     0x1A
#define stop      0x1B
#define recall    0x1C

#define ninety		500	//time motor on for 90 degree turn
#define forty5		250	//time motor on for 45 degree turn
#define go			1000 //time motor on for straight
#define sevenDeg  925
#define leftF		PIN_B7
#define leftR		PIN_B6
#define rightF		PIN_B5
#define rightR		PIN_B4

#define time2     300 // 300ms

// Global Variables

// sensors
char sensor = 'r';

boolean sensorSent = false;
boolean valueRecieved = false;
boolean scoutSent = false;
boolean timeout = true;
boolean startProgram = false;
boolean acked = false;


char captureValue = 'f';
char ack = 'n';
int16 rightDistance = 0;
int16 rightDistance2 = 0;
int16 rightDistance3 = 0;
int16 leftDistance = 0;
int16 leftDistance2 = 0;
int16 leftDistance3 = 0;
int16 fwdDistance = 0;
int16 fwdDistance2 = 0;
int16 fwdDistance3 = 0;

int positionTimerStart = 0;
int positionTimerEnd = 0;
int positionTimer = 0;
boolean timerGot = false;

char tempTemp = 0;

int msg = 0;
int i;
// timer
long int timer = 0;
boolean timerUP = false;

// Motor Drive
char command;
char turning = 'q';
// position
signed long int positionX = 10;
signed long int positionY = 10;

long int previousXposition = 10;
long int previousYposition = 10;
long int theta = 0;
char thetaC = 'e';

char updatePosition = 'n';
long delayTime = 0;
long moveCount = 0;

long int wheelTimer = 0;

// txData = 's' for scout and 'w' for wall, 'n' for waiting
char txData = 'n';
char txSensor;
long int sendX = 0;
long int sendY = 0;
long int xWall = 0;
long int yWall = 0;

long int pulse_width = 0;

// function prototypes
void sensor_init();
void readSensors();
void recieveRight();
void recieveLeft();
void recieveFwd();

void sendSensors();
void readRight();
void readLeft();
void readFwd();

void motorDrive();
void control(char);
void turnLeft(long);
void turnRight(long);
void forward();
void stopScout();

void wirelessCom();
void position();
//int trigMulti(int inputVal, int theta, char trig, signed int xMultiplier, signed int yMultiplier);
void wallPosition(long int baseX,long int baseY,long int theta, char sensor);
void getPosition(int,int);
char evalCoords(int,int,int);

void main() {
   // Initialize system

   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
	set_timer1(0xFC4F);               // 200us

   setup_timer_2(T2_DIV_BY_4, 250, 4); // 1 ms

   //setup_ccp1(CCP_CAPTURE_RE);
   //setup_ccp2(CCP_CAPTURE_FE);

   //set_tris_a(0xF0);
   //set_tris_c(0x86); // 1 in 0 out, 7-0
                        // 10000110

   enable_interrupts(INT_TIMER1);
   //enable_interrupts(INT_TIMER2);
   enable_interrupts(INT_CCP1);
   enable_interrupts(INT_CCP2);
   enable_interrupts(GLOBAL);

   putc('z');
   
	while(1) {
      if(startProgram) {
         sendSensors();
         readSensors();
         wirelessCom();

         if(updatePosition == 'y') {
            motorDrive();
            position();
            updatePosition = 'n';
         }
      } else {
         tempTemp = getc();
         control(tempTemp);
         
      }
   }

}

/****************************************

Scout Module - Motor Controls mxc1

****************************************/

void motorDrive() {

   // For automatic control

   // if you are approaching a wall in front of you
   if(fwdDistance <= 15) {
      // if the left wall is far away, turn left
      if(leftDistance >= 10) {
         turnLeft(sevenDeg);
         //evalCoords(l);
      // if the right wall and left wall are far way, turn left
      } else if(rightDistance >= 10 && leftDistance < 10) {
         turnRight(sevenDeg);
         //evalCoords(r);
      // the right and left wall are close, turn around
      } else if(rightDistance < 10 && leftDistance < 10) {
         for(i = 0;i < 12;i++) {
            turnRight(sevenDeg); // 180 degree turn, doesn't work
         }
      // stop
      } else {
         stopScout();
      }
   // The fwd wall is far away > 15cm
   } else {
      if(rightDistance > 30) {
         turnRight(sevenDeg);
         forward();
         delay_ms(500);
         stopScout();
         positionTimer = 500;
         position();
         //evalCoords(r);
      }else if(rightDistance < 10) {
         turnLeft(sevenDeg);
      } else {
         forward();
      }
   }

}


char evalCoords(int newC, int midC, int oldC) {
   char turn = 'n';
   // angled wall
   if(newC < midC && midC < oldC) {
      // getting closer to an angled wall
      turn = 'y';      
   } else if(newC > midC && midC > oldC) {
      // getting further from an angled wall
      turn = 'y'; // y = yes
   } else if(newC < midC && oldC < midC) {
      // turned too far
      turn = 'b'; // b = back
   }
   return turn;
}

void turnLeft(long degree){
   //if(turning != 'l') {
      //printf("turning left \r\n");
      output_high(rightF);
      output_high(leftR);
      output_low(rightR);
      output_low(leftF);
      //delay_ms(degree);
      //stopScout();
      delayTime = degree;
      moveCount = 0;
      enable_interrupts(INT_TIMER2);
      if(theta % 5 == 0) {
         theta = (theta + 7) % 360;
      } else {
         theta = (theta + 8) % 360;
      }
      //turning = 'l';
   //}
}
void turnRight(long degree){
  //if(turning != 'r') {
      //printf("turning right \r\n");
      output_high(leftF);
      output_high(rightR);
      output_low(leftR);
      output_low(rightF);
      //delay_ms(degree);
      //stopScout();
      delayTime = degree;
      moveCount = 0;
      enable_interrupts(INT_TIMER2);
      if(theta >= 7) {
         if(theta % 5 == 0) {
            theta = (theta - 8);
         } else {
            theta = (theta - 7);
         }
      } else {
         if(theta % 5 == 0) {
            theta = 360 + theta - 8;
         } else {
            theta = 360 + theta - 7;
         }
      }
     
      //stopScout();
      //delay_ms(degree);
      //turning = 'r';
  //}
}

void forward(){
   if(!timerGot) {
      positionTimerEnd = get_timer1();
      positionTimer = timer +positionTimerEnd - positionTimerStart;
      position();
      timer = 0;
      
   }
   positionTimerStart = get_timer1();
   //if(turning != 'f') {
   //printf("Going Forward \r\n");
   output_high(rightF);
   output_high(leftF);
   output_low(leftR);
   output_low(rightR);
   enable_interrupts(INT_TIMER2);
   //delay_ms(300);
   //stopScout();
   delayTime = 300;
   moveCount = 0;
   //delay_ms(go);
   //turning = 'f';
   //}
}
void stopScout(){
   output_low(rightF);
   output_low(rightR);
   output_low(leftF);
   output_low(leftR);
}

/****************************************
Sensor Control sc1
****************************************/

void sendSensors() {
   if('r' == sensor && !sensorSent) {
      readRight();
      /*output_low(PIN_A0);
      //delay_us(50);

      output_high(PIN_A3);
      delay_us(5);
      output_low(PIN_A3);

      output_high(PIN_A0);

      output_low(PIN_C3);

      setup_ccp1(CCP_CAPTURE_RE);
      setup_ccp2(CCP_CAPTURE_FE);

      sensorSent = true;
      timer = 0;*/

   } else if('l' == sensor && !sensorSent) {
      readLeft();
      /*output_low(PIN_A1);

      //delay_us(50);

      output_high(PIN_A3);
      delay_us(5);
      output_low(PIN_A3);

      output_high(PIN_A1);

      output_low(PIN_C4);

      setup_ccp1(CCP_CAPTURE_RE);
      setup_ccp2(CCP_CAPTURE_FE);

      sensorSent = true;
      timer = 0;*/

   } else if('f' == sensor && !sensorSent) {
      readFwd();         
      /*output_low(PIN_A2);

      //delay_us(50);

      output_high(PIN_A3);
      delay_us(5);
      output_low(PIN_A3);

      output_high(PIN_A2);

      output_low(PIN_C5);

      setup_ccp1(CCP_CAPTURE_RE);
      setup_ccp2(CCP_CAPTURE_FE);

      sensorSent = true;
      timer = 0;*/
      
   }/* else if(timer >= 150 && valueRecieved == false && txData == 'n') { // System is ready for another reading
      sensorSent = false;
      timer = 0;

      timeout = true;
      if(sensor == 'r') {
         sensor = 'f';
      } else if(sensor == 'f') {
         sensor = 'l';
      } else if(sensor == 'l') {
         sensor = 'r';
      } else {
         sensor = 'r';
      }
   } else if((timer == 100) && valueRecieved == false && timeout == true) { // No return signal
      pulse_width = 99999;
      valueRecieved = true;
      //printf("TIMEOUT \r\n");
   }*/
}

void readRight() {
   output_low(PIN_A0);
   //delay_us(50);
   
   output_high(PIN_A3);
   delay_us(5);
   output_low(PIN_A3);
   
   output_high(PIN_A0);
   
   output_low(PIN_C3);
   
   setup_ccp1(CCP_CAPTURE_RE);
   setup_ccp2(CCP_CAPTURE_FE);
   
   sensorSent = true;
   timer = 0;  
}

void readLeft() {
   output_low(PIN_A1);
   
   //delay_us(50);
   
   output_high(PIN_A3);
   delay_us(5);
   output_low(PIN_A3);
   
   output_high(PIN_A1);
   
   output_low(PIN_C4);
   
   setup_ccp1(CCP_CAPTURE_RE);
   setup_ccp2(CCP_CAPTURE_FE);
   
   sensorSent = true;
   timer = 0;

}

void readFwd() {
   output_low(PIN_A2);
   
   //delay_us(50);
   
   output_high(PIN_A3);
   delay_us(5);
   output_low(PIN_A3);
   
   output_high(PIN_A2);
   
   output_low(PIN_C5);
   
   setup_ccp1(CCP_CAPTURE_RE);
   setup_ccp2(CCP_CAPTURE_FE);
   
   sensorSent = true;
   timer = 0;
}

void readSensors() {
   if('r' == sensor && valueRecieved) {

      rightDistance3 = rightDistance2;
      rightDistance2 = rightDistance;
      rightDistance = pulse_width;

      //printf("Unmodded Right: %lu \r\n", rightDistance);
      rightDistance = pulse_width / 300;

      valueRecieved = false;
      timeout = false;
      sensorSent = false;
      output_high(PIN_C3);
      delay_ms(50);
      txData = 'r';
      txSensor = 'r';
      sensor = 'f';
   } else if('l' == sensor && valueRecieved) {

      leftDistance3 = leftDistance2;
      leftDistance2 = leftDistance;
      leftDistance = pulse_width;

      //printf("Unmodded Left: %lu \r\n", leftDistance);
      leftDistance = leftDistance / 300;

      valueRecieved = false;
      timeout = false;
      sensorSent = false;
      
      output_high(PIN_C4);

      delay_ms(50);

      txData = 'l';
      txSensor = 'l';
      sensor = 'r';
      updatePosition = 'y';
   } else if('f' == sensor && valueRecieved) {

      fwdDistance3 = fwdDistance2;
      fwdDistance2 = fwdDistance;
      fwdDistance = pulse_width;
      
      //printf("Unmodded Forward: %lu \r\n", fwdDistance);
      fwdDistance = fwdDistance / 300;

      valueRecieved = false;
      timeout = false;
      sensorSent = false;

      output_high(PIN_C5);
      
      delay_ms(50);
      
      txData = 'f';
      txSensor = 'f';
      sensor = 'l';
   }
}

void recieveRight() {
   
   rightDistance3 = rightDistance2;
   rightDistance2 = rightDistance;
   rightDistance = pulse_width / 300;
   
   printf("Right: %lu \r\n", rightDistance);
   
   valueRecieved = false;
      
   output_high(PIN_C3);
   
}

void recieveLeft() {

   leftDistance3 = leftDistance2;
   leftDistance2 = leftDistance;
   leftDistance = pulse_width / 300;
   
   printf("Left: %lu \r\n", leftDistance);
   
   valueRecieved = false;
   
   output_high(PIN_C4);
}

void recieveFwd() {

   fwdDistance3 = fwdDistance2;
   fwdDistance2 = fwdDistance;
   fwdDistance = pulse_width / 300;
   
   printf("Forward: %lu \r\n", fwdDistance);
   
   valueRecieved = false;
   
   output_high(PIN_C5);
}

/***********************************************************************

Scout Module - Position pos1

***********************************************************************/
void position() {

      previousXposition = positionX;
      previousYposition = positionY;
   
      getPosition(positionTimer,theta);
}


void wallPosition(long int baseX,long int baseY,long int theta, char sensor) {

   long int newTheta;

   newTheta = theta;

   while(newTheta > 90) {
      newTheta = newTheta - 90;
   }

   if(sensor == 'r') {
      if(theta <= 90) {
         xWall = baseX + trigMulti(rightDistance, newTheta, 's', 1, 1);
         yWall = baseY - trigMulti(rightDistance, newTheta, 'c', 1, 1);
      } else if(theta <= 180) {
         newTheta = 90 - newTheta;
         xWall = baseX + trigMulti(rightDistance, newTheta, 's', 1, 1);
         yWall = baseY + trigMulti(rightDistance, newTheta, 'c', 1, 1);
      } else if(theta <= 270) {
         xWall = baseX - trigMulti(rightDistance, newTheta, 's', 1, 1);
         yWall = baseY + trigMulti(rightDistance, newTheta, 'c', 1, 1);
      } else if(theta <= 360) {
         newTheta = 90 - newTheta;
         xWall = baseX - trigMulti(rightDistance, newTheta, 's', 1, 1);
         yWall = baseY - trigMulti(rightDistance, newTheta, 'c', 1, 1);
      }

   } else if(sensor == 'f') {
      if(theta < 90) {
         xWall = baseX + trigMulti(fwdDistance, newTheta, 's', 1, 1);
         yWall = baseY + trigMulti(fwdDistance, newTheta, 'c', 1, 1);
      } else if(theta < 180) {
         newTheta = 90 - newTheta;
         xWall = baseX - trigMulti(fwdDistance, newTheta, 's', 1, 1);
         yWall = baseY + trigMulti(fwdDistance, newTheta, 'c', 1, 1);
      } else if(theta < 270) {
         xWall = baseX - trigMulti(fwdDistance, newTheta, 's', 1, 1);
         yWall = baseY - trigMulti(fwdDistance, newTheta, 'c', 1, 1);
      } else if(theta < 360) {
         newTheta = 90 - newTheta;
         xWall = baseX + trigMulti(fwdDistance, newTheta, 's', 1, 1);
         yWall = baseY - trigMulti(fwdDistance, newTheta, 'c', 1, 1);
      }

   } else if(sensor == 'l') {
      if(theta < 90) {
         xWall = baseX - trigMulti(leftDistance, newTheta, 's', 1, 1);
         yWall = baseY + trigMulti(leftDistance, newTheta, 'c', 1, 1);
      } else if(theta < 180) {
         newTheta = 90 - newTheta;
         xWall = baseX - trigMulti(leftDistance, newTheta, 's', 1, 1);
         yWall = baseY - trigMulti(leftDistance, newTheta, 'c', 1, 1);
      } else if(theta < 270) {
         xWall = baseX + trigMulti(leftDistance, newTheta, 's', 1, 1);
         yWall = baseY - trigMulti(leftDistance, newTheta, 'c', 1, 1);
      } else if(theta < 360) {
         newTheta = 90 - newTheta;
         xWall = baseX + trigMulti(leftDistance, newTheta, 's', 1, 1);
         yWall = baseY + trigMulti(leftDistance, newTheta, 'c', 1, 1);
      }
   }
}


void getPosition(int time, int theta) {

   int newTheta = 0;
   newTheta = theta;

   while(newTheta > 90) {
      newTheta = newTheta - 90;
   }

   if(theta > 270) {

      newTheta = 90 - newTheta;

      positionX = trigMulti(angularVelocity * time / 100, newTheta,'s',-1,1) + previousXposition;
      positionY = trigMulti(angularVelocity * time / 100, newTheta,'c',-1,1) + previousYposition;

   } else if(theta > 180) {

		positionX = trigMulti(angularVelocity * time / 100, newTheta,'s',-1,-1) + previousXposition;
      positionY = trigMulti(angularVelocity * time / 100, newTheta,'c',-1,-1) + previousYposition;

   } else if(theta > 90) {
		newTheta = 90 - newTheta;

		positionX = trigMulti(angularVelocity * time / 100, newTheta,'s',1,-1) + previousXposition;
      positionY = trigMulti(angularVelocity * time / 100, newTheta,'c',1,-1) + previousYposition;

   } else { // theta >= 0

		positionX = trigMulti(angularVelocity * time / 100, newTheta,'s',1,1) + previousXposition;
      positionY = trigMulti(angularVelocity * time / 100, newTheta,'c',1,1) + previousYposition;

   }
}



/***********************************************************************

Wireless Communications wxc1

***********************************************************************/


void wirelessCom() {
   // Transmission
   if('r' == txData) {

		wallPosition(positionX,positionY,theta,txSensor);
      //printf("Right Distance: %lu \r\n", rightDistance);
      //printf("Scout X: %d Y: %d \r\n\r\n",positionX,positionY);
      //printf("x%ldy%lde",xWall,yWall);
      while(!acked) {
         putc('x');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(xWall);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';;
      while(!acked) {
         putc('y');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(yWall);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('a');
         while(!kbhit());
         ack = getc();
         if(ack == 'e') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      //printf("\r\n");
		txData = 'n';

	} else if('f' == txData) {

		wallPosition(positionX,positionY,theta,txSensor);
      //printf("Forward Distance: %lu \r\n", fwdDistance);
      //printf("Scout X: %d Y: %d \r\n\r\n",positionX,positionY);
      //printf("x%ldy%lde",xWall,yWall);
     while(!acked) {
         putc('x');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(xWall);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('y');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(yWall);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('a');
         while(!kbhit());
         ack = getc();
         if(ack == 'e') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      //printf("\r\n");
		txData = 'n';

	} else if('l' == txData) {

		wallPosition(positionX,positionY,theta,txSensor);
      //printf("Left Distance: %lu \r\n", leftDistance);
      //printf("Scout X: %lu Y: %lu \r\n\r\n",positionX,positionY);
      //printf("x%ldy%lde",xWall,yWall);
      while(!acked) {
         putc('x');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(xWall);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('y');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(yWall);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('a');
         while(!kbhit());
         ack = getc();
         if(ack == 'e') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      //printf("\r\n");
      //printf("a%ldb%lde",positionX,positionY);
      while(!acked) {
         putc('a');
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(positionX);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('a');
         while(!kbhit());
         ack = getc();
         if(ack == 'b') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc(positionY);
         while(!kbhit());
         ack = getc();
         if(ack == 'a') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      while(!acked) {
         putc('a');
         while(!kbhit());
         ack = getc();
         if(ack == 'e') {
            acked = true;
         }
      }
      acked = false;
      ack = 'n';
      //printf("\r\n");
   	txData = 'n';

	}

	// Recieving
	if(kbhit()) {
		// decode msg
		msg = getc();
		// perform action
		control(msg);
   }

}

void control(char c){
   if(c==0x01){
      // turn left
      turnLeft(sevenDeg);
      printf("Seven Degrees Left\r\n");

   }else if(c==0x02){
      // Read In All Three Sensors
      readRight();
      while(!valueRecieved);
      recieveRight();
      delay_ms(50);
      readLeft();
      while(!valueRecieved);
      recieveLeft();
      delay_ms(50);
      readFwd();
      while(!valueRecieved);
      recieveFwd();
   }else if(c==0x03){
      // Turn Right
      turnRight(sevenDeg);
      printf("Seven Degrees Right \r\n");
   }else if(c==0x04){
      // Go Forward
      forward();
      delay_ms(500);
      stopScout();
      printf("Foward \r\n");
   }else if(c==0x05){
      // Update Position
      updatePosition = 'y';
      //printf("Pos X: %lu \r\nPos Y: %lu \r\ntheta: %lu \r\n",positionX,positionY,theta);
      
      position();
      
      //printf("Pos X: %lu \r\nPos Y: %lu \r\ntheta: %lu \r\n",positionX,positionY,theta);
      
   } else if(c == 0x06) {
      // Debug
      printf("Sensor Active: %c \r\n", sensor);
      printf("Sensor Sent? %d \r\n",sensorSent);
      printf("Value Recieved? %d \r\n", valueRecieved);
      printf("Timer: %ld \r\n", timer);
   } else if(c ==  0x07) {
      // Start
      startProgram = true;
   }else if(c == 0x08) {
      // Stop
      startProgram = false;
   } else if(c == 0x09) {
      printf("Foward Further \r\n");
      forward();
      delay_ms(500);
      stopScout();
   } else if(c == 0x10) {
      printf("x30y30e");
   } else if(c == 0x1A) {
      printf("x31y31e");
   } else if(c == 0x1B) {
      printf("x32y32e"); 
   } else if(c == 0x1C) {
      printf("x33y33e"); 
   } else if(c == 0x1D) {
      printf("x34y34e");
   } else if(c == 0x1E) {
      printf("x35y35e");
   } else {
      stopScout();
      printf("bad command \r\n");
   }
}


/**************************************************************************

Interrupts

*************************************************************************/

#INT_TIMER1
void timer1_ISR() {
   timer = timer + 2;
   if(timer == 500) {
      pulse_width = 99999;
      valueRecieved = true;
      setup_ccp1(CCP_OFF);
      setup_ccp2(CCP_OFF);
   }
}

#INT_TIMER2
void timer2_ISR() {
  if(moveCount == delayTime) {
      stopScout();
      disable_interrupts(INT_TIMER2);
      timerGot = true;        
  } else {
     moveCount++;
  }   
}

#INT_CCP1
void ccp1_ISR() {
     timer = 0;
}

#INT_CCP2
void ccp2_ISR() {
      pulse_width = (timer * 100) + CCP_2 - CCP_1;
      valueRecieved = true;
      setup_ccp1(CCP_OFF);
      setup_ccp2(CCP_OFF);
}




