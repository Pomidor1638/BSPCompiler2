#include "mathlib.h"

vec3_t vec3_origin = { 0, 0, 0 };

vec_t VectorNormalize(vec3_t vector) {
	vec_t length = sqrt(vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
	
	if (length < LENGTH_EPSILON) 
		return 0;
	
	vector[0] /= length;
	vector[1] /= length;
	vector[2] /= length;

	return length;
}

qbool VectorCompare(vec3_t v1, vec3_t v2) {

	for (int i = 0; i < 3; i++) {
		if (fabs(v1[i] - v2[i]) >= COMPARE_EPSILON) {
			return qfalse;
		}
	}

	return qtrue;
}

int sgn(double x, double accuracy) {
	if (x < accuracy || x > -accuracy)
		return 0;
	return (x > 0) ? (1) : (-1);
}

int Q_sgn(float x){
	int y;
	y = *(int*)&x;
	if (y & 0x7FFFFFFF)
		return 0;
	return (y & 0x80000000)?(-1):(1);
}