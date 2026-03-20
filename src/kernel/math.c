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

// Float versions
float sinf(float x)   { return __builtin_sinf(x); }
float cosf(float x)   { return __builtin_cosf(x); }
float tanf(float x)   { return __builtin_tanf(x); }
float asinf(float x)  { return __builtin_asinf(x); }
float acosf(float x)  { return __builtin_acosf(x); }
float atanf(float x)  { return __builtin_atanf(x); }
float atan2f(float y, float x) { return __builtin_atan2f(y, x); }

// Double versions
double sin(double x)  { return __builtin_sin(x); }
double cos(double x)  { return __builtin_cos(x); }
double tan(double x)  { return __builtin_tan(x); }
double asin(double x) { return __builtin_asin(x); }
double acos(double x) { return __builtin_acos(x); }
double atan(double x) { return __builtin_atan(x); }
double atan2(double y, double x) { return __builtin_atan2(y, x); }