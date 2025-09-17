
#include "mathlib.h"

#define _USE_MATH_DEFINES
#include <cmath>


double radiansToDegrees(double rad) {
    return 180.0 * rad / M_PI;
}
double degreesToRadians(double deg) {
    return M_PI * deg / 180.0;
}

vec3::vec3() {
	v[0] = 0;
	v[1] = 0;
	v[2] = 0;
}
vec3::vec3(vec_t x, vec_t y, vec_t z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

vec_t& vec3::operator[](int k) {
	return v[k];
}

vec_t vec3::operator[](int k) const {
	return v[k];
}

vec3 vec3::operator+(const vec3& a) const {
	return {
		v[0] + a.v[0],
		v[1] + a.v[1],
		v[2] + a.v[2],
	};
}
vec3 vec3::operator-(const vec3& a) const {
	return {
		v[0] - a.v[0],
		v[1] - a.v[1],
		v[2] - a.v[2],
	};
}
vec3 vec3::operator-() const {
	return {
		-v[0],
		-v[1],
		-v[2],
	};
}

vec3 vec3::operator*(const vec_t& a) const {
	return {
		v[0] * a,
		v[1] * a,
		v[2] * a,
	};
}

vec3 vec3::operator/(const vec_t& a) const {
	return {
		v[0] / a,
		v[1] / a,
		v[2] / a,
	};
}

vec_t vec3::operator*(const vec3& a) const {
	return v[0] * a.v[0] + v[1] * a.v[1] + v[2] * a.v[2];
}

vec3 vec3::cross(const vec3& a, const vec3& b) {
	return {
		a.v[1] * b.v[2] - a.v[2] * b.v[1],
		a.v[2] * b.v[0] - a.v[0] * b.v[2],
		a.v[0] * b.v[1] - a.v[1] * b.v[0],
	};
}

vec3 vec3::operator+=(const vec3& v) {

	this->v[0] += v.v[0];
	this->v[1] += v.v[1];
	this->v[2] += v.v[2];

	return *this;
}


vec3 vec3::operator-=(const vec3& v) {

	this->v[0] -= v.v[0];
	this->v[1] -= v.v[1];
	this->v[2] -= v.v[2];

	return *this;
}
vec3 vec3::operator*=(const vec_t& v) {

	this->v[0] *= v;
	this->v[1] *= v;
	this->v[2] *= v;

	return *this;
}
vec3 vec3::operator/=(const vec_t& a) {

	this->v[0] /= a;
	this->v[1] /= a;
	this->v[2] /= a;

	return *this;
}

vec_t vec3::length() const {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

vec_t vec3::normalize(vec3& a) {
	double length = a.v[0] * a.v[0] + a.v[1] * a.v[1] + a.v[2] * a.v[2];

	if (length < EPSILON_ZERO * EPSILON_ZERO) {
		return 0;
	}


	length = sqrt(length);

	a.v[0] /= length;
	a.v[1] /= length;
	a.v[2] /= length;

	return length;
}

vec_t vec3::normalize() {
	double length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

	if (length < EPSILON_ZERO * EPSILON_ZERO) {
		return 0;
	}


	length = sqrt(length);

	v[0] /= length;
	v[1] /= length;
	v[2] /= length;

	return length;
}

