#ifndef _MATH_H_
#define _MATH_H_

#define isnan(x)    __builtin_isnan(x)
#define signbit(x)  __builtin_signbit(x)
#define isinf(x)    __builtin_isinf(x)
#define isfinite(x) __builtin_isfinite(x)
#define INFINITY    (__builtin_inff())
#define NAN         (__builtin_nanf(""))

float  floorf(float x);
float  ceilf(float x);
float  truncf(float x);
float  rintf(float x);
float  sqrtf(float x);
float  fabsf(float x);
float  copysignf(float x, float y);

double floor(double x);
double ceil(double x);
double trunc(double x);
double rint(double x);
double sqrt(double x);
double fabs(double x);
double copysign(double x, double y);
double pow(double x, double y);
double fmod(double x, double y);

#endif