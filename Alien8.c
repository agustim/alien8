//******************************************************************************
// alien8.c
// Funci�n main() y funciones de estado.
// Por Ignacio P�rez Gil 18/05/2008.
//******************************************************************************

// Mi madre era una minipime,
// mi padre era un transist�,
// y escuchando un disco de Kraftwerk
// mi padre a mi madre la antena le meti�.
//
// Le ech� dos voltios sin sacarla, la antena,
// y de all� nas�o yo.
// Y aunque soy una m�quina mec�nica soy,
// la verg�ensa de t� los rob�.
//
// Soy un consolad�,
// soy una m�quina de haser el amor,
// si quieres que te haga fel�,
// ll�vame muy dentro, muy dentro de ti.
//
//                    Soy una m�quina de haser el amor - Mojinos Escoz�os

//******************************************************************************
// Includes.
//******************************************************************************
#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include "tiempo.h"
#include "tcj.h"
#include "juego.h"
#include "audio.h"
#include "internet.h"

char ini_video(void);
char comprobar_dats(void);
void ini_entrada(void);
char menu(void);
char creditos(void);
char opciones(void);
char preguntar(void);
char redefinir(void);

char volmus,volson,pantalla_completa;
char teclas_control[7];
void leer_cfg(void);
void escribir_cfg(void);
void elegir_idioma(void);
void intro(void);

//******************************************************************************
// Funci�n main()
//    Bucle principal.
//******************************************************************************
int main(void)
{
 char (*funcion_estado[6])(void)={menu,juego,creditos,opciones,preguntar,redefinir};
 char estado=EST_MENU;

 allegro_init();
 set_uformat(U_ASCII);
 leer_cfg();
 if(ini_video()) return -1;
 if(comprobar_dats()) return -1;
 ini_juego();
 ini_tiempo();
 ini_entrada();
 ini_audio();

 clear_bitmap(buffer);
 intro();
 if(idioma==2) elegir_idioma();

 while(estado!=EST_SALIR) estado=funcion_estado[estado]();

 fin_juego();
 return 0;
}
END_OF_MAIN();


//******************************************************************************
// Funci�n ini_video()
//    Inicializa el modo de v�deo.
//******************************************************************************
char ini_video(void)
{
 hicolor=1;
 set_color_depth(16);
 if(set_gfx_mode(pantalla_completa?GFX_AUTODETECT_FULLSCREEN:GFX_AUTODETECT_WINDOWED, RES_X, RES_Y, 0, 0))
  {
   hicolor=0;
   set_color_depth(32);
   if(set_gfx_mode(pantalla_completa?GFX_AUTODETECT_FULLSCREEN:GFX_AUTODETECT_WINDOWED, RES_X, RES_Y, 0, 0))
    {
     pantalla_completa=(pantalla_completa?0:1);
     hicolor=1;
     set_color_depth(16);
     if(set_gfx_mode(GFX_AUTODETECT, RES_X, RES_Y, 0, 0))
      {
       hicolor=0;
       set_color_depth(32);
       if(set_gfx_mode(GFX_AUTODETECT, RES_X, RES_Y, 0, 0))
        {
         allegro_message("No se ha podido establecer ning�n modo de video.\nCould not set any video mode.");
         return -1;
        }
      }
    }
  }
 return 0;
}

//******************************************************************************
// Funci�n comprobar_dats()
//    Comprueba que se encuentran presentes todos los ficheros .dat necesarios
//******************************************************************************
char comprobar_dats(void)
{
 char no_encontrado=0;
 DATAFILE *f_dat=NULL;

 if(f_dat=load_datafile("intro.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero intro.dat no encontrado\nERROR: file intro.dat not found");
  }

 if(f_dat=load_datafile("objetos.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero objetos.dat no encontrado\nERROR: file objetos.dat not found");
  }

 if(f_dat=load_datafile("escenarios.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero escenarios.dat no encontrado\nERROR: file escenarios.dat not found");
  }

 if(f_dat=load_datafile("anims.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero anims.dat no encontrado\nERROR: file anims.dat not found");
  }

 if(f_dat=load_datafile("robots.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero robots.dat no encontrado\nERROR: file robots.dat not found");
  }

 if(f_dat=load_datafile("msc.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero msc.dat no encontrado\nERROR: file msc.dat not found");
  }

 if(f_dat=load_datafile("snd.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero snd.dat no encontrado\nERROR: file snd.dat not found");
  }

 if(f_dat=load_datafile("fuentes.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero fuentes.dat no encontrado\nERROR: file fuentes.dat not found");
  }

 if(f_dat=load_datafile("sfn.dat"))
   unload_datafile(f_dat);
 else
  {
   no_encontrado=1;
   allegro_message("ERROR: fichero sfn.dat no encontrado\nERROR: file sfn.dat not found");
  }

 return no_encontrado;
}

//******************************************************************************
// Funci�n ini_entrada()
//    Inicializa los dispositivos de entrada (teclado y joystick).
//******************************************************************************
void ini_entrada(void)
{
 tcj_inicializar(TCJ_INSTALAR_T_J);
 tcj_incorporar_tecla(TCJ_IZQ, teclas_control[0]);
 tcj_incorporar_tecla(TCJ_ARR, teclas_control[1]);
 tcj_incorporar_tecla(TCJ_DER, teclas_control[2]);
 tcj_incorporar_tecla(TCJ_ABJ, teclas_control[3]);
 tcj_incorporar_tecla(TCJ_AC0, teclas_control[4]);
 tcj_incorporar_tecla(TCJ_AC1, teclas_control[5]);
 tcj_incorporar_tecla(TCJ_AC2, teclas_control[6]);
 tcj_incorporar_tecla(TCJ_AC3, KEY_ESC);
 tcj_incorporar_tecla(TCJ_AC4, KEY_ENTER);
 tcj_incorporar_tecla(TCJ_AC4, KEY_ENTER_PAD);
}

//******************************************************************************
// Funci�n menu()
//    Men� principal.
//******************************************************************************
char menu(void)
{
 char f, n, pulsado=1, contador=0, opc=0, elegida=50;
 char opciones[10][18]={"Comenzar",         "Start game",
                        "Opciones",         "Options",
                        "Cr�ditos",         "Credits",
                        "Redefinir teclas", "Redefine keys",
                        "Salir",            "Exit"};
 static char pos_cursor[10]={8,10,8,7,8,7,16,13,5,4};
 BITMAP *m_trans=create_bitmap(421,48);

 set_trans_blender(0,0,0,100);
 for(f=0;f<10;f++) opciones[f][pos_cursor[f]+1]=0;

 blit((BITMAP *)(f_esc[27].dat),buffer,0,0,0,0,RES_X,RES_Y);
 for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
 reproducir_sonido(23,PLAYMODE_PLAY);
 clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
 textout_centre_ex(m_trans, (FONT *)(f_fuentes[0].dat), "ALIEN 8", m_trans->w/2, -8, -1, -1);
 draw_trans_sprite(buffer,m_trans,(RES_X-m_trans->w)/2-5, 63);
 textout_centre_ex(buffer, (FONT *)(f_fuentes[0].dat), "ALIEN 8", RES_X/2,50, -1, -1);
 for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}

 for(f=0;f<5;f++)
  {
   reproducir_sonido(23,PLAYMODE_PLAY);
   textout_ex(buffer, (FONT *)(f_fuentes[1].dat), opciones[f*2+idioma], 170, 130+60*f, -1, -1);
   for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
  }
 reproducir_sonido(23,PLAYMODE_PLAY);

 while(elegida==50)
  {
   if(contador==0 || contador==30)
    {
     blit((BITMAP *)(f_esc[27].dat),buffer,0,0,0,0,RES_X,RES_Y);
     clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
     textout_centre_ex(m_trans, (FONT *)(f_fuentes[0].dat), "ALIEN 8", m_trans->w/2, -8, -1, -1);
     draw_trans_sprite(buffer,m_trans,(RES_X-m_trans->w)/2-5, 63);
     textout_centre_ex(buffer, (FONT *)(f_fuentes[0].dat), "ALIEN 8", RES_X/2,50, -1, -1);
     for(f=idioma;f<10;f+=2) opciones[f][pos_cursor[f]]=0;
     if(contador==0) opciones[opc*2+idioma][pos_cursor[opc*2+idioma]]='_';
     clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
     textout_ex(m_trans, (FONT *)(f_fuentes[0].dat), opciones[opc*2+idioma], 0, -8, -1, -1);
     for(f=0;f<5;f++)
      {
       if(f==opc) draw_trans_sprite(buffer,m_trans,165, 143+60*f);
       textout_ex(buffer, (FONT *)(f_fuentes[f==opc?0:1].dat), opciones[f*2+idioma], 170, 130+60*f, -1, -1);
      }
    }
   contador=(contador<59?contador+1:0);
   fm_volcar();
   tcj_comprobar();
   reproducir_musica(0);

   if(pulsado)
    {
     if(!tcj_estado[TCJ_ARR] && !tcj_estado[TCJ_ABJ] && !tcj_estado[TCJ_IZQ] &&
        !tcj_estado[TCJ_DER] && !tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] &&
        !tcj_estado[TCJ_AC2] && !tcj_estado[TCJ_AC4])
       pulsado=0;
    }
   else
    {
     if(tcj_estado[TCJ_ABJ] || tcj_estado[TCJ_DER])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       opc=(opc<4?opc+1:0);
       contador=0;
       pulsado=1;
      }
     else if(tcj_estado[TCJ_ARR] || tcj_estado[TCJ_IZQ])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       opc=(opc?opc-1:4);
       contador=0;
       pulsado=1;
      }
     else if(tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC1] ||
             tcj_estado[TCJ_AC2] || tcj_estado[TCJ_AC4])
      {
       if(opc) reproducir_sonido(23,PLAYMODE_PLAY);
       if(opc==0)
        {elegida=EST_JUEGO;}
       else if(opc==1)
        {elegida=EST_OPCIONES;}
       else if(opc==2)
        {elegida=EST_CREDITOS;}
       else if(opc==3)
        {elegida=EST_REDEFINIR;}
       else if(opc==4)
        {elegida=EST_SALIR;}
       pulsado=1;
      }
    }
  }
 destroy_bitmap(m_trans);
 return elegida;
}

#define fm_escribir_centrado(_T_,_Y_)\
  clear_to_color(bmptxt,ct);\
  textout_centre_ex(bmptxt, (FONT *)(f_fuentes[0].dat), _T_, RES_X/2, -8, -1, -1);\
  draw_trans_sprite(buffer,bmptxt,-5,_Y_+5);\
  draw_sprite(buffer,bmptxt,0,_Y_);

//******************************************************************************
// Funci�n creditos()
//    Cr�ditos del juego.
//******************************************************************************
char creditos(void)
{
 int f,ct=makecol(255,0,255);
 BITMAP *bmptxt=create_bitmap(RES_X,48);

 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 set_trans_blender(0,0,0,100);
 for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

 if(idioma)
  {
   fm_escribir_centrado("Graphics and music:",108);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
   fm_escribir_centrado("D-o-S",168);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado("Code and sound effects:",268);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
   fm_escribir_centrado("Ignacio P�rez Gil",328);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
  }
 else
  {
   fm_escribir_centrado("Gr�ficos y m�sica:",108-25);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
   fm_escribir_centrado("D-o-S",168-25);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado("C�digo y efectos",268-25);
   fm_escribir_centrado("de sonido:",328-25);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
   fm_escribir_centrado("Ignacio P�rez Gil",388-25);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
  }

 do
  {
   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();
  }
 while(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2] &&
       !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4]);

 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);

 fm_escribir_centrado("� Ultimate Play",108);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

 fm_escribir_centrado("the Game - 1985",168);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

 fm_escribir_centrado(idioma?"Remake by":"Remake por",268);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

 fm_escribir_centrado("Retrospec - 2008",328);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

 destroy_bitmap(bmptxt);
 do
  {
   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();
  }
 while(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2] &&
       !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4]);

 reproducir_sonido(23,PLAYMODE_PLAY);
 return EST_MENU;
}

#define fm_num_pctj(_N_)\
  if(_N_==100){numtxt[0]='1';numtxt[1]='0';numtxt[2]='0';numtxt[3]='%';numtxt[4]=0;}\
  else if(_N_<10){numtxt[0]='0'+_N_;numtxt[1]='%';numtxt[2]=0;}\
  else {numtxt[0]='0'+_N_/10;numtxt[1]='0'+_N_%10;numtxt[2]='%';numtxt[3]=0;}
//******************************************************************************
// Funci�n opciones()
//    Men� de opciones.
//******************************************************************************
char opciones(void)
{
 char f, n, pulsado=1, contador=0, opc=0, elegida=4;
 char opciones[8][24]={"Modo ventana","Windowed mode","Modo pantalla completa",
                       "Fullscreen mode","Turn to English","Cambiar a Espa�ol","Salir","Exit"};
 static char pos_cursor[8]={12,13,22,15,15,17,5,4};
 static short pos_x[8]={133,107,7,100,117,75,248,260};
 BITMAP *m_trans=create_bitmap(RES_X,48);
 char numtxt[5];

 set_trans_blender(0,0,0,100);
 for(f=0;f<8;f++) opciones[f][pos_cursor[f]+1]=0;

 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}

 reproducir_sonido(23,PLAYMODE_PLAY);
 textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),idioma?"Sound volume":"Volumen de sonido",RES_X/2,0,-1,-1);
 fm_num_pctj(volson);
 textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),numtxt,RES_X/2,50,-1,-1);
 textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"-",190,50,-1,-1);
 textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"+",420,50,-1,-1);
 for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}

 reproducir_sonido(23,PLAYMODE_PLAY);
 textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),idioma?"Music volume":"Volumen de m�sica",RES_X/2,130,-1,-1);
 fm_num_pctj(volmus);
 textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),numtxt,RES_X/2,180,-1,-1);
 textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"-",190,180,-1,-1);
 textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"+",420,180,-1,-1);
 for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}

 reproducir_sonido(23,PLAYMODE_PLAY);
 if(idioma)
  {
   if(pantalla_completa)
     textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[1],pos_x[1],260,-1,-1);
   else
     textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[3],pos_x[3],260,-1,-1);
   for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
   reproducir_sonido(23,PLAYMODE_PLAY);
   textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[5],pos_x[5],340,-1,-1);
   for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
   reproducir_sonido(23,PLAYMODE_PLAY);
   textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[7],pos_x[7],420,-1,-1);
  }
 else
  {
   if(pantalla_completa)
     textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[0],pos_x[0],260,-1,-1);
   else
     textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[2],pos_x[2],260,-1,-1);
   for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
   reproducir_sonido(23,PLAYMODE_PLAY);
   textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[4],pos_x[4],340,-1,-1);
   for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
   reproducir_sonido(23,PLAYMODE_PLAY);
   textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[6],pos_x[6],420,-1,-1);
  }
 for(n=0;n<10;n++) {reproducir_musica(0);fm_volcar();}
 reproducir_sonido(23,PLAYMODE_PLAY);

 while(elegida==4)
  {
   if(contador==0 || contador==30)
    {
     blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
     for(f=idioma;f<8;f+=2) opciones[f][pos_cursor[f]]=0;
     if(contador==0 && opc>3) opciones[(opc-4)*2+idioma][pos_cursor[(opc-4)*2+idioma]]='_';
     clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));

     fm_num_pctj(volson);
     if(opc<2)
      {
       textout_centre_ex(m_trans,(FONT *)(f_fuentes[0].dat),idioma?"Sound volume":"Volumen de sonido", RES_X/2, -8, -1, -1);
       draw_trans_sprite(buffer,m_trans,-5,13);
       textout_centre_ex(buffer, (FONT *)(f_fuentes[0].dat),idioma?"Sound volume":"Volumen de sonido", RES_X/2, 0, -1, -1);
       clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
       textout_centre_ex(m_trans,(FONT *)(f_fuentes[0].dat),numtxt,RES_X/2,-8,-1,-1);
       if(contador)
         {numtxt[1]=0;}
       else
         {numtxt[1]='_';numtxt[2]=0;}
       if(opc==0)
        {
         numtxt[0]='-';
         textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),numtxt,190,-8,-1,-1);
         textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"+",420,50,-1,-1);
        }
       else
        {
         numtxt[0]='+';
         textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),numtxt,420,-8,-1,-1);
         textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"-",190,50,-1,-1);
        }
       draw_trans_sprite(buffer,m_trans,-5,63);
       draw_sprite(buffer,m_trans,0,58);
      }
     else
      {
       textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),idioma?"Sound volume":"Volumen de sonido",RES_X/2,0,-1,-1);
       textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),numtxt,RES_X/2,50,-1,-1);
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"-",190,50,-1,-1);
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"+",420,50,-1,-1);
      }

     fm_num_pctj(volmus);
     if(opc==2 || opc==3)
      {
       textout_centre_ex(m_trans,(FONT *)(f_fuentes[0].dat),idioma?"Music volume":"Volumen de m�sica", RES_X/2, -8, -1, -1);
       draw_trans_sprite(buffer,m_trans,-5,143);
       textout_centre_ex(buffer, (FONT *)(f_fuentes[0].dat),idioma?"Music volume":"Volumen de m�sica", RES_X/2, 130, -1, -1);
       clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
       textout_centre_ex(m_trans,(FONT *)(f_fuentes[0].dat),numtxt,RES_X/2,-8,-1,-1);
       if(contador)
         {numtxt[1]=0;}
       else
         {numtxt[1]='_';numtxt[2]=0;}
       if(opc==2)
        {
         numtxt[0]='-';
         textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),numtxt,190,-8,-1,-1);
         textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"+",420,180,-1,-1);
        }
       else
        {
         numtxt[0]='+';
         textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),numtxt,420,-8,-1,-1);
         textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"-",190,180,-1,-1);
        }
       draw_trans_sprite(buffer,m_trans,-5,193);
       draw_sprite(buffer,m_trans,0,188);
      }
     else
      {
       textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),idioma?"Music volume":"Volumen de m�sica",RES_X/2,130,-1,-1);
       fm_num_pctj(volmus);
       textout_centre_ex(buffer,(FONT *)(f_fuentes[1].dat),numtxt,RES_X/2,180,-1,-1);
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"-",190,180,-1,-1);
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),"+",420,180,-1,-1);
      }

     if(opc==4 || opc==5)
      {
       textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),opciones[(opc-4)*2+idioma],pos_x[(opc-4)*2+idioma],-8,-1,-1);
       draw_trans_sprite(buffer,m_trans,-5,273);
       textout_ex(buffer,(FONT *)(f_fuentes[0].dat),opciones[(opc-4)*2+idioma],pos_x[(opc-4)*2+idioma],260,-1,-1);
      }
     else
      {
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[(pantalla_completa?0:2)+idioma],pos_x[(pantalla_completa?0:2)+idioma],260,-1,-1);
      }

     if(opc==6)
      {
       textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),opciones[4+idioma],pos_x[4+idioma],-8,-1,-1);
       draw_trans_sprite(buffer,m_trans,-5,353);
       textout_ex(buffer,(FONT *)(f_fuentes[0].dat),opciones[4+idioma],pos_x[4+idioma],340,-1,-1);
      }
     else
      {
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[4+idioma],pos_x[4+idioma],340,-1,-1);
      }

     if(opc==7)
      {
       textout_ex(m_trans,(FONT *)(f_fuentes[0].dat),opciones[6+idioma],pos_x[6+idioma],-8,-1,-1);
       draw_trans_sprite(buffer,m_trans,-5,433);
       textout_ex(buffer,(FONT *)(f_fuentes[0].dat),opciones[6+idioma],pos_x[6+idioma],420,-1,-1);
      }
     else
      {
       textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[6+idioma],pos_x[6+idioma],420,-1,-1);
      }
    }
   contador=(contador<59?contador+1:0);
   fm_volcar();
   tcj_comprobar();
   reproducir_musica(0);

   if(pulsado)
    {
     if(!tcj_estado[TCJ_ARR] && !tcj_estado[TCJ_ABJ] && !tcj_estado[TCJ_IZQ] &&
        !tcj_estado[TCJ_DER] && !tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] &&
        !tcj_estado[TCJ_AC2] && !tcj_estado[TCJ_AC4] && !key[KEY_PGDN])
       pulsado=0;
    }
   else
    {
     if(tcj_estado[TCJ_ABJ] || tcj_estado[TCJ_DER])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       opc=(opc<7?opc+1:0);
       if((pantalla_completa && opc==5) || (!pantalla_completa && opc==4))
         opc++;
       contador=0;
       pulsado=1;
      }
     else if(tcj_estado[TCJ_ARR] || tcj_estado[TCJ_IZQ])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       opc=(opc?opc-1:7);
       if((pantalla_completa && opc==5) || (!pantalla_completa && opc==4))
         opc--;
       contador=0;
       pulsado=1;
      }
     else if(tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC1] ||
             tcj_estado[TCJ_AC2] || tcj_estado[TCJ_AC4])
      {
       if(opc>3) pulsado=1;
       if(opc==0)
        {
         if(volson) {modificar_volson(--volson,-1);reproducir_sonido(23,PLAYMODE_PLAY);}
        }
       else if(opc==1)
        {
         if(volson<100) {modificar_volson(++volson,-1);reproducir_sonido(23,PLAYMODE_PLAY);}
        }
       else if(opc==2)
        {
         if(volmus) {modificar_volmus(--volmus);reproducir_sonido(23,PLAYMODE_PLAY);}
        }
       else if(opc==3)
        {
         if(volmus<100) {modificar_volmus(++volmus);reproducir_sonido(23,PLAYMODE_PLAY);}
        }
       else if(opc==4 || opc==5)
        {
         if(!set_gfx_mode(pantalla_completa?GFX_AUTODETECT_WINDOWED:GFX_AUTODETECT_FULLSCREEN, RES_X, RES_Y, 0, 0))
           pantalla_completa=(pantalla_completa?0:1);
         opc=(opc==4?5:4);
        }
       else if(opc==6)
        {idioma=(idioma?0:1);}
       else if(opc==7)
        {elegida=EST_MENU;}
       if(opc>3) reproducir_sonido(23,PLAYMODE_PLAY);
       contador=0;
      }
     else if(key[KEY_PGDN])
      {
       pulsado=1;
       nuevo_marcador=(nuevo_marcador?0:1);
       textout_ex(buffer,font,nuevo_marcador?"Marcador activado":"Marcador desactivado",10,470,-1,-1);
       contador=(contador>30?31:1);
      }
    }
  }
 destroy_bitmap(m_trans);
 escribir_cfg();
 return elegida;
}
#undef fm_num_pctj(_N_)

//******************************************************************************
// Funci�n preguntar()
//    Pregunta si se quiere enviar la puntuaci�n.
//******************************************************************************
char preguntar(void)
{
 int ct=makecol(255,0,255);
 char opciones[3][5]={"S�","Yes","No"};
 char opc=0, contador=0, pulsado=1;
 unsigned char elegida=10;
 BITMAP *bmptxt=create_bitmap(RES_X,48);
 char linea[]="00 000 000 0000 xxxxxxxxxxxxxxxxxx";
 char nombre[22];
 FILE *salpnt=NULL;
 int resultado;

 // Si el fichero de salida no existe, lo creamos y escribimos el t�tulo.
 reproducir_musica(0);
 if(exists("scores.txt"))
  {
   salpnt=fopen("scores.txt","a");
  }
 else
  {
   salpnt=fopen("scores.txt","a");
   fputs("CA CSV EXP LYRS               CODE",salpnt);
   putc(0x0d,salpnt);
   putc(0x0a,salpnt);
  }

 reproducir_musica(0);
 linea[0]+=puntuaciones[0]/10;
 linea[1]+=puntuaciones[0]%10;

 linea[3]+=puntuaciones[1]/100;
 linea[4]+=(puntuaciones[1]%100)/10;
 linea[5]+=puntuaciones[1]%10;

 linea[7]+=puntuaciones[2]/100;
 linea[8]+=(puntuaciones[2]%100)/10;
 linea[9]+=puntuaciones[2]%10;

 linea[11]+=puntuaciones[3]/1000;
 linea[12]+=(puntuaciones[3]%1000)/100;
 linea[13]+=(puntuaciones[3]%100)/10;
 linea[14]+=puntuaciones[3]%10;

 fputs(linea,salpnt);
 putc(0x0d,salpnt);
 putc(0x0a,salpnt);
 fclose(salpnt);
 reproducir_musica(0);

 contador=0;
 set_trans_blender(0,0,0,100);
 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 for(contador=0;contador<10;contador++) {reproducir_musica(0);fm_volcar();}
 reproducir_sonido(23,PLAYMODE_PLAY);
 fm_escribir_centrado((idioma?"Do you want to send":"�Quieres enviar tu"),58);
 fm_escribir_centrado((idioma?"your score to the":"puntuaci�n a la tabla"),108);
 fm_escribir_centrado((idioma?"online hiscore table?":"de puntuaciones online?"),158);
 for(contador=0;contador<10;contador++) {reproducir_musica(0);fm_volcar();}
 reproducir_sonido(23,PLAYMODE_PLAY);
 textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[idioma],260,240,-1,-1);
 for(contador=0;contador<10;contador++) {reproducir_musica(0);fm_volcar();}
 reproducir_sonido(23,PLAYMODE_PLAY);
 textout_ex(buffer,(FONT *)(f_fuentes[1].dat),opciones[2],260,320,-1,-1);
 for(contador=0;contador<10;contador++) {reproducir_musica(0);fm_volcar();}

 contador=opciones[0][3]=opciones[1][4]=opciones[2][3]=0;
 while(elegida==10)
  {
   reproducir_musica(0);
   if(contador==0 || contador==30)
    {
     blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
     fm_escribir_centrado((idioma?"Do you want to send":"�Quieres enviar tu"),58);
     fm_escribir_centrado((idioma?"your score to the":"puntuaci�n a la tabla"),108);
     fm_escribir_centrado((idioma?"online hiscore table?":"de puntuaciones online?"),158);
     opciones[0][2]=opciones[1][3]=opciones[2][2]=0;
     if(contador==0) opciones[opc?2:idioma][!opc&&idioma?3:2]='_';

     clear_to_color(bmptxt,ct);
     textout_ex(bmptxt,(FONT *)(f_fuentes[0].dat),opciones[opc?2:idioma],260,-8,-1, -1);
     draw_trans_sprite(buffer,bmptxt,-5,opc?333:253);
     textout_ex(buffer,(FONT *)(f_fuentes[opc?1:0].dat),opciones[idioma],260,240,-1,-1);
     textout_ex(buffer,(FONT *)(f_fuentes[opc?0:1].dat),opciones[2],260,320,-1,-1);
    }
   fm_volcar();
   contador=(contador<59?contador+1:0);
   tcj_comprobar();

   if(pulsado)
    {
     if(!tcj_estado[TCJ_ARR] && !tcj_estado[TCJ_ABJ] && !tcj_estado[TCJ_IZQ] &&
        !tcj_estado[TCJ_DER] && !tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] &&
        !tcj_estado[TCJ_AC2] && !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4])
       pulsado=0;
    }
   else
    {
     if(tcj_estado[TCJ_ARR] || tcj_estado[TCJ_ABJ] || tcj_estado[TCJ_IZQ] || tcj_estado[TCJ_DER])
      {
       opc=(opc?0:1);
       contador=0;
       pulsado=1;
       reproducir_sonido(23,PLAYMODE_PLAY);
      }
     else if(tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC1] ||
             tcj_estado[TCJ_AC2] || tcj_estado[TCJ_AC4])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       elegida=opc;
      }
    }
  }

 if(elegida)
  { // Ha elegido no enviar
   destroy_bitmap(bmptxt);
   return EST_MENU;
  }

 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 for(contador=0;contador<10;contador++) fm_volcar();
 reproducir_sonido(23,PLAYMODE_PLAY);
 contador=0;
 elegida=10;
 nombre[opc=0]='_';
 nombre[opc+1]=0;

 clear_keybuf();
 while(elegida!=13)
  {
   reproducir_musica(0);
   if(contador==0)
    {
     blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
     fm_escribir_centrado((idioma?"Please write":"Por favor, escribe"),58);
     fm_escribir_centrado((idioma?"your name:":"tu nombre:"),108);
     fm_escribir_centrado(nombre,208);
     contador=1;
    }

   fm_volcar();
   tcj_comprobar();

   if(tcj_estado[TCJ_AC3])
     elegida=13;
   else
     if(keypressed())
      {
       contador=0;
       elegida=((unsigned char)readkey())&0xff;
       if(elegida==8)
        { // Se ha pulsado suprimir;
         if(opc)
          {
           reproducir_sonido(23,PLAYMODE_PLAY);
           nombre[opc--]=0;
           nombre[opc]='_';
          }
        }
       else
        {
         if(opc<20 && elegida>=' ' && !(elegida>=LETRA_CERR_0 && elegida<=LETRA_VDAS_0+6))
          {
           reproducir_sonido(23,PLAYMODE_PLAY);
           nombre[opc++]=elegida;
           nombre[opc]='_';
           nombre[opc+1]=0;
          }
        }
      }
  }

 reproducir_sonido(23,PLAYMODE_PLAY);
 if(tcj_estado[TCJ_AC3])
  { // Ha elegido no enviar
   destroy_bitmap(bmptxt);
   return EST_MENU;
  }

 parar_musica();
 nombre[opc]=0;
 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 fm_escribir_centrado((idioma?"Sending score...":"Enviando..."),208);
 fm_volcar();
 if((resultado=inet_connect("retrospec.sgn.net"))==INET_SUCCESS)
  {char codigo[55];
   int f,n;
   for(f=16,n=0;linea[f];f++,n++)
     if(linea[f]=='+')
      {codigo[n++]='%';codigo[n++]='2';codigo[n]='B';}
     else
       codigo[n]=linea[f];
   codigo[n]=0;
   resultado=inet_sendscore(nombre,codigo);
  }
 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 reproducir_sonido(23,PLAYMODE_PLAY);
 if(resultado==INET_SUCCESS)
  {fm_escribir_centrado((idioma?"Done!":"�Hecho!"),208);}
 else
  {fm_escribir_centrado((idioma?"Error!":"�Error!"),208);}

 destroy_bitmap(bmptxt);
 do
  {
   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();
  }
 while(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2] &&
       !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4]);

 reproducir_sonido(23,PLAYMODE_PLAY);
 return EST_MENU;
}
#undef fm_escribir_centrado(_T_,_Y_)

//******************************************************************************
// Funci�n redefinir()
//    Redefine el teclado.
//******************************************************************************
#define NUM_TECLAS_POSIBLES 75
char redefinir(void)
{
 char tecla_elegida[7]={-1,-1,-1,-1,-1,-1,-1};
 char num_elegida[7]={-1,-1,-1,-1,-1,-1,-1};
 char tecla_posible[NUM_TECLAS_POSIBLES]=
   {KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,KEY_J,
    KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,
    KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z,KEY_0,KEY_1,KEY_2,KEY_3,
    KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_F1,KEY_F2,KEY_F3,KEY_F4,
    KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,
    KEY_0_PAD,KEY_1_PAD,KEY_2_PAD,KEY_3_PAD,KEY_4_PAD,KEY_5_PAD,
    KEY_6_PAD,KEY_7_PAD,KEY_8_PAD,KEY_9_PAD,KEY_UP,KEY_RIGHT,
    KEY_DOWN,KEY_LEFT,KEY_SPACE,KEY_LCONTROL,KEY_LSHIFT,KEY_ALT,
    KEY_TAB,KEY_ENTER,KEY_MINUS_PAD,KEY_PLUS_PAD,KEY_DEL_PAD,
    KEY_ENTER_PAD,KEY_RSHIFT,KEY_RCONTROL,KEY_ALTGR};
 char *nombre_tecla[]={"ARR"   ,"UP",
                       "DER"   ,"RIGHT",
                       "ABJ"   ,"DOWN",
                       "IZQ"   ,"LEFT",
                       "ESP"   ,"SPACE",
                       "CTRLI" ,"LCTRL",
                       "SHFTI" ,"LSHIFT",
                       "ALT"   ,"ALT",
                       "TAB"   ,"TAB",
                       "INTRO" ,"ENTER",
                       "-PAD"  ,"-PAD",
                       "+PAD"  ,"+PAD",
                       "SUPR"  ,"DEL",
                       "INTPD" ,"ENPAD",
                       "SHFTD" ,"RSHIFT",
                       "CTRLD" ,"RCTRL",
                       "ALTGR" ,"ALTGR"};
 char *desc_opc[]={"Arriba izq: "   ,"Up left: ",
                   "Arriba der: "   ,"Up right: ",
                   "Abajo der: "    ,"Down right: ",
                   "Abajo izq: "    ,"Down left: ",
                   "Saltar: "       ,"Jump: ",
                   "Tomar/soltar: " ,"Get/drop: ",
                   "Ambas: "        ,"Both: "};
 char tcl,tclant,f,n,i,todas_elegidas=0,pulsado=1;
 BITMAP *bmptxt=create_bitmap(RES_X,48);
 int ct=makecol(255,0,255);

 for(tclant=-1,tcl=0;tcl<7;)
  {
   // Se comprueba si hay que redibujar la pantalla.
   if(tclant<tcl)
    {
     tclant=tcl;
     blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
     clear_to_color(bmptxt,ct);
     textout_centre_ex(bmptxt, (FONT *)(f_fuentes[todas_elegidas?1:0].dat),
                       idioma?"Press a key for:":"Pulsa una tecla para:", RES_X/2, -8, -1, -1);
     if(!todas_elegidas) draw_trans_sprite(buffer,bmptxt,-5,10+5);
     draw_sprite(buffer,bmptxt,0,10);

     for(f=0;f<=tcl;f++)
      {
       char cd[10];
       n=0;
       if(num_elegida[f]<0)
        {cd[n++]='_';} // Tecla por definir.
       else if(num_elegida[f]<26)
        {cd[n++]='A'+num_elegida[f];} // Letra.
       else if(num_elegida[f]<36)
        {cd[n++]='0'+num_elegida[f]-26;} // N�mero.
       else if(num_elegida[f]<45)
        {cd[n++]='F';cd[n++]='1'+num_elegida[f]-36;} // Tecla de funci�n.
       else if(num_elegida[f]<48)
        {cd[n++]='F';cd[n++]='1';cd[n++]='0'+num_elegida[f]-45;} // Tecla de funci�n.
       else if(num_elegida[f]<58)
        {cd[n++]='0'+num_elegida[f]-48;cd[n++]='P';cd[n++]='A';cd[n++]='D';} // Tecla de funci�n.
       else
        {
         for(i=0;nombre_tecla[(num_elegida[f]-58)*2+idioma][i];i++,n++)
           cd[n]=nombre_tecla[(num_elegida[f]-58)*2+idioma][i];
        }
       cd[n]=0;

       clear_to_color(bmptxt,ct);
       textout_ex(bmptxt, (FONT *)(f_fuentes[f==tcl&&!todas_elegidas?0:1].dat), desc_opc[f*2+idioma], 50, -8, -1, -1);
       textout_ex(bmptxt, (FONT *)(f_fuentes[f==tcl&&!todas_elegidas?0:1].dat), cd, 450, -8, -1, -1);
       if(f==tcl&&!todas_elegidas) draw_trans_sprite(buffer,bmptxt,-5,(f+1)*60+5+10);
       draw_sprite(buffer,bmptxt,0,(f+1)*60+10);
      }
    }

   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();

   // Se comprueba si se ha pulsado una tecla.
   if(todas_elegidas)
    {
     todas_elegidas=0;
     tcl++;
    }
   else
    {
     if(pulsado)
      {
       pulsado=0;
       for(f=0;f<NUM_TECLAS_POSIBLES;f++)
        {
         if(key[tecla_posible[f]])
          {
           pulsado=1;
           f=NUM_TECLAS_POSIBLES;
          }
        }
      }
     else
      {
       for(f=0;f<NUM_TECLAS_POSIBLES;f++)
        {
         if(key[tecla_posible[f]])
          {
           for(n=0;n<7;n++)
             if(tecla_elegida[n]==tecla_posible[f])
               n=100;
           if(n<100)
            {
             tecla_elegida[tcl]=tecla_posible[num_elegida[tcl]=f];
             if(tcl==6)
               todas_elegidas=tclant=1;
             else
               tcl++;
             f=NUM_TECLAS_POSIBLES;
             reproducir_sonido(23,PLAYMODE_PLAY);
            }
          }
        }
      }
    }
  }

 destroy_bitmap(bmptxt);
 for(f=0;f<7;f++) teclas_control[f]=tecla_elegida[f];
 ini_entrada();
 escribir_cfg();

 do
  {
   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();
  }
 while(tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC1] || tcj_estado[TCJ_AC2] ||
       tcj_estado[TCJ_AC3] || tcj_estado[TCJ_AC4]);
 do
  {
   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();
  }
 while(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2] &&
       !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4]);
 reproducir_sonido(23,PLAYMODE_PLAY);

 return EST_MENU;
}

//******************************************************************************
// Funci�n intro()
//    introducci�n.
//******************************************************************************
void intro(void)
{
  printf("Init!!!\n");
}

//******************************************************************************
// Funci�n elegir_idioma()
//    Elige un idioma.
//******************************************************************************
void elegir_idioma(void)
{
 char opciones[2][9]={"Espa�ol","English"};
 char opc=0, contador=0, pulsado=1;
 BITMAP *m_trans=create_bitmap(216,57);

 set_trans_blender(0,0,0,100);
 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 for(contador=0;contador<10;contador++) fm_volcar();
 textout_ex(buffer, (FONT *)(f_fuentes[1].dat), opciones[0], 212, 170, -1, -1);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(contador=0;contador<10;contador++) fm_volcar();
 textout_ex(buffer, (FONT *)(f_fuentes[1].dat), opciones[1], 212, 250, -1, -1);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(contador=0;contador<10;contador++) fm_volcar();
 reproducir_sonido(23,PLAYMODE_PLAY);

 contador=opciones[0][8]=opciones[1][8]=0;
 while(idioma==2)
  {
   if(contador==0 || contador==30)
    {
     blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
     opciones[opc][7]=(contador<30?'_':0);
     opciones[(opc?0:1)][7]=0;
     clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
     textout_ex(m_trans, (FONT *)(f_fuentes[0].dat), opciones[opc], 0, 0, -1, -1);
     draw_trans_sprite(buffer,m_trans,207,opc?255:175);
     textout_ex(buffer, (FONT *)(f_fuentes[opc].dat), opciones[0], 212, 170, -1, -1);
     textout_ex(buffer, (FONT *)(f_fuentes[opc?0:1].dat), opciones[1], 212, 250, -1, -1);
    }
   fm_volcar();
   contador=(contador<59?contador+1:0);
   tcj_comprobar();

   if(pulsado)
    {
     if(!tcj_estado[TCJ_ARR] && !tcj_estado[TCJ_ABJ] && !tcj_estado[TCJ_IZQ] &&
        !tcj_estado[TCJ_DER] && !tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] &&
        !tcj_estado[TCJ_AC2] && !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4])
       pulsado=0;
    }
   else
    {
     if(tcj_estado[TCJ_ARR] || tcj_estado[TCJ_ABJ] || tcj_estado[TCJ_IZQ] || tcj_estado[TCJ_DER])
      {
       opc=(opc?0:1);
       contador=0;
       pulsado=1;
       reproducir_sonido(23,PLAYMODE_PLAY);
      }
     else if(tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC1] || tcj_estado[TCJ_AC2] ||
             tcj_estado[TCJ_AC3] || tcj_estado[TCJ_AC4])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       idioma=opc;
      }
    }

  }
 destroy_bitmap(m_trans);
 escribir_cfg();
}

//***********************************\\
//  Funciones de configuraci�n       \\
//***********************************\\

//******************************************************************************
// Funci�n leer_cfg()
//    Carga el fichero de configuraci�n. En caso de no encontrarlo, se pregunta
//  por el idioma y se crea uno con el resto de valores por defecto.
//******************************************************************************
void leer_cfg(void)
{
 PACKFILE *cfg=pack_fopen("cfg.dat",F_READ);

 if(cfg)
  {
   char f;
   pack_fread(&idioma, sizeof(char), cfg);
   pack_fread(&volson, sizeof(char), cfg);
   pack_fread(&volmus, sizeof(char), cfg);
   pack_fread(&pantalla_completa, sizeof(char), cfg);
   for(f=0;f<7;f++) pack_fread(teclas_control+f, sizeof(char), cfg);
   pack_fread(&nuevo_marcador, sizeof(char), cfg);
   pack_fclose(cfg);
  }
 else
  {
   idioma=2;
   volmus=volson=100;
   pantalla_completa=1;
   nuevo_marcador=1;
   teclas_control[0]=KEY_LEFT;
   teclas_control[1]=KEY_UP;
   teclas_control[2]=KEY_RIGHT;
   teclas_control[3]=KEY_DOWN;
   teclas_control[4]=KEY_Z;
   teclas_control[5]=KEY_X;
   teclas_control[6]=KEY_C;
  }
 modificar_volmus(volmus);
 modificar_volson(volson,-1);
}

//******************************************************************************
// Funci�n escribir_cfg()
//    Se escribe el fichero de configuraci�n.
//******************************************************************************
void escribir_cfg(void)
{
 PACKFILE *cfg=pack_fopen("cfg.dat",F_WRITE);
 char f;

 pack_fwrite(&idioma, sizeof(char), cfg);
 pack_fwrite(&volson, sizeof(char), cfg);
 pack_fwrite(&volmus, sizeof(char), cfg);
 pack_fwrite(&pantalla_completa, sizeof(char), cfg);
 for(f=0;f<7;f++) pack_fwrite(teclas_control+f, sizeof(char), cfg);
 pack_fwrite(&nuevo_marcador, sizeof(char), cfg);
 pack_fclose(cfg);
}
