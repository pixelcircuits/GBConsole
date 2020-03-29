//-----------------------------------------------------------------------------------------
// Title:	Vector
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef VECTOR_H
#define VECTOR_H


//! Class that handles Vector math
class Vector
{
public:
	//! Default constructor
	Vector() : X(0), Y(0) {}

	//! Main constructor
	Vector(int x, int y) : X(x), Y(y) {}

	//! Copy constructor
	Vector(const Vector& other) : X(other.X), Y(other.Y) {}

	//! Lerp (linear interpolation)
	void lerp(const Vector& other, unsigned char per) {
		X = ((X+other.X)*per)/100;
		Y = ((Y+other.Y)*per)/100;
	}

	//! Operators
	Vector operator-() const { return Vector(-X, -Y); }
	Vector& operator=(const Vector& other) { X = other.X; Y = other.Y; return *this; }
	Vector operator+(const Vector& other) const { return Vector(X + other.X, Y + other.Y); }
	Vector& operator+=(const Vector& other) { X+=other.X; Y+=other.Y; return *this; }
	Vector operator-(const Vector& other) const { return Vector(X - other.X, Y - other.Y); }
	Vector& operator-=(const Vector& other) { X-=other.X; Y-=other.Y; return *this; }
	Vector operator*(const Vector& other) const { return Vector(X * other.X, Y * other.Y); }
	Vector& operator*=(const Vector& other) { X*=other.X; Y*=other.Y; return *this; }
	Vector operator*(const int v) const { return Vector(X * v, Y * v); }
	Vector& operator*=(const int v) { X*=v; Y*=v; return *this; }
	Vector operator/(const Vector& other) const { return Vector(X / other.X, Y / other.Y); }
	Vector& operator/=(const Vector& other) { X/=other.X; Y/=other.Y; return *this; }
	Vector operator/(const int v) const { return Vector(X / v, Y / v); }
	Vector& operator/=(const int v) { X/=v; Y/=v; return *this; }
	bool operator==(const Vector& other) const { return X==other.X && Y==other.Y; }
	bool operator!=(const Vector& other) const { return !(X==other.X && Y==other.Y); }

	//! X coordinate
	int X;

	//! Y coordinate
	int Y;
};

#endif
