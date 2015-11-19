//******************************************************************************
// audio.c
// Funciones genéricas de sonido.
// Por Ignacio Pérez Gil 17/03/2008.
//******************************************************************************

//******************************************************************************
// Includes.
//******************************************************************************
#include <allegro.h>
#include <fmod.h>
#include "audio.h"

//******************************************************************************
// Declaración de estructuras de datos y variables globales.
//******************************************************************************
DATAFILE *f_msc = NULL;
DATAFILE *f_audio = NULL;
FMOD_SYSTEM  *sistema = NULL;
FMOD_SOUND   *sm = NULL;
FMOD_CHANNEL *canal_musica = NULL;
char musica_sonando=-1;
const char *msc_fin=NULL;
unsigned int long_msc_fin;
unsigned char vol_son=0;
float vol_mus=0;
char rep_snd_onda(SAMPLE *onda, char sonido, char bucle);
struct _voz // Estructura para almacenar los datos de las voces
   {char activa; // Indica si la voz está activa
                 // 0-desactivada, 1-normal, 2-bucle, 3-no parar
    int v;       // Voz
   } voz[16]={{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
              {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}; 

//******************************************************************************
// Función iniciar_audio()
//   Inicializa el sistema de audio y carga los ficheros de sonidos y música.
//******************************************************************************
void ini_audio(void)
{
 reserve_voices(16,0);

 install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);

 FMOD_System_Create(&sistema);
 FMOD_System_Init(sistema, 16, FMOD_INIT_NORMAL, NULL);

 f_msc = load_datafile("msc.dat");
 f_audio = load_datafile("snd.dat");
}

//******************************************************************************
// Función finalizar_audio()
//   Descarga los ficheros de sonidos y música.
//******************************************************************************
void fin_audio(void)
{
 if(musica_sonando>=0) FMOD_Sound_Release(sm);
 FMOD_System_Close(sistema);
 FMOD_System_Release(sistema);
 unload_datafile(f_msc);
 unload_datafile(f_audio);
}

//******************************************************************************
// Función reproducir_musica(...)
//   Inicia la reproducción del fichero de música n.
//******************************************************************************
void reproducir_musica(char n)
{
 if(vol_mus)
  {
   FMOD_System_Update(sistema);
   if(musica_sonando!=n && n>=0)
    {
     FMOD_CREATESOUNDEXINFO inf_tm;
     if(musica_sonando>=0) FMOD_Sound_Release(sm);
     musica_sonando=n;
     memset(&inf_tm, 0, sizeof(FMOD_CREATESOUNDEXINFO));
     inf_tm.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
     if(msc_fin)
      {
       inf_tm.length=long_msc_fin;
       FMOD_System_CreateStream(sistema, msc_fin, FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_OPENMEMORY, &inf_tm, &sm);
      }
     else
      {
       inf_tm.length=f_msc[n].size;
       FMOD_System_CreateStream(sistema, (const char *)(f_msc[n].dat), FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_OPENMEMORY, &inf_tm, &sm);
      }
     FMOD_System_PlaySound(sistema, FMOD_CHANNEL_FREE, sm, 1, &canal_musica);
     FMOD_Channel_SetVolume(canal_musica,vol_mus);
     FMOD_Channel_SetPaused(canal_musica,0);
    }
  }
}

//******************************************************************************
// Función modificar_volmus(...)
//   Modifica el volúmen de la música.
//******************************************************************************
void modificar_volmus(unsigned char vol)
{
 if(vol<=100)
  {
   vol_mus=vol;
   vol_mus/=100;
   if(musica_sonando>=0)
    {
     if(!vol_mus)
      {
       FMOD_Channel_Stop(canal_musica);
       musica_sonando=-1;
      }
     else
       FMOD_Channel_SetVolume(canal_musica,vol_mus);
    }
  }
}

//******************************************************************************
// Función parar_musica()
//   Detiene la música que se esté reproduciendo.
//******************************************************************************
void parar_musica(void)
{
 if(musica_sonando>=0)
  {
   FMOD_Sound_Release(sm);
   musica_sonando=-1;
  }
 msc_fin=NULL;
}

//******************************************************************************
// Función musica_fin(...)
//   Obtiene un puntero a la música del fin del juego, que hay que reproducir
//   una sola vez.
//******************************************************************************
void musica_fin(void *msc_fin_void, unsigned int long_msc_fin_void)
{
 msc_fin=(const char *)msc_fin_void;
 long_msc_fin=long_msc_fin_void;
}

//******************************************************************************
// Función reproducir_sonido(...)
//   Reproduce el sonido indicado, con la opción de buclearlo.
//******************************************************************************
char reproducir_sonido(char sonido, char bucle)
{
 return rep_snd_onda(NULL,sonido,bucle);
}

//******************************************************************************
// Función reproducir_onda(...)
//   Reproduce el sample indicado, con la opción de buclearlo.
//******************************************************************************
char reproducir_onda(SAMPLE *onda, char bucle)
{
 return rep_snd_onda(onda,0,bucle);
}

//******************************************************************************
// Función rep_snd_onda(...)
//   Reproduce el sample o el sonido indicado, con el modo indicado.
//******************************************************************************
char rep_snd_onda(SAMPLE *onda, char sonido, char modo)
{
 char f = -1; 
 
 if(!vol_son) return -1;

 do // Busco la primera voz que no se esté usando
  {
   f++;
   if(voz[f].activa==1 || voz[f].activa==3)
     if(voice_get_position(voz[f].v) == -1)
      {
       deallocate_voice(voz[f].v);
       voz[f].activa = 0;
      }
  }while(voz[f].activa && f<15);
 
 if(voz[f].activa) return -1; // Las 16 voces están siendo usadas.

 // Ahora f apunta a la voz que vamos a usar para el sonido.
 voz[f].v = allocate_voice(onda?onda:(SAMPLE *)f_audio[sonido].dat);
 if(voz[f].v == -1) return -1;
 voice_set_volume(voz[f].v, vol_son);
 voice_set_playmode(voz[f].v, modo<0?PLAYMODE_PLAY:modo);
 voice_start(voz[f].v);
 if(modo<0)
   voz[f].activa = 3;
 else
   voz[f].activa = (modo==PLAYMODE_PLAY?1:2);
  
 return f;  
}

//******************************************************************************
// Función pausar_sonidos()
//   Pone los sonidos bucleados en pausa, y para los demás.
//******************************************************************************
void pausar_sonidos(void)
{
 char f;

 for(f=0;f<16;f++)
   if(voz[f].activa)
    {
     if(voz[f].activa==1 || voz[f].activa==3)
      {// Si la voz está parada o no está bucleada, aprovecho para liberarla.
       deallocate_voice(voz[f].v);
       voz[f].activa = 0;
      }
     else 
       voice_stop(voz[f].v);
    }
}

//******************************************************************************
// Función reanudar_sonidos()
//   Reanuda la reproducción de los sonidos pausados.
//******************************************************************************
void reanudar_sonidos(void)
{
 char f;

 for(f=0;f<16;f++)
   if(voz[f].activa)
       voice_start(voz[f].v);
}

//******************************************************************************
// Función reiniciar_sonido(...)
//   Vuelve a colocar un sonido al principio.
//******************************************************************************
void reiniciar_sonido(char vz)
{
 if(vz>=0)
 if(voz[vz].activa)
   voice_set_position(voz[vz].v, 0);
}

//******************************************************************************
// Función modificar_volson(...)
//   Modifica el volúmen máximo de los sonidos.
//******************************************************************************
void modificar_volson(unsigned char vol, char vz)
{
 float fvol=vol*2.55;
 vol_son=(unsigned char)fvol;
 if(vz>=0) voice_set_volume(voz[vz].v, vol_son);
}

//******************************************************************************
// Función detener_sonido(...)
//   Detiene un sonido determinado.
//******************************************************************************
void detener_sonido(char vz)
{
 if(vz>=0)
 if(voz[vz].activa)
  {
   deallocate_voice(voz[vz].v);
   voz[vz].activa = 0;
  }
}

//******************************************************************************
// Función detener_sonidos(char todos)
//   Detiene todos los sonidos que se estén reproduciendo.
//******************************************************************************
void detener_sonidos(char todos)
{
 char f;
 for(f=0;f<16;f++)
   if(voz[f].activa>0 && (voz[f].activa<3 || todos))
    {
     deallocate_voice(voz[f].v);
     voz[f].activa = 0;
    }
}

