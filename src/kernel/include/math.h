#ifndef _MATH_H_
#define _MATH_H_

#define isnan(x)    __builtin_isnan(x)
#define signbit(x)  __builtin_signbit(x)
#define isinf(x)    __builtin_isinf(x)
#define isfinite(x) __builtin_isfinite(x)
#define INFINITY    (__builtin_inff())
#define NAN         (__builtin_nanf(""))
#define M_PI        3.14159265358979323846

// --- Float versions ---
float  floorf(float x);
float  ceilf(float x);
float  truncf(float x);
float  rintf(float x);
float  sqrtf(float x);
float  fabsf(float x);
float  copysignf(float x, float y);

float  sinf(float x);
float  cosf(float x);
float  tanf(float x);
float  asinf(float x);
float  acosf(float x);
float  atanf(float x);
float  atan2f(float y, float x);

// --- Double versions ---
double floor(double x);
double ceil(double x);
double trunc(double x);
double rint(double x);
double sqrt(double x);
double fabs(double x);
double copysign(double x, double y);
double pow(double x, double y);
double fmod(double x, double y);

double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y, double x);

#endif