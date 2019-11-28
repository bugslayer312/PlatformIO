/*
  LCD5110_Graph.cpp - Arduino/chipKit library support for Nokia 5110 compatible LCDs
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
  
  Basic functionality of this library are based on the demo-code provided by
  ITead studio. You can find the latest version of the library at
  http://www.RinkyDinkElectronics.com/

  This library has been made to make it easy to use the Nokia 5110 LCD module 
  as a graphics display on an Arduino or a chipKit.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/

#include "LCD5110_Graph.h"
#if defined(__AVR__)
	#include <avr/pgmspace.h>
	#include "hardware/avr/HW_AVR.h"
#elif defined(__PIC32MX__)
	#pragma message("Compiling for PIC32 Architecture...")
	#include "hardware/pic32/HW_PIC32.h"
#elif defined(__arm__)
	#pragma message("Compiling for ARM Architecture...")
	#include "hardware/arm/HW_ARM.h"
#endif

void SetBits(uint8_t* dest, uint8_t src) {
	*dest |= src;
}

void ClearBits(uint8_t* dest, uint8_t src) {
	*dest &= ~src;
}

void FlipBits(uint8_t* dest, uint8_t src) {
	*dest ^= src;
}

LCD5110::LCD5110(int SCK, int MOSI, int DC, int RST, int CS)
{ 
	P_SCK	= portOutputRegister(digitalPinToPort(SCK));
	B_SCK	= digitalPinToBitMask(SCK);
	P_MOSI	= portOutputRegister(digitalPinToPort(MOSI));
	B_MOSI	= digitalPinToBitMask(MOSI);
	P_DC	= portOutputRegister(digitalPinToPort(DC));
	B_DC	= digitalPinToBitMask(DC);
	P_RST	= portOutputRegister(digitalPinToPort(RST));
	B_RST	= digitalPinToBitMask(RST);
	P_CS	= portOutputRegister(digitalPinToPort(CS));
	B_CS	= digitalPinToBitMask(CS);
	pinMode(SCK,OUTPUT);
	pinMode(MOSI,OUTPUT);
	pinMode(DC,OUTPUT);
	pinMode(RST,OUTPUT);
	pinMode(CS,OUTPUT);
	SCK_Pin=SCK;
	RST_Pin=RST;
}

void LCD5110::_LCD_Write(unsigned char data, unsigned char mode)
{   
    cbi(P_CS, B_CS);

	if (mode==LCD_COMMAND)
		cbi(P_DC, B_DC);
	else
		sbi(P_DC, B_DC);

	for (unsigned char c=0; c<8; ++c)
	{
		if (data & 0x80)
			sbi(P_MOSI, B_MOSI);
		else
			cbi(P_MOSI, B_MOSI);
		data = data<<1;
		pulseClock;
	}

	sbi(P_CS, B_CS);
}

void LCD5110::InitLCD(int contrast)
{
	if (contrast>0x7F)
		contrast=0x7F;
	if (contrast<0)
		contrast=0;

	resetLCD;

	_LCD_Write(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION, LCD_COMMAND);
	_LCD_Write(PCD8544_SETVOP | contrast, LCD_COMMAND);
	_LCD_Write(PCD8544_SETTEMP | LCD_TEMP, LCD_COMMAND);
	_LCD_Write(PCD8544_SETBIAS | LCD_BIAS, LCD_COMMAND);
	_LCD_Write(PCD8544_FUNCTIONSET, LCD_COMMAND);
	_LCD_Write(PCD8544_SETYADDR, LCD_COMMAND);
	_LCD_Write(PCD8544_SETXADDR, LCD_COMMAND);
	_LCD_Write(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL, LCD_COMMAND);

	clrScr();
	update();
	cfont.font=0;
	_sleep=false;
	_contrast=contrast;
}

void LCD5110::setContrast(int contrast)
{
	if (contrast>0x7F)
		contrast=0x7F;
	if (contrast<0)
		contrast=0;
	_LCD_Write(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION, LCD_COMMAND);
	_LCD_Write(PCD8544_SETVOP | contrast, LCD_COMMAND);
	_LCD_Write(PCD8544_FUNCTIONSET, LCD_COMMAND);
	_contrast=contrast;
}

void LCD5110::enableSleep()
{
	_sleep = true;
	_LCD_Write(PCD8544_SETYADDR, LCD_COMMAND);
	_LCD_Write(PCD8544_SETXADDR, LCD_COMMAND);
	for (int b=0; b<504; ++b)
		_LCD_Write(0, LCD_DATA);
	_LCD_Write(PCD8544_FUNCTIONSET | PCD8544_POWERDOWN, LCD_COMMAND);
}

void LCD5110::disableSleep()
{
	_sleep = false;
	_LCD_Write(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION, LCD_COMMAND);
	_LCD_Write(PCD8544_SETVOP | _contrast, LCD_COMMAND);
	_LCD_Write(PCD8544_SETTEMP | LCD_TEMP, LCD_COMMAND);
	_LCD_Write(PCD8544_SETBIAS | LCD_BIAS, LCD_COMMAND);
	_LCD_Write(PCD8544_FUNCTIONSET, LCD_COMMAND);
	_LCD_Write(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL, LCD_COMMAND);
	update();
}

void LCD5110::update()
{
	if (_sleep==false)
	{
		_LCD_Write(PCD8544_SETYADDR, LCD_COMMAND);
		_LCD_Write(PCD8544_SETXADDR, LCD_COMMAND);
		for (int b=0; b<504; ++b)
			_LCD_Write(scrbuf[b], LCD_DATA);
	}
}

void LCD5110::clrScr()
{
	for (int c=0; c<504; ++c)
		scrbuf[c]=0;
}

void LCD5110::fillScr()
{
	for (int c=0; c<504; ++c)
		scrbuf[c]=255;
}

void LCD5110::invert(bool mode)
{
	if (mode==true)
		_LCD_Write(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED, LCD_COMMAND);
	else
		_LCD_Write(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL, LCD_COMMAND);
}

void LCD5110::_editPixel(int x, int y, EditByteFunc f)
{
	int by, bi;

	if ((x>=0) and (x<84) and (y>=0) and (y<48))
	{
		by=((y/8)*84)+x;
		bi=y % 8;
		f(&scrbuf[by], 1<<bi);
	}
}

void LCD5110::_drawHLine(int x, int y, int l, EditByteFunc f)
{
	int by, bi;

	if ((x>=0) and (x<84) and (y>=0) and (y<48))
	{
		for (int cx=0; cx<l; ++cx)
		{
			by=((y/8)*84)+x;
			bi=y % 8;
			f(&scrbuf[by+cx], 1<<bi);
		}
	}
}

void LCD5110::_drawVLine(int x, int y, int l, EditByteFunc f)
{
	if ((x>=0) and (x<84) and (y>=0) and (y<48))
	{
		for (int cy=0; cy<l; ++cy)
		{
			_editPixel(x, y+cy, f);
		}
	}
}

void LCD5110::_drawLine(int x1, int y1, int x2, int y2, EditByteFunc f)
{
	int tmp;
	float delta, tx, ty;

	if (y1==y2)
	{
		if (x1>x2)
		{
			tmp=x1;
			x1=x2;
			x2=tmp;
		}
		_drawHLine(x1, y1, x2-x1, f);
	}
	else if (x1==x2)
	{
		if (y1>y2)
		{
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		_drawVLine(x1, y1, y2-y1, f);
	}
	else if (abs(x2-x1)>abs(y2-y1))
	{
		if (x2 < x1)
		{
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		delta = float(y2-y1)/float(x2-x1);
		ty=float(y1);
		for (int i=x1; i<=x2; ++i)
		{
			_editPixel(i, int(ty+0.5f), f);
			ty=ty+delta;
		}
	}
	else
	{
		if (y2 < y1)
		{
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		delta = float(x2-x1)/float(y2-y1);
		tx=float(x1);
		for (int i=y1; i<=y2; ++i)
		{
			_editPixel(int(tx+0.5f), i, f);
			tx=tx+delta;
		}
	}
}

void LCD5110::_drawRect(int x1, int y1, int x2, int y2, EditByteFunc f)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	_drawHLine(x1, y1, x2-x1, f);
	_drawHLine(x1, y2, x2-x1, f);
	_drawVLine(x1, y1, y2-y1, f);
	_drawVLine(x2, y1, y2-y1+1, f);
}

void LCD5110::_fillRect(int x1, int y1, int x2, int y2, EditByteFunc f)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	tmp = x2 - x1;
	for (int y=y1; y <= y2; ++y) {
		_drawHLine(x1, y, tmp, f);
	}
}

void LCD5110::_drawRoundRect(int x1, int y1, int x2, int y2, EditByteFunc f)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		_editPixel(x1+1, y1+1, f);
		_editPixel(x2-1, y1+1, f);
		_editPixel(x1+1, y2-1, f);
		_editPixel(x2-1, y2-1, f);
		_drawHLine(x1+2, y1, x2-x1-3, f);
		_drawHLine(x1+2, y2, x2-x1-3, f);
		_drawVLine(x1, y1+2, y2-y1-3, f);
		_drawVLine(x2, y1+2, y2-y1-3, f);
	}
}

void LCD5110::_drawCircle(int x, int y, int radius, EditByteFunc f)
{
	int ff = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	
	_editPixel(x, y + radius, f);
	_editPixel(x, y - radius, f);
	_editPixel(x + radius, y, f);
	_editPixel(x - radius, y, f);
 
	while(x1 < y1)
	{
		if(ff >= 0) 
		{
			--y1;
			ddF_y += 2;
			ff += ddF_y;
		}
		++x1;
		ddF_x += 2;
		ff += ddF_x;    
		_editPixel(x + x1, y + y1, f);
		_editPixel(x - x1, y + y1, f);
		_editPixel(x + x1, y - y1, f);
		_editPixel(x - x1, y - y1, f);
		_editPixel(x + y1, y + x1, f);
		_editPixel(x - y1, y + x1, f);
		_editPixel(x + y1, y - x1, f);
		_editPixel(x - y1, y - x1, f);
	}
}

void LCD5110::invertText(bool mode)
{
	cfont.inverted = mode ? 1 : 0;
}

void LCD5110::print(char *st, int x, int y)
{
	int stl;

	stl = strlen(st);
	if (x == RIGHT)
		x = 84-(stl*cfont.x_size);
	if (x == CENTER)
		x = (84-(stl*cfont.x_size))/2;

	for (int cnt=0; cnt<stl; ++cnt)
		_print_char(*st++, x + (cnt*(cfont.x_size)), y);
}

void LCD5110::print(String st, int x, int y)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	print(buf, x, y);
}

void LCD5110::printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); ++c)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); ++i)
			{
				st[i+neg]=filler;
				++f;
			}
		}

		for (int i=0; i<c; ++i)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	print(st,x,y);
}

void LCD5110::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	boolean neg=false;

	if (num<0)
		neg = true;

	_convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (uint8_t i=0; i<sizeof(st); ++i)
			if (st[i]=='.')
				st[i]=divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0]='-';
			for (uint8_t i=1; i<sizeof(st); ++i)
				if ((st[i]==' ') || (st[i]=='-'))
					st[i]=filler;
		}
		else
		{
			for (uint8_t i=0; i<sizeof(st); ++i)
				if (st[i]==' ')
					st[i]=filler;
		}
	}

	print(st,x,y);
}

void LCD5110::_print_char(unsigned char c, int x, int y)
{
	if ((cfont.y_size % 8) == 0)
	{
		int font_idx = ((c - cfont.offset)*(cfont.x_size*(cfont.y_size/8)))+4;
		for (int rowcnt=0; rowcnt<(cfont.y_size/8); rowcnt++)
		{
			for(int cnt=0; cnt<cfont.x_size; ++cnt)
			{
				for (int b=0; b<8; ++b)
					if ((fontbyte(font_idx+cnt+(rowcnt*cfont.x_size)) & (1<<b))!=0)
						_editPixel(x+cnt, y+(rowcnt*8)+b, (cfont.inverted ? &ClearBits : &SetBits));
					else
						_editPixel(x+cnt, y+(rowcnt*8)+b, (cfont.inverted ? &SetBits : &ClearBits));
			}
		}
	}
	else
	{
		int font_idx = ((c - cfont.offset)*((cfont.x_size*cfont.y_size/8)))+4;
		int cbyte=fontbyte(font_idx);
		int cbit=7;
		for (int cx=0; cx<cfont.x_size; ++cx)
		{
			for (int cy=0; cy<cfont.y_size; ++cy)
			{
				if ((cbyte & (1<<cbit)) != 0)
					_editPixel(x+cx, y+cy, (cfont.inverted ? &ClearBits : &SetBits));
				else
					_editPixel(x+cx, y+cy, (cfont.inverted ? &SetBits : &ClearBits));
				if (--cbit<0)
				{
					cbit=7;
					cbyte=fontbyte(++font_idx);
				}
			}
		}
	}
}

void LCD5110::setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=fontbyte(0);
	cfont.y_size=fontbyte(1);
	cfont.offset=fontbyte(2);
	cfont.numchars=fontbyte(3);
	cfont.inverted=0;
}

void LCD5110::drawBitmap(int x, int y, uint8_t* bitmap, int sx, int sy)
{
	int bit;
	byte data;

	for (int cy=0; cy<sy; ++cy)
	{
		bit= cy % 8;
		for(int cx=0; cx<sx; ++cx)
		{
			data=bitmapbyte(cx+((cy/8)*sx));
			_editPixel(x+cx, y+cy, data & (1<<bit) ? &SetBits : &ClearBits);
		}
	}      
}
