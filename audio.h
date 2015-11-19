//******************************************************************************
// audio.h
// Fichero de cabecera de audio.c
// Por Ignacio Pérez Gil 14/03/2008.
//******************************************************************************

#ifndef _AUDIOH_
#define _AUDIOH_

//******************************************************************************
// Declaración de funciones.
//******************************************************************************
void ini_audio(void);
void fin_audio(void);
void reproducir_musica(char n);
void modificar_volmus(unsigned char vol);
void modificar_volson(unsigned char vol, char vz);
void parar_musica(void);
void musica_fin(void *msc_fin_void, unsigned int long_msc_fin_void);
char reproducir_sonido(char sonido, char modo);
char reproducir_onda(SAMPLE *onda, char bucle);
void pausar_sonidos(void);
void reanudar_sonidos(void);
void reiniciar_sonido(char vz);
void detener_sonido(char vz);
void detener_sonidos(char todos);

#endif

