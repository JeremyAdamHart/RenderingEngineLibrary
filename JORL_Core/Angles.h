#pragma once

class Degrees;

class Radians {
private:
public:
	float angle;
	explicit Radians(float angle);
	Radians(Degrees deg);
	operator float();
};

class Degrees {
private:
public:
	float angle;
	explicit Degrees(float angle);
	Degrees(Radians rad);
	operator float();
};

Degrees operator+(Degrees, Degrees);
Radians operator+(Radians, Radians);

Degrees operator-(Degrees, Degrees);
Radians operator-(Radians, Radians);

Degrees operator*(Degrees, float);
Degrees operator*(float, Degrees);
Radians operator*(Radians, float);
Radians operator*(float, Radians);
