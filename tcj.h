//******************************************************************************
// tcj.h
// Fichero de cabecera de tcj.c
// Por Ignacio Pérez Gil 31/03/2004.
//******************************************************************************

#ifndef TCJ_H
#define TCJ_H

//******************************************************************************
// Definiciones de direcciones y botones.
//******************************************************************************
#define TCJ_ARR  0
#define TCJ_A_D  1
#define TCJ_D_A  1
#define TCJ_DER  2
#define TCJ_D_B  3
#define TCJ_B_D  3
#define TCJ_ABJ  4
#define TCJ_B_I  5
#define TCJ_I_B  5
#define TCJ_IZQ  6
#define TCJ_I_A  7
#define TCJ_A_I  7
#define TCJ_AC0  8
#define TCJ_AC1  9
#define TCJ_AC2 10
#define TCJ_AC3 11
#define TCJ_AC4 12
#define TCJ_AC5 13

//******************************************************************************
// Definiciones varias.
//******************************************************************************
#define TCJ_MAX_ENTRADAS 3
#define TCJ_NUM_BOTONES  6
#define TCJ_NO_INSTALAR  0
#define TCJ_INSTALAR_T   1
#define TCJ_INSTALAR_J   2
#define TCJ_INSTALAR_T_J 3

//******************************************************************************
// Declaración de funciones.
//******************************************************************************
void tcj_inicializar(unsigned char instalar);
void tcj_incorporar_tecla(char diracc, unsigned char nueva_entrada);
void tcj_comprobar(void);

#ifndef TCJ_C
extern
#endif
       char tcj_estado[8+TCJ_NUM_BOTONES];
#endif


