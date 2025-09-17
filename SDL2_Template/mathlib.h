#pragma once

#define M_PI    3.14159265358979323846264338327950288

//#define DOUBLE_VEC_T
#undef DOUBLE_VEC_T

#ifdef DOUBLE_VEC_T
typedef double vec_t;
#else
typedef float vec_t;
#endif
constexpr double EPSILON_ZERO = 1.0 / 1024.0; // 0.0009765625

typedef struct vec3 {
private:
	vec_t v[3];
public:

	vec3();
	vec3(vec_t x, vec_t y, vec_t z);

	vec_t& operator[](int k);
	vec_t operator[](int k) const;

	vec3 operator+(const vec3& v) const;
	vec3 operator-(const vec3& v) const;
	vec3 operator-() const;

	vec3 operator*(const vec_t& s) const;
	vec3 operator/(const vec_t& a) const;
	vec_t operator*(const vec3& v) const;

	vec3 operator+=(const vec3& v);
	vec3 operator-=(const vec3& v);

	vec3 operator*=(const vec_t& s);
	vec3 operator/=(const vec_t& s);

	static vec3 cross(const vec3& a, const vec3& b);
	static vec_t normalize(vec3& a);
	vec_t normalize();
	vec_t length() const;
} vec3_t;



double radiansToDegrees(double rad);
double degreesToRadians(double deg);

enum {
	ROLL = 1,
	PITCH = 0,
	YAW = 2,
};
