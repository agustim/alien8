//******************************************************************************
// tiempo.h
// Fichero de cabecera de tiempo.c
// Por Ignacio P�rez Gil 11/09/2004.
//******************************************************************************

#ifndef _TIEMPOH_
#define _TIEMPOH_

#define TICKS 60
//******************************************************************************
// Declaraci�n de funciones.
//******************************************************************************
void ini_tiempo(void);
void temporizador(void);
extern volatile char tic_reloj;
#endif
