#pragma once
#include <math.h> 

typedef double vec_t;
typedef vec_t vec3_t[3];

extern vec3_t vec3_origin;

typedef enum { qfalse, qtrue } qbool;



typedef enum {
	SIDE_FRONT =  0,
	SIDE_ON    =  2,
	SIDE_BACK  =  1, 
	SIDE_CROSS = -2
};



#define	Q_PI	3.14159265358979323846

#define VectorCopy(src, dst) {dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];}
#define VectorAdd(a, b, c) {c[0] = a[0]+b[0];c[1] = a[1]+b[1];c[2] = a[2]+b[2];}
#define VectorSubtract(a, b, c) {c[0] = a[0]-b[0];c[1] = a[1]-b[1];c[2] = a[2]-b[2];}
#define VectorScale(v, scale, res) {res[0] = scale*v[0];res[1] = scale*v[1];res[2] = scale*v[2];}
#define VectorMA(v1, scale, v2, res) {res[0] = v1[0]+scale*v2[0];res[1] = v1[1]+scale*v2[1];res[2] = v1[2]+scale*v2[2];}
#define VectorInverse(v) {v[0]=-v[0];v[1]=-v[1];v[2]=-v[2];} 
#define DotProduct(v1, v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define VectorLength(v) (sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]))
#define CrossProduct(a, b, c) {c[0]=a[1]*b[2]-a[2]*b[1]; c[1]=a[2]*b[0]-a[0]*b[2];c[2]=a[0]*b[1]-a[1]*b[0];}


#define LENGTH_EPSILON 0.0001

extern vec_t VectorNormalize(vec3_t vector);

#define COMPARE_EPSILON 0.0001

extern qbool VectorCompare(vec3_t v1, vec3_t v2);


#define Q_round(a) (floor(a + 0.5))

extern int sgn(double x, double accuracy);
extern int Q_sgn(float x);
