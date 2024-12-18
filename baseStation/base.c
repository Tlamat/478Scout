//*****************************************************************************
//
// File Name	: 'base.c'
// Title		: Main loop and control of LCD BaseStation
// Author		: Richard MacGregor
// Created		: 11/29/2008
// Revised		: 12/2/2008
// Target MCU	: Microchip PIC18F4620
//
//*****************************************************************************

#include <18F4620.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP
#use delay(clock=20000000)

#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7)

#include <stdlib.h>
#include <string.h>
//#include <input.c>

#include "glcd.c"

//#define DEBUG

// null is = "\0"

// Global Variables

BOOLEAN button1 = FALSE;
BOOLEAN button2 = FALSE;

char string[8];

// coordinate strings and ints for walls
//char stringX[4];
//char stringY[4];
int x=0;
int y=0;
int xo=0;
int yo=0;

BOOLEAN Xhit;
BOOLEAN Yhit;
BOOLEAN XGO;
BOOLEAN YGO;

// coordinate strings and ints for rover
//char stringA[4];
//char stringB[4];
int a;
int b;

int i;
int j;

char temp;

// function prototypes
void lcdProcess(char *s);


void main() {
	button1 = FALSE;
	button2 = FALSE;

	Xhit=FALSE;
	Yhit=FALSE;
	XGO=FALSE;
	YGO=FALSE;

	ext_int_edge(0, H_TO_L); // Set up EXT0
	ext_int_edge(1, H_TO_L); // Set up EXT1

	enable_interrupts(INT_EXT);
	enable_interrupts(INT_EXT1);
	enable_interrupts(GLOBAL);

   output_low(PIN_B7);
   output_low(PIN_B6);

   glcd_init(ON);

 #ifdef DEBUG
   glcd_fillScreen(ON);
   delay_ms(3000);
   glcd_fillScreen(OFF);
   delay_ms(3000);
   glcd_pixel(0,0,ON);
   glcd_pixel(33,33,ON);
   delay_ms(3000);
   glcd_fillScreen(OFF);
#endif

	while(1){

		// get coordinates from scout and display them on lcd
		// packets should look like --> |x123y123|
		if(kbhit()){
			//gets(string);
			//lcdProcess(string);
			temp = getc();

			if(temp=='z'){
				output_high(PIN_B7);
			}
			else if(temp=='x' && !Xhit && !Yhit){
				Xhit=TRUE;
            // putc('a');
			}
			else if(temp=='y' && !Xhit && !Yhit){
				Yhit=TRUE;
            // putc('a');
			}
         else if(temp=='a'){
				Xhit=TRUE;
            // putc('a');
			}
			else if(temp=='b'){
				Yhit=TRUE;
            // putc('a');
			}
			else if(Xhit==TRUE){
				x=temp;
				Xhit=FALSE;
				XGO=TRUE;
            // putc('a');
			}
			else if(Yhit==TRUE){
				y=temp;
				Yhit=FALSE;
				YGO=TRUE;
            // putc('a');
			}
         //else if(temp=='e'){
           // XGO=TRUE;
           // YGO=TRUE;
            // putc('a');
         //}
		}

		if(XGO && YGO){
			glcd_pixel(x,y,ON);
			XGO=FALSE;
			YGO=FALSE;
		}

		if(button1){
			putc(0x07);	// go go gadget scout
         button1=FALSE;
		}
		else if(button2){
			putc(0x08);	// stop and put your hands in the sky
         button2=FALSE;
		}
      //output_low(PIN_B7);
      //output_low(PIN_B6);
	}
}


/**************************************************************************

Functions

*************************************************************************/
/*
void lcdProcess(char *s){
	if(strcmp(string[0], 'x')){
		i = 1;
      j = 0;
      output_high(PIN_B7);

		while(!strcmp(string[i], 'y')){
			stringX[i-1] = string[i];
			i++;
		}

		stringX[i]=NULL;


		while(!strcmp(string[i], 'e')){
			stringY[j]=string[i];
			i++;
         j++;
		}
		stringY[j]=NULL;

		x = atoi(stringX);
		y = atoi(stringY);
      output_high(PIN_B6);
		glcd_pixel(x,y,ON);
      xo=x;
      yo=y;

   }/*else if(strcmp(string[0], 'a')){
		i = 1;
      j = 0;

		while(!strcmp(string[i], 'b')){
			stringA[i-1] = string[i];
			i++;
		}

			stringA[i]=NULL;

		while(!strcmp(string[i], 'e')){
			stringB[j]=string[i];
			i++;
         j++;
		}

			stringB[j]=NULL;


		//glcd_pixel(a,b,OFF);		// last scout position delete

		a = atoi(stringA);
		b = atoi(stringB);

		glcd_pixel(a,b,ON);			// plot new scout position

	}*/

//}



/**************************************************************************

Interrupts

*************************************************************************/

// button 1 for start scout
#INT_EXT
EXT_ISR() {
	button1 = TRUE;
	button2 = FALSE;
}

// button 2 for stop scout
#INT_EXT1
EXT1_ISR() {
	button1 = FALSE;
	button2 = TRUE;
}
