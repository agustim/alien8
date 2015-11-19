//******************************************************************************
// tcj.c
// Funciones de control del teclado y joystick/joypad.
// Por Ignacio Pérez Gil 31/03/2004.
//******************************************************************************

//******************************************************************************
// Includes.
//******************************************************************************
#include <allegro.h>
#define TCJ_C
#include "tcj.h"

//******************************************************************************
// Definiciones y variables globales.
//******************************************************************************
volatile char *entrada[TCJ_MAX_ENTRADAS*(4+TCJ_NUM_BOTONES)];
char ini_t=0,ini_j=0;

//******************************************************************************
// Función tcj_inicializar(...)
//    Inicializa el sistema, instalando el teclado, el joystick/joypad o ambos.
//******************************************************************************
void tcj_inicializar(unsigned char instalar)
{
 char f,n;

 for(f=0;f<(TCJ_MAX_ENTRADAS*(4+TCJ_NUM_BOTONES));entrada[f++]=NULL);

 if(instalar&TCJ_INSTALAR_T && !ini_t)
  {
   install_keyboard();
   ini_t=1;
  }

 if(instalar&TCJ_INSTALAR_J && !ini_j)
   if(!install_joystick(JOY_TYPE_AUTODETECT))
     if(num_joysticks)
      {
       ini_j=1;
#ifdef ALLEGRO_BIG_ENDIAN
#define BGE 3
#else
#define BGE 0
#endif
       entrada[0]                 =(volatile char *)(&(joy[0].stick[0].axis[1].d1))+BGE;
       entrada[TCJ_MAX_ENTRADAS]  =(volatile char *)(&(joy[0].stick[0].axis[0].d2))+BGE;
       entrada[TCJ_MAX_ENTRADAS*2]=(volatile char *)(&(joy[0].stick[0].axis[1].d2))+BGE;
       entrada[TCJ_MAX_ENTRADAS*3]=(volatile char *)(&(joy[0].stick[0].axis[0].d1))+BGE;

       for(n=0,f=TCJ_MAX_ENTRADAS*4;f<(TCJ_MAX_ENTRADAS*(4+TCJ_NUM_BOTONES));n++,f+=TCJ_MAX_ENTRADAS)
         entrada[f]=(volatile char *)(&(joy[0].button[n].b))+BGE;
#undef BGE
      }
}

//******************************************************************************
// Función tcj_incorporar_tecla(...)
//    Define una tecla como una dirección o botón determinados.
//******************************************************************************
void tcj_incorporar_tecla(char diracc, unsigned char nueva_entrada)
{
 if(diracc<8+TCJ_NUM_BOTONES && diracc!=TCJ_A_D && diracc!=TCJ_D_B && diracc!=TCJ_B_I && diracc!=TCJ_I_A)
  {char maxdiracc=TCJ_MAX_ENTRADAS+(diracc=(diracc<TCJ_AC0?diracc/2:diracc-4)*TCJ_MAX_ENTRADAS);
   for(;diracc<maxdiracc;diracc++)
     if(!entrada[diracc])
      {
       entrada[diracc]=key+nueva_entrada;
       diracc=maxdiracc;
      }
  }  
}

//******************************************************************************
// Función tcj_comprobar()
//    Comprueba el estado del teclado y/o joystick/joypad para actualizar los
//    valores de la tabla tcj_estado.
//******************************************************************************
void tcj_comprobar()
 {
  char f;
 
  // inicializamos y hacemos los polls necesarios
  for(f=0;f<8+TCJ_NUM_BOTONES;tcj_estado[f++]=0);
  if(ini_t) poll_keyboard();
  if(ini_j) poll_joystick();

  // Comprobamos las cuatro direcciones básicas.
  for(f=0;f<TCJ_MAX_ENTRADAS*4;f++)
    if(entrada[f]) if(*entrada[f]) tcj_estado[(f/TCJ_MAX_ENTRADAS)*2]=1;

  // En función de las cuatro básicas, informamos las cuatro de suma.
  if(tcj_estado[TCJ_ARR] && tcj_estado[TCJ_ABJ])
    tcj_estado[TCJ_ARR]=tcj_estado[TCJ_ABJ]=0;
  if(tcj_estado[TCJ_IZQ] && tcj_estado[TCJ_DER])
    tcj_estado[TCJ_IZQ]=tcj_estado[TCJ_DER]=0;
  if(tcj_estado[TCJ_ARR] && tcj_estado[TCJ_DER])
   {
    tcj_estado[TCJ_ARR]=tcj_estado[TCJ_DER]=0;
    tcj_estado[TCJ_A_D]=1;
   }
  if(tcj_estado[TCJ_ARR] && tcj_estado[TCJ_IZQ])
   {
    tcj_estado[TCJ_ARR]=tcj_estado[TCJ_IZQ]=0;
    tcj_estado[TCJ_A_I]=1;
   }
  if(tcj_estado[TCJ_ABJ] && tcj_estado[TCJ_DER])
   {
    tcj_estado[TCJ_ABJ]=tcj_estado[TCJ_DER]=0;
    tcj_estado[TCJ_B_D]=1;
   }
  if(tcj_estado[TCJ_ABJ] && tcj_estado[TCJ_IZQ])
   {
    tcj_estado[TCJ_ABJ]=tcj_estado[TCJ_IZQ]=0;
    tcj_estado[TCJ_B_I]=1;
   }
  
  // Comprobamos las acciones.
  for(;f<TCJ_MAX_ENTRADAS*(4+TCJ_NUM_BOTONES);f++)
    if(entrada[f]) if(*entrada[f]) tcj_estado[(f/TCJ_MAX_ENTRADAS)+4]=1;
}

