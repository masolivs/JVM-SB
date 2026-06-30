#ifndef TYPES_H
#define TYPES_H

/**
 * @file types.h
 * @brief Tipos primitivos para leitura do formato .class Java (big-endian).
 *
 * Define tipos fixos usados no parsing de bytecode conforme
 * Java Virtual Machine Specification — Java 8.
 */

#include <stdint.h>

/** @typedef u1
 *  @brief Unsigned 1-byte (uint8_t). Usado em access_flags, opcodes, tags do CP, etc.
 */
typedef uint8_t  u1;

/** @typedef u2
 *  @brief Unsigned 2-byte big-endian (uint16_t). Usado em indices do CP, counts.
 */
typedef uint16_t u2;

/** @typedef u4
 *  @brief Unsigned 4-byte big-endian (uint32_t). Usado em magic number, tamanhos.
 */
typedef uint32_t u4;

/** @typedef jint
 *  @brief Inteiro de 32 bits com sinal (tipo primitivo Java int).
 */
typedef int32_t  jint;

/** @typedef jlong
 *  @brief Inteiro de 64 bits com sinal (tipo primitivo Java long).
 */
typedef int64_t  jlong;

/** @typedef jfloat
 *  @brief Ponto flutuante IEEE 754 de 32 bits (tipo primitivo Java float).
 */
typedef float    jfloat;

/** @typedef jdouble
 *  @brief Ponto flutuante IEEE 754 de 64 bits (tipo primitivo Java double).
 */
typedef double   jdouble;

/** @typedef jbool
 *  @brief Booleano Java (0 = false, 1 = true).
 */
typedef uint8_t  jbool;

/** @typedef jchar
 *  @brief Caractere UTF-16 de 16 bits (tipo primitivo Java char).
 */
typedef uint16_t jchar;

/** @typedef jpc
 *  @brief Program counter do interpretador (offset no bytecode do metodo).
 */
typedef uint32_t jpc;

#endif
