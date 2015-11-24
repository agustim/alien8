//******************************************************************************
// audio.c
// Funciones gen�ricas de sonido.
// Por Ignacio P�rez Gil 17/03/2008.
//******************************************************************************

//******************************************************************************
// Includes.
//******************************************************************************
#include <allegro.h>
#include "audio.h"

//******************************************************************************
// Declaraci�n de estructuras de datos y variables globales.
//******************************************************************************
char musica_sonando=-1;
const char *msc_fin=NULL;
unsigned int long_msc_fin;
unsigned char vol_son=0;
float vol_mus=0;
char rep_snd_onda(SAMPLE *onda, char sonido, char bucle);
struct _voz // Estructura para almacenar los datos de las voces
   {char activa; // Indica si la voz est� activa
                 // 0-desactivada, 1-normal, 2-bucle, 3-no parar
    int v;       // Voz
   } voz[16]={{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
              {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};

//******************************************************************************
// Funci�n iniciar_audio()
//   Inicializa el sistema de audio y carga los ficheros de sonidos y m�sica.
//******************************************************************************
void ini_audio(void)
{

}

//******************************************************************************
// Funci�n finalizar_audio()
//   Descarga los ficheros de sonidos y m�sica.
//******************************************************************************
void fin_audio(void)
{
}

//******************************************************************************
// Funci�n reproducir_musica(...)
//   Inicia la reproducci�n del fichero de m�sica n.
//******************************************************************************
void reproducir_musica(char n)
{
}

//******************************************************************************
// Funci�n modificar_volmus(...)
//   Modifica el vol�men de la m�sica.
//******************************************************************************
void modificar_volmus(unsigned char vol)
{

}

//******************************************************************************
// Funci�n parar_musica()
//   Detiene la m�sica que se est� reproduciendo.
//******************************************************************************
void parar_musica(void)
{
}

//******************************************************************************
// Funci�n musica_fin(...)
//   Obtiene un puntero a la m�sica del fin del juego, que hay que reproducir
//   una sola vez.
//******************************************************************************
void musica_fin(void *msc_fin_void, unsigned int long_msc_fin_void)
{
}

//******************************************************************************
// Funci�n reproducir_sonido(...)
//   Reproduce el sonido indicado, con la opci�n de buclearlo.
//******************************************************************************
char reproducir_sonido(char sonido, char bucle)
{
}

//******************************************************************************
// Funci�n reproducir_onda(...)
//   Reproduce el sample indicado, con la opci�n de buclearlo.
//******************************************************************************
char reproducir_onda(SAMPLE *onda, char bucle)
{
}

//******************************************************************************
// Funci�n rep_snd_onda(...)
//   Reproduce el sample o el sonido indicado, con el modo indicado.
//******************************************************************************
char rep_snd_onda(SAMPLE *onda, char sonido, char modo)
{
}

//******************************************************************************
// Funci�n pausar_sonidos()
//   Pone los sonidos bucleados en pausa, y para los dem�s.
//******************************************************************************
void pausar_sonidos(void)
{
}

//******************************************************************************
// Funci�n reanudar_sonidos()
//   Reanuda la reproducci�n de los sonidos pausados.
//******************************************************************************
void reanudar_sonidos(void)
{
}

//******************************************************************************
// Funci�n reiniciar_sonido(...)
//   Vuelve a colocar un sonido al principio.
//******************************************************************************
void reiniciar_sonido(char vz)
{
}

//******************************************************************************
// Funci�n modificar_volson(...)
//   Modifica el vol�men m�ximo de los sonidos.
//******************************************************************************
void modificar_volson(unsigned char vol, char vz)
{
}

//******************************************************************************
// Funci�n detener_sonido(...)
//   Detiene un sonido determinado.
//******************************************************************************
void detener_sonido(char vz)
{
}

//******************************************************************************
// Funci�n detener_sonidos(char todos)
//   Detiene todos los sonidos que se est�n reproduciendo.
//******************************************************************************
void detener_sonidos(char todos)
{
}
