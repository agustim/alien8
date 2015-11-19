//******************************************************************************
// juego.h
// Fichero de cabecera de juego.c
// Por Ignacio Pérez Gil 15/03/2004.
//******************************************************************************

#ifndef JUEGO_H
#define JUEGO_H

void ini_juego(void);
char juego(void);
void fin_juego(void);

// Defines para la resolución de pantalla.
#define RES_X 640
#define RES_Y 480

// Estados del juego.
#define EST_SALIR    -1
#define EST_MENU      0
#define EST_JUEGO     1
#define EST_CREDITOS  2
#define EST_OPCIONES  3
#define EST_PREGUNTAR 4
#define EST_REDEFINIR 5

#define LETRA_CERR_0  0x80
#define LETRA_VDAS_0  0x99

#define fm_volcar()\
    {while(!tic_reloj);\
     tic_reloj=0;\
     vsync();\
     acquire_screen();\
     blit(buffer,screen,0,0,0,0,RES_X,RES_Y);\
     release_screen();}

#ifdef _JUEGOC_
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN char hicolor;
EXTERN char idioma;
EXTERN char nuevo_marcador;
EXTERN BITMAP *buffer;
EXTERN DATAFILE *f_obj;
EXTERN DATAFILE *f_esc;
EXTERN DATAFILE *f_anim;
EXTERN DATAFILE *f_robots;
EXTERN DATAFILE *f_fuentes;
EXTERN int puntuaciones[4];

#endif
