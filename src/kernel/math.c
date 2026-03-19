#include "math.h"

// Float versions
float floorf(float x)   { return __builtin_floorf(x); }
float ceilf(float x)    { return __builtin_ceilf(x); }
float truncf(float x)   { return __builtin_truncf(x); }
float rintf(float x)    { return __builtin_rintf(x); }
float sqrtf(float x)    { return __builtin_sqrtf(x); }
float fabsf(float x)    { return __builtin_fabsf(x); }
float copysignf(float x, float y) { return __builtin_copysignf(x, y); }

// Double versions
double floor(double x)  { return __builtin_floor(x); }
double ceil(double x)   { return __builtin_ceil(x); }
double trunc(double x)  { return __builtin_trunc(x); }
double rint(double x)   { return __builtin_rint(x); }
double sqrt(double x)   { return __builtin_sqrt(x); }
double fabs(double x)   { return __builtin_fabs(x); }
double copysign(double x, double y) { return __builtin_copysign(x, y); }

// Others
double pow(double x, double y)  { return __builtin_pow(x, y); }
double fmod(double x, double y) { return __builtin_fmod(x, y); }