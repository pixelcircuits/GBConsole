//-----------------------------------------------------------------------------------------
// Title:	Color
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef COLOR_H
#define COLOR_H

// Base Colors
#define COLOR_GRAY Color(80,80,80)
#define COLOR_DARKGRAY Color(45,45,45)
#define COLOR_LIGHTGRAY Color(221,221,221)
#define COLOR_WHITE Color(245,245,245)
#define COLOR_OFFWHITE Color(230,230,230)
#define COLOR_LIGHTBLUE Color(109,186,241)
#define COLOR_LIGHTORANGE Color(243,116,111)
#define COLOR_ACCENT Color(83,183,209)

//! Class that handles color math
class Color
{
public:
	//! Default constructor
	Color() : Red(0), Green(0), Blue(0) {}

	//! Main constructor
	Color(unsigned char r, unsigned char g, unsigned char b) : Red(r), Green(g), Blue(b) {}

	//! Copy constructor
	Color(const Color& other) : Red(other.Red), Green(other.Green), Blue(other.Blue) {}

	//! Lerp (linear interpolation)
	void lerp(const Color& other, unsigned char per) {
		if(Red > other.Red) {
			unsigned int rDiff = (unsigned int)Red-(unsigned int)other.Red;
			Red = (unsigned int)other.Red + (rDiff*(unsigned int)per)/100;
		} else {
			unsigned int rDiff = (unsigned int)other.Red-(unsigned int)Red;
			Red = (unsigned int)Red + (rDiff*(unsigned int)per)/100;
		}
		if(Green > other.Green) {
			unsigned int rDiff = (unsigned int)Green-(unsigned int)other.Green;
			Green = (unsigned int)other.Green + (rDiff*(unsigned int)per)/100;
		} else {
			unsigned int rDiff = (unsigned int)other.Green-(unsigned int)Green;
			Green = (unsigned int)Green + (rDiff*(unsigned int)per)/100;
		}
		if(Blue > other.Blue) {
			unsigned int rDiff = (unsigned int)Blue-(unsigned int)other.Blue;
			Blue = (unsigned int)other.Blue + (rDiff*(unsigned int)per)/100;
		} else {
			unsigned int rDiff = (unsigned int)other.Blue-(unsigned int)Blue;
			Blue = (unsigned int)Blue + (rDiff*(unsigned int)per)/100;
		}
	}

	//! Operators
	Color& operator=(const Color& other) {
		Red = other.Red; Green = other.Green; Blue = other.Blue; return *this;
	}
	Color operator+(const Color& other) const {
		signed short r = Red + other.Red; if(r > 255) r = 255;
		signed short g = Green + other.Green; if(g > 255) g = 255;
		signed short b = Blue + other.Blue; if(b > 255) b = 255;
		return Color(r, g, b);
	}
	Color& operator+=(const Color& other) {
		signed short r = Red + other.Red; if(r > 255) r = 255;
		signed short g = Green + other.Green; if(g > 255) g = 255;
		signed short b = Blue + other.Blue; if(b > 255) b = 255;
		Red=r; Green=g; Blue=b; return *this;
	}
	Color operator-(const Color& other) const {
		signed short r = Red - other.Red; if(r < 0) r = 0;
		signed short g = Green - other.Green; if(g < 0) g = 0;
		signed short b = Blue - other.Blue; if(b < 0) b = 0;
		return Color(r, g, b);
	}
	Color& operator-=(const Color& other) {
		signed short r = Red - other.Red; if(r < 0) r = 0;
		signed short g = Green - other.Green; if(g < 0) g = 0;
		signed short b = Blue - other.Blue; if(b < 0) b = 0;
		Red=r; Green=g; Blue=b; return *this;
	}
	Color operator*(const Color& other) const {
		unsigned short r = Red * other.Red; if(r > 255) r = 255;
		unsigned short g = Green * other.Green; if(g > 255) g = 255;
		unsigned short b = Blue * other.Blue; if(b > 255) b = 255;
		return Color(r, g, b);
	}
	Color& operator*=(const Color& other) {
		unsigned short r = Red * other.Red; if(r > 255) r = 255;
		unsigned short g = Green * other.Green; if(g > 255) g = 255;
		unsigned short b = Blue * other.Blue; if(b > 255) b = 255;
		Red=r; Green=g; Blue=b; return *this;
	}
	Color operator*(const int v) const {
		int v2 = v;
		if(v2 > 255) v2 = 255;
		unsigned short r = Red * v2; if(r > 255) r = 255;
		unsigned short g = Green * v2; if(g > 255) g = 255;
		unsigned short b = Blue * v2; if(b > 255) b = 255;
		return Color(r, g, b);
	}
	Color& operator*=(const int v) {
		int v2 = v;
		if(v2 > 255) v2 = 255;
		unsigned short r = Red * v2; if(r > 255) r = 255;
		unsigned short g = Green * v2; if(g > 255) g = 255;
		unsigned short b = Blue * v2; if(b > 255) b = 255;
		Red=r; Green=g; Blue=b; return *this;
	}
	Color operator/(const Color& other) const {
		return Color(Red / other.Red, Green / other.Green, Blue / other.Blue);
	}
	Color& operator/=(const Color& other) {
		Red/=other.Red; Green/=other.Green; Blue/=other.Blue; return *this;
	}
	Color operator/(const int v) const {
		return Color(Red / v, Green / v, Blue / v);
	}
	Color& operator/=(const int v) {
		Red/=v; Green/=v; Blue/=v; return *this;
	}
	bool operator==(const Color& other) const {
		return Red==other.Red && Green==other.Green && Blue==other.Blue;
	}
	bool operator!=(const Color& other) const {
		return !(Red==other.Red && Green==other.Green && Blue==other.Blue);
	}

	//! Red value
	unsigned char Red;
	
	//! Green value
	unsigned char Green;
	
	//! Blue value
	unsigned char Blue;
};

#endif
