/////////////////////////////////////////////////////////////////////////
////                           GLCD.C                                ////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2003 Custom Computer Services         ////
//// This source code may only be used by licensed users of the CCS  ////
//// C compiler.  This source code may only be distributed to other  ////
//// licensed users of the CCS C compiler.  No other use,            ////
//// reproduction or distribution is permitted without written       ////
//// permission.  Derivative programs created using this software    ////
//// in object code form are not restricted in any way.              ////
/////////////////////////////////////////////////////////////////////////
////                                                                 ////
////	Revised by:  Chris Williams									 ////
////	Date:  12/1/08												 ////
/////////////////////////////////////////////////////////////////////////





#ifndef GLCD_C
#define GLCD_C
#include "glcd.h"


// Purpose:       Initialize a graphic LCD. This must be called before any
//                other glcd function is used.
// Inputs:        The initialization mode
//                OFF - Turns the LCD off
//                ON  - Turns the LCD on
// Date:          5/28/2003
void glcd_init(int1 mode)
{
   // Initialze some pins
   output_high(GLCD_RST);
   output_low(GLCD_E);
   output_low(GLCD_CS1);
   output_low(GLCD_CS2);

   output_low(GLCD_DI);                // Set for instruction
   glcd_writeByte(GLCD_CS1, 0xC0);     // Specify first RAM line at the top
   glcd_writeByte(GLCD_CS2, 0xC0);     //   of the screen
   glcd_writeByte(GLCD_CS1, 0x40);     // Set the column address to 0
   glcd_writeByte(GLCD_CS2, 0x40);
   glcd_writeByte(GLCD_CS1, 0xB8);     // Set the page address to 0
   glcd_writeByte(GLCD_CS2, 0xB8);
   if(mode == ON)
   {
      glcd_writeByte(GLCD_CS1, 0x3F);  // Turn the display on
      glcd_writeByte(GLCD_CS2, 0x3F);
   }
   else
   {
      glcd_writeByte(GLCD_CS1, 0x3E);  // Turn the display off
      glcd_writeByte(GLCD_CS2, 0x3E);
   }

   glcd_fillScreen(OFF);               // Clear the display
}


// Purpose:       Turn a pixel on a graphic LCD on or off
// Inputs:        x - the x coordinate of the pixel
//                y - the y coordinate of the pixel
//                color - ON or OFF
// Output:        1 if coordinate out of range, 0 if in range
void glcd_pixel(int x, int y, int1 color)
{
   BYTE data;
   BYTE chip = GLCD_CS1;  // Stores which chip to use on the LCD

   if(x > 63)  // Check for first or second display area
   {
      x -= 64;
      chip = GLCD_CS2;
   }

   output_low(GLCD_DI);                                     // Set for instruction
   bit_clear(x,7);                                          // Clear the MSB. Part of an instruction code
   bit_set(x,6);                                            // Set bit 6. Also part of an instruction code
   glcd_writeByte(chip, x);                                 // Set the horizontal address
   glcd_writeByte(chip, (y/8 & 0b10111111) | 0b10111000);   // Set the vertical page address
   output_high(GLCD_DI);                                    // Set for data
   data = glcd_readByte(chip);

   if(color == ON)
      bit_set(data, y%8);        // Turn the pixel on
   else                          // or
      bit_clear(data, y%8);      // turn the pixel off
   output_low(GLCD_DI);          // Set for instruction
   glcd_writeByte(chip, x);      // Set the horizontal address
   output_high(GLCD_DI);         // Set for data
   glcd_writeByte(chip, data);   // Write the pixel data
}


// Purpose:       Draw a line on a graphic LCD using Bresenham's
//                line drawing algorithm
// Inputs:        (x1, y1) - the start coordinate
//                (x2, y2) - the end coordinate
//                color - ON or OFF
// Dependencies:  glcd_pixel()
void glcd_line(int x1, int y1, int x2, int y2, int1 color)
{
   signed int  x, y, addx, addy, dx, dy;
   signed long P;
   int i;
   dx = abs((signed int)(x2 - x1));
   dy = abs((signed int)(y2 - y1));
   x = x1;
   y = y1;

   if(x1 > x2)
      addx = -1;
   else
      addx = 1;
   if(y1 > y2)
      addy = -1;
   else
      addy = 1;

   if(dx >= dy)
   {
      P = 2*dy - dx;

      for(i=0; i<=dx; ++i)
      {
         glcd_pixel(x, y, color);

         if(P < 0)
         {
            P += 2*dy;
            x += addx;
         }
         else
         {
            P += 2*dy - 2*dx;
            x += addx;
            y += addy;
         }
      }
   }
   else
   {
      P = 2*dx - dy;

      for(i=0; i<=dy; ++i)
      {
         glcd_pixel(x, y, color);

         if(P < 0)
         {
            P += 2*dx;
            y += addy;
         }
         else
         {
            P += 2*dx - 2*dy;
            x += addx;
            y += addy;
         }
      }
   }
}


// Purpose:       Draw a rectangle on a graphic LCD
// Inputs:        (x1, y1) - the start coordinate
//                (x2, y2) - the end coordinate
//                fill  - YES or NO
//                color - ON or OFF
// Dependencies:  glcd_pixel(), glcd_line()
void glcd_rect(int x1, int y1, int x2, int y2, int fill, int1 color)
{
   if(fill)
   {
      int y, ymax;                          // Find the y min and max
      if(y1 < y2)
      {
         y = y1;
         ymax = y2;
      }
      else
      {
         y = y2;
         ymax = y1;
      }

      for(; y<=ymax; ++y)                    // Draw lines to fill the rectangle
         glcd_line(x1, y, x2, y, color);
   }
   else
   {
      glcd_line(x1, y1, x2, y1, color);      // Draw the 4 sides
      glcd_line(x1, y2, x2, y2, color);
      glcd_line(x1, y1, x1, y2, color);
      glcd_line(x2, y1, x2, y2, color);
   }
}


// Purpose:       Draw a bar (wide line) on a graphic LCD
// Inputs:        (x1, y1) - the start coordinate
//                (x2, y2) - the end coordinate
//                width  - The number of pixels wide
//                color - ON or OFF
void glcd_bar(int x1, int y1, int x2, int y2, int width, int1 color)
{
   signed int  x, y, addx, addy, j;
   signed long P, dx, dy, c1, c2;
   int i;
   dx = abs((signed int)(x2 - x1));
   dy = abs((signed int)(y2 - y1));
   x = x1;
   y = y1;
   c1 = -dx*x1 - dy*y1;
   c2 = -dx*x2 - dy*y2;

   if(x1 > x2)
   {
      addx = -1;
      c1 = -dx*x2 - dy*y2;
      c2 = -dx*x1 - dy*y1;
   }
   else
      addx = 1;
   if(y1 > y2)
   {
      addy = -1;
      c1 = -dx*x2 - dy*y2;
      c2 = -dx*x1 - dy*y1;
   }
   else
      addy = 1;

   if(dx >= dy)
   {
      P = 2*dy - dx;

      for(i=0; i<=dx; ++i)
      {
         for(j=-(width/2); j<width/2+width%2; ++j)
         {
            if(dx*x+dy*(y+j)+c1 >= 0 && dx*x+dy*(y+j)+c2 <=0)
               glcd_pixel(x, y+j, color);
         }
         if(P < 0)
         {
            P += 2*dy;
            x += addx;
         }
         else
         {
            P += 2*dy - 2*dx;
            x += addx;
            y += addy;
         }
      }
   }
   else
   {
      P = 2*dx - dy;

      for(i=0; i<=dy; ++i)
      {
         if(P < 0)
         {
            P += 2*dx;
            y += addy;
         }
         else
         {
            P += 2*dx - 2*dy;
            x += addx;
            y += addy;
         }
         for(j=-(width/2); j<width/2+width%2; ++j)
         {
            if(dx*x+dy*(y+j)+c1 >= 0 && dx*x+dy*(y+j)+c2 <=0)
               glcd_pixel(x+j, y, color);
         }
      }
   }
}


// Purpose:       Draw a circle on a graphic LCD
// Inputs:        (x,y) - the center of the circle
//                radius - the radius of the circle
//                fill - YES or NO
//                color - ON or OFF
void glcd_circle(int x, int y, int radius, int1 fill, int1 color)
{
   signed int a, b, P;
   a = 0;
   b = radius;
   P = 1 - radius;

   do
   {
      if(fill)
      {
         glcd_line(x-a, y+b, x+a, y+b, color);
         glcd_line(x-a, y-b, x+a, y-b, color);
         glcd_line(x-b, y+a, x+b, y+a, color);
         glcd_line(x-b, y-a, x+b, y-a, color);
      }
      else
      {
         glcd_pixel(a+x, b+y, color);
         glcd_pixel(b+x, a+y, color);
         glcd_pixel(x-a, b+y, color);
         glcd_pixel(x-b, a+y, color);
         glcd_pixel(b+x, y-a, color);
         glcd_pixel(a+x, y-b, color);
         glcd_pixel(x-a, y-b, color);
         glcd_pixel(x-b, y-a, color);
      }

      if(P < 0)
         P+= 3 + 2*a++;
      else
         P+= 5 + 2*(a++ - b--);
    } while(a <= b);
}

// Purpose:       Write text on a graphic LCD
// Inputs:        (x,y) - The upper left coordinate of the first letter
//                textptr - A pointer to an array of text to display
//                size - The size of the text: 1 = 5x7, 2 = 10x14, ...
//                color - ON or OFF
void glcd_text57(int x, int y, char* textptr, int size, int1 color)
{
   int i, j, k, l, m;                     // Loop counters
   BYTE pixelData[5];                     // Stores character data

   for(i=0; textptr[i] != '\0'; ++i, ++x) // Loop through the passed string
   {
      if(textptr[i] < 'S') // Checks if the letter is in the first text array
         memcpy(pixelData, TEXT[textptr[i]-' '], 5);
      else if(textptr[i] <= '~') // Check if the letter is in the second array
         memcpy(pixelData, TEXT2[textptr[i]-'S'], 5);
      else
         memcpy(pixelData, TEXT[0], 5);   // Default to space

      if(x+5*size >= GLCD_WIDTH)          // Performs character wrapping
      {
         x = 0;                           // Set x at far left position
         y += 7*size + 1;                 // Set y at next position down
      }
      for(j=0; j<5; ++j, x+=size)         // Loop through character byte data
      {
         for(k=0; k<7*size; ++k)          // Loop through the vertical pixels
         {
            if(bit_test(pixelData[j], k)) // Check if the pixel should be set
            {
               for(l=0; l<size; ++l)      // The next two loops change the
               {                          // character's size
                  for(m=0; m<size; ++m)
                  {
                     glcd_pixel(x+m, y+k*size+l, color); // Draws the pixel
                  }
               }
            }
         }
      }
   }
}


// Purpose:       Fill the LCD screen with the passed in color.
//                Works much faster than drawing a rectangle to fill the screen.
// Inputs:        ON - turn all the pixels on
//                OFF - turn all the pixels off
// Dependencies:  glcd_writeByte()
void glcd_fillScreen(int1 color)
{
   int i, j;

   // Loop through the vertical pages
   for(i = 0; i < 8; ++i)
   {
      output_low(GLCD_DI);                      // Set for instruction
      glcd_writeByte(GLCD_CS1, 0b01000000);     // Set horizontal address to 0
      glcd_writeByte(GLCD_CS2, 0b01000000);
      glcd_writeByte(GLCD_CS1, i | 0b10111000); // Set page address
      glcd_writeByte(GLCD_CS2, i | 0b10111000);
      output_high(GLCD_DI);                     // Set for data

      // Loop through the horizontal sections
      for(j = 0; j < 64; ++j)
      {
         glcd_writeByte(GLCD_CS1, 0xFF*color);  // Turn pixels on or off
         glcd_writeByte(GLCD_CS2, 0xFF*color);  // Turn pixels on or off
      }
   }
}

// Purpose:       Write a byte of data to the specified chip
// Inputs:        chipSelect - which chip to write the data to
//                data - the byte of data to write
glcd_writeByte(BYTE chip, BYTE data)
{
   if(chip == GLCD_CS1)       // Choose which chip to write to
      output_high(GLCD_CS1);
   else
      output_high(GLCD_CS2);

   output_low(GLCD_RW);       // Set for writing
   output_D(data);            // Put the data on the port
   output_high(GLCD_E);       // Pulse the enable pin
   delay_us(2);
   output_low(GLCD_E);

   output_low(GLCD_CS1);      // Reset the chip select lines
   output_low(GLCD_CS2);
}


// Purpose:       Reads a byte of data from the specified chip
// Ouputs:        A byte of data read from the chip
BYTE glcd_readByte(BYTE chip)
{
   BYTE data;                 // Stores the data read from the LCD
   if(chip == GLCD_CS1)       // Choose which chip to read from
      output_high(GLCD_CS1);
   else
      output_high(GLCD_CS2);

   input_D();                 // Set port d to input
   output_high(GLCD_RW);      // Set for reading
   output_high(GLCD_E);       // Pulse the enable pin
   delay_us(2);
   output_low(GLCD_E);
   delay_us(2);
   output_high(GLCD_E);       // Pulse the enable pin
   delay_us(2);
   data = input_D();          // Get the data from the display's output register
   output_low(GLCD_E);

   output_low(GLCD_CS1);      // Reset the chip select lines
   output_low(GLCD_CS2);
   return data;               // Return the read data
}

#endif
