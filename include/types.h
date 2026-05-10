#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/* Tipos de leitura do .class (big-endian) */
typedef uint8_t  u1;   /* 1 byte  */
typedef uint16_t u2;   /* 2 bytes */
typedef uint32_t u4;   /* 4 bytes */

/* Tipos primitivos Java */
typedef int32_t  jint;
typedef int64_t  jlong;
typedef float    jfloat;
typedef double   jdouble;
typedef uint8_t  jbool;
typedef uint16_t jchar;

/* Program counter */
typedef uint32_t jpc;

#endif