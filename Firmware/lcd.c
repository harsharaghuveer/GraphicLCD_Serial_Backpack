#include<avr/io.h>
#include "glcdbp.h"
#include "ks0108b.h"
#include "lcd.h"

extern enum DISPLAY_TYPE display;

void lcdConfig(void)
{
	ks0108bReset();
	ks0108bDisplayOn();
	ks0108bClear();
}

void lcdSetData(uint8_t data)
{
	DDRB |= 0x03;
	DDRD |= 0xFC;
	
	PORTB &= 0xFC;					// Clear PB7:2 in preparation for data.
	PORTD &= 0x03;					// Clear PD1:0.
	
	PORTB |= (data & 0x03);			// Mask off PB1:0 so we don't change them
									//  and then write the other 6 bits.
	PORTD |= (data & 0xFC);			// Mask off PD7:2 so we don't change them
									//	and then write the other two bits.
									//  The data is now in place.
}

uint8_t lcdReadData(void)
{
	DDRB &= ~(0x03);
	DDRD &= ~(0xFC);
	
	uint8_t data = (PINB & 0x03);
	data |= (PIND & 0xFC);
	
	return data;
}


 // Draws a line between two points p1(p1x,p1y) and p2(p2x,p2y).
 // This function is based on the Bresenham's line algorithm and is highly 
 // optimized to be able to draw lines very quickly. There is no floating point 
 // arithmetic nor multiplications and divisions involved. Only addition, 
 // subtraction and bit shifting are used. 

 // Note that you have to define your own customized lcdDrawPixel(x,y) function, 
 // which essentially lights a pixel on the screen.
 
 // This code adopted from code originally posted to codekeep.net
 //	 (http://www.codekeep.net/snippets/e39b2d9e-0843-4405-8e31-44e212ca1c45.aspx)
 //	 by Woon Khang Tang on 1/29/2009.
 
void lcdDrawLine(uint8_t p1x, uint8_t p1y, uint8_t p2x, uint8_t p2y)
{
    uint8_t F, x, y;

    if (p1x > p2x)  // Swap points if p1 is on the right of p2
    {
		x = p1x;
		p1x = p2x;
		p2x = x;
		y = p1y;
		p1y = p2y;
		p2y = y;
    }

    // Handle trivial cases separately for algorithm speed up.
    // Trivial case 1: m = +/-INF (Vertical line)
    if (p1x == p2x)
    {
        if (p1y > p2y)  // Swap y-coordinates if p1 is above p2
        {
			y = p1y;
			p1y = p2y;
			p2y = y;
        }

        x = p1x;
        y = p1y;
        while (y <= p2y)
        {
            lcdDrawPixel(x, y, ON);
            y++;
        }
        return;
    }
    // Trivial case 2: m = 0 (Horizontal line)
    else if (p1y == p2y)
    {
        x = p1x;
        y = p1y;

        while (x <= p2x)
        {
            lcdDrawPixel(x, y, ON);
            x++;
        }
        return;
    }


    uint8_t dy            = p2y - p1y;  // y-increment from p1 to p2
    uint8_t dx            = p2x - p1x;  // x-increment from p1 to p2
    uint8_t dy2           = (dy << 1);  // dy << 1 == 2*dy
    uint8_t dx2           = (dx << 1);
    uint8_t dy2_minus_dx2 = dy2 - dx2;  // precompute constant for speed up
    uint8_t dy2_plus_dx2  = dy2 + dx2;


    if (dy >= 0)    // m >= 0
    {
        // Case 1: 0 <= m <= 1 (Original case)
        if (dy <= dx)   
        {
            F = dy2 - dx;    // initial F

            x = p1x;
            y = p1y;
            while (x <= p2x)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F += dy2;
                }
                else
                {
                    y++;
                    F += dy2_minus_dx2;
                }
                x++;
            }
        }
        // Case 2: 1 < m < INF (Mirror about y=x line
        // replace all dy by dx and dx by dy)
        else
        {
            F = dx2 - dy;    // initial F

            y = p1y;
            x = p1x;
            while (y <= p2y)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F += dx2;
                }
                else
                {
                    x++;
                    F -= dy2_minus_dx2;
                }
                y++;
            }
        }
    }
    else    // m < 0
    {
        // Case 3: -1 <= m < 0 (Mirror about x-axis, replace all dy by -dy)
        if (dx >= -dy)
        {
            F = -dy2 - dx;    // initial F

            x = p1x;
            y = p1y;
            while (x <= p2x)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F -= dy2;
                }
                else
                {
                    y--;
                    F -= dy2_plus_dx2;
                }
                x++;
            }
        }
        // Case 4: -INF < m < -1 (Mirror about x-axis and mirror 
        // about y=x line, replace all dx by -dy and dy by dx)
        else    
        {
            F = dx2 + dy;    // initial F

            y = p1y;
            x = p1x;
            while (y >= p2y)
            {
                lcdDrawPixel(x, y, ON);
                if (F <= 0)
                {
                    F += dx2;
                }
                else
                {
                    x++;
                    F += dy2_plus_dx2;
                }
                y--;
            }
        }
    }
}

void lcdDrawPixel(uint8_t x, uint8_t y, PIX_VAL pixel)
{
	if (display == SMALL)
	{
		ks0108bDrawPixel(x, y, pixel);
	}
}
