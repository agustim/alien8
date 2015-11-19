//******************************************************************************
// juego.c
// Funciones del juego.
// Por Ignacio P�rez Gil 18/05/2008.
//******************************************************************************
#include <time.h>
#include <allegro.h>
#define _JUEGOC_
#include "juego.h"
#include "isomot.h"
#include "mapa.h"
#include "tiempo.h"
#include "tcj.h"
#include "audio.h"

unsigned char h;
char puerta[4];
unsigned char dx, dy;
char embolsando=1;
char puede_saltar;
char canal_llave;
#define X_ORG 319
#define Y_ORG 156
int x_org,y_org;
#define ANCHO_CELDA 18
#define ALTURA_BASE 30
BITMAP *mapa_vidas=NULL;
BITMAP *mapa_cerraduras=NULL;
BITMAP *mapa_al=NULL;

void cargar_hab(void);
void ini_partida(void);

#define NORM_0   1
#define OBST_0   9
#define ASTR_0  16
#define MOV_0   22
#define JOY_0   24
#define PUE_0   32
#define PUE_1   44
#define PUE_2   50
#define PUE_C   70
#define fm_norm(_N_)         (BITMAP *)(f_obj[NORM_0+tipo_blq[(_N_)%3]].dat)
#define fm_obst(_N_)         (BITMAP *)(f_obj[OBST_0-OBS00+_N_].dat)
#define fm_cerr()            (BITMAP *)(f_obj[0].dat)
#define fm_astr(_N_,_T_)     (BITMAP *)(f_obj[ASTR_0+(_N_)*3+(_T_)].dat)
#define fm_mov(_N_)          (BITMAP *)(f_obj[MOV_0+(_N_==MOV01?0:1)].dat)
#define fm_joy(_N_,_T_)      (BITMAP *)(f_obj[(_N_<0?NORM_0:JOY_0+_N_+((_T_)*4))].dat)
#define fm_puertax(_T_,_N_)  (BITMAP *)(f_obj[PUE_0+(_T_==2?6:0)+_N_].dat)
#define fm_puertay(_T_,_N_)  (BITMAP *)(f_obj[PUE_0+(_T_==2?9:3)+_N_].dat)
#define fm_puertap(_T_,_N_)  (BITMAP *)(f_obj[PUE_1+_T_*3+_N_].dat)
#define fm_puertasi(_T_,_N_) (BITMAP *)(f_obj[PUE_2+_T_*2+_N_+10].dat)
#define fm_puertasd(_T_,_N_) (BITMAP *)(f_obj[PUE_2+_T_*2+_N_].dat)
#define fm_puertacrr(_N_)    (BITMAP *)(f_obj[PUE_C+_N_].dat)

#define NUM_TIPOS_HC  4
#define NUM_BOLSLL_0 32
#define fm_suelo(_N_)             (BITMAP *)(f_esc[1+_N_].dat)
#define fm_pared_sy(_N_,_T_)      (BITMAP *)(f_esc[1+NUM_TIPOS_HC+_N_*2+_T_].dat)
#define fm_pared_sx(_N_,_T_)      (BITMAP *)(f_esc[1+NUM_TIPOS_HC*3+_N_*2+_T_].dat)
#define fm_pared_ix()             (BITMAP *)(f_esc[30].dat)
#define fm_pared_iy()             (BITMAP *)(f_esc[31].dat)
#define fm_pared_msx(_N_)        ((BITMAP *)(f_esc[36+(_N_)].dat))
#define fm_pared_msy(_N_)        ((BITMAP *)(f_esc[39+(_N_)].dat))

#define fm_bolsillo(_N_)          (BITMAP *)(f_esc[NUM_BOLSLL_0+(_N_)].dat)
#define fm_escenario_pasillo(_N_) (BITMAP *)(f_esc[1+NUM_TIPOS_HC*5+_N_].dat)

#define TLBT_0      0
#define RBAL_0      4
#define MINA_0     24
#define SATR_0     32
#define RATA_0     40
#define PLAS_0     56
#define SOMB_0     64
#define ROBS_0    133
#define LLAVE_0    72
#define NUMS_AL_0 121
#define fm_telebot(_N_)        (BITMAP *)(f_anim[TLBT_0+_N_].dat)
#define fm_robotal(_N_,_F_)    (BITMAP *)(f_anim[RBAL_0+_N_*5+_F_].dat)
#define fm_minat(_N_)          (BITMAP *)(f_anim[MINA_0+_N_].dat)
#define fm_saturno(_N_)        (BITMAP *)(f_anim[SATR_0+(_N_)].dat)
#define fm_ratamec(_D_,_F_)    (BITMAP *)(f_anim[RATA_0+_D_*4+_F_].dat)
#define fm_plasma(_N_)         (BITMAP *)(f_anim[PLAS_0+_N_].dat)
#define fm_sombra(_N_)         (BITMAP *)(f_anim[SOMB_0+_N_].dat)
#define fm_robot_obsoleto(_N_) (BITMAP *)(f_anim[ROBS_0+_N_-ROBS0].dat)
#define fm_nacho(_D_,_F_)      (BITMAP *)(f_robots[_D_*5+_F_+dt_anim_nacho.tipo*32].dat)
#define fm_nacho_giro(_D_,_F_) (BITMAP *)(f_robots[_D_*3+_F_+dt_anim_nacho.tipo*32+20].dat)
#define fm_nacho_sgiro(_F_)    (BITMAP *)(f_robots[_F_+33].dat)
#define fm_nacho_aplastado()   (BITMAP *)(f_robots[32].dat)
#define fm_llave(_T_,_N_)      (BITMAP *)(f_anim[_N_+LLAVE_0+1+_T_*11].dat)
#define fm_sombra_llave(_T_)   (BITMAP *)(f_anim[_T_*11+LLAVE_0].dat)
#define fm_nums_al(_T_)        (BITMAP *)(f_anim[_T_+NUMS_AL_0+oscuridad*2].dat)

// Datos para los objetos con funci�n de movimiento.
// La posici�n 0 est� reservada para Nacho.
// Las posiciones 1 y 2 est�n reservadas para las llaves.
// Las posiciones 3, 4 y 5 est�n reservadas para los bolsillos.
#define MAX_MOVS 40
char num_movs;
struct{
 ism_id id, anclado;
 int m0,m1;
 unsigned char sentmov;
 void (*mover)(char);
 char dir;
 short g;
 char empujable;
} movs[MAX_MOVS];
char primera_cascara; // �ndice de la primera c�scara.
char primera_mina_voladora; // �ndice de la primera mina voladora.

// Datos varios referentes a la animaci�n de Nacho.
struct{
 char cont;
 char fot;
 char tipo;
 char dirini;
} dt_anim_nacho;
char fot_nacho[17]={2,3,3,4,4,3,3,2,2,1,1,0,0,1,1,2,2};

// Identificadores de las jambas de las puertas
ism_id id_jamba[8];

// Datos varios referentes a la partida en curso.
struct{
 char cerraduras;
 char vidas;
 unsigned char crionautas;
} dt_partida;

// Datos para los telebots y sus joysticks
struct{
 ism_id id[5];      // Identificadores de los bloques del joystick.
 ism_id id_t[2];    // Identificadores de los telebots.
 char tc[5];        // Indica que bloques est�n pisados anteriormente.
 char ta[5];        // Indica que bloques est�n pisados ahora.
} dt_telebot;

// Datos para los obst�culos
#define MAX_OBST 35
char num_obst;
ism_id id_obst[MAX_OBST];

char sonido_interferencia;
char sonido_empujar;
char sonido_autom;
char trampas;

void mov_nacho_andar(char f);
void mov_nacho_saltar(char f);
void mov_nacho_fotograma(void);
void mov_nacho_girar(char f);
void mov_nacho_explotar(char f);
void mov_desaparece(char f);
void mov_xy_autom(char f);
void mov_transportar(char f);
void mov_z_pisado(char f);
void mov_zmn_pisado(char f);
void mov_mina_voladora(char f);
void mov_mina_terrestre(char f);
void mov_z_autom(char f);
void mov_joystick(char f);
void mov_telebot(char f);
void mov_rbal_l(char f);
void mov_rbal_f(char f);
void mov_ratamec(char f);
void mov_saturno(char f);
void mov_plasma(char f);
void mov_cerradura(char f);
void mov_llave(char f);
void mov_vida(char f);
void mov_empujable(char f);
void mov_cascara(char f);
void mov_astronauta(char f);
void mov_bolsillo(char f);
void mov_empujar(char i, unsigned char dato, int valor);
char anclar(char f);
void embolsar(void);
void abrir_cerradura(void);
char pausa(void);
void reprogramar(void);
void detener_nave(void);
void mostrar_resultado(void);

// Datos y funci�n para los d�gitos del contador de a�os luz
unsigned char pos_a_luz[4], dec_pos_a_luz, oscuridad;
void decrementar_a_luz(char n);
void dibujar_a_luz(void);
#define A_LUZ_X_A 565
#define A_LUZ_Y_A 441
#define A_LUZ_X_N 562
#define A_LUZ_Y_N 440
int   colores_al_32[6][3][4]; // [oscuridad] [blanco,negro,rojo] [sin aclarar,aclarado,aclarado+,aclarado-]
short colores_al_16[6][3][4]; // [oscuridad] [blanco,negro,rojo] [sin aclarar,aclarado,aclarado+,aclarado-]

//******************************************************************************
// Funci�n ini_juego()
//    Inicializa los datos del juego.
//******************************************************************************
void ini_juego()
{
 unsigned char f,n;
 int r,g,b;

 for(f=1;f<NUM_HABS;f++)
   hab[f].n+=hab[f-1].n;
 buffer=create_bitmap(RES_X,RES_Y);
 mapa_al=create_sub_bitmap(buffer,A_LUZ_X_N,A_LUZ_Y_N,46,18);
 f_obj=load_datafile("objetos.dat");
 f_esc=load_datafile("escenarios.dat");
 f_anim=load_datafile("anims.dat");
 f_robots=load_datafile("robots.dat");
 f_fuentes=load_datafile("fuentes.dat");
 mapa_vidas=create_bitmap(fm_pared_msx(0)->w,fm_pared_msx(0)->h);
 mapa_cerraduras=create_bitmap(fm_pared_msx(0)->w,fm_pared_msx(0)->h);
 ism_habilitar_rect_sucio(1);
 srand(time(NULL));
 movs[3].mover=movs[4].mover=movs[5].mover=mov_bolsillo;
 movs[3].empujable=movs[4].empujable=movs[5].empujable=0;

 if(hicolor)
  {
   for(f=0;f<6;f++)
    {
     oscuridad=f;
     colores_al_16[f][0][0]=(short)getpixel(fm_nums_al(0),0,0); // Blanco.
     colores_al_16[f][1][0]=(short)getpixel(fm_nums_al(0),1,3); // Negro.
     colores_al_16[f][2][0]=(short)getpixel(fm_nums_al(1),0,0); // Rojo.
     for(n=0;n<3;n++)
      {
       if(n==1)
         r=g=b=48; // El negro se aclara m�s que los dem�s, para que se note mejor.
       else
        {
         r=getr(colores_al_16[f][n][0]);
         g=getg(colores_al_16[f][n][0]);
         b=getb(colores_al_16[f][n][0]);
        }
       colores_al_16[f][n][1]=(short)makecol((r>255-16?255:r+16),(g>255-16?255:g+16),(b>255-16?255:b+16)); // Aclarado.
       colores_al_16[f][n][2]=(short)makecol((r>255-32?255:r+32),(g>255-32?255:g+32),(b>255-32?255:b+32)); // Aclarado+.
       colores_al_16[f][n][3]=(short)makecol((r>255-8 ?255:r+8 ),(g>255-8 ?255:g+8 ),(b>255-8 ?255:b+8 )); // Aclarado-.
      }
    }
  }
 else
  {
   for(f=0;f<6;f++)
    {
     oscuridad=f;
     colores_al_32[f][0][0]=getpixel(fm_nums_al(0),0,0); // Blanco.
     colores_al_32[f][1][0]=getpixel(fm_nums_al(0),1,3); // Negro.
     colores_al_32[f][2][0]=getpixel(fm_nums_al(1),0,0); // Rojo.
     for(n=0;n<3;n++)
      {
       if(n==1)
         r=g=b=48; // El negro se aclara m�s que los dem�s, para que se note mejor.
       else
        {
         r=getr(colores_al_32[f][n][0]);
         g=getg(colores_al_32[f][n][0]);
         b=getb(colores_al_32[f][n][0]);
        }
       colores_al_32[f][n][1]=makecol((r>255-16?255:r+16),(g>255-16?255:g+16),(b>255-16?255:b+16)); // Aclarado.
       colores_al_32[f][n][2]=makecol((r>255-32?255:r+32),(g>255-32?255:g+32),(b>255-32?255:b+32)); // Aclarado+.
       colores_al_32[f][n][3]=makecol((r>255-8 ?255:r+8 ),(g>255-8 ?255:g+8 ),(b>255-8 ?255:b+8 )); // Aclarado-.
      }
    }
  }
}

//******************************************************************************
// Funci�n juego()
//    Bucle principal del juego.
//******************************************************************************
char juego(void)
{
 char f;//,tmp=1;
 int terminar=0;

 puede_saltar=trampas=0;
 dt_anim_nacho.cont=dt_anim_nacho.fot=dt_anim_nacho.tipo=dt_anim_nacho.dirini=0;

 if(nuevo_marcador)
  {
   masked_blit((BITMAP *)(f_esc[37].dat),(BITMAP *)(f_esc[22].dat),0,0,(((BITMAP *)(f_esc[22].dat))->w)-20,0,20,17);
   if((mapa_vidas->w)!=fm_pared_msx(0)->w)
    {
     destroy_bitmap(mapa_vidas);
     destroy_bitmap(mapa_cerraduras);
     mapa_vidas=create_bitmap(fm_pared_msx(0)->w,fm_pared_msx(0)->h);
     mapa_cerraduras=create_bitmap(fm_pared_msx(0)->w,fm_pared_msx(0)->h);
    }
  }
 else
  {
   rectfill((BITMAP *)(f_esc[22].dat), (((BITMAP *)(f_esc[22].dat))->w)-20, 0, (((BITMAP *)(f_esc[22].dat))->w)-1, 16, 0);
   if((mapa_vidas->w)==fm_pared_msx(0)->w)
    {
     destroy_bitmap(mapa_vidas);
     destroy_bitmap(mapa_cerraduras);
     mapa_vidas=create_bitmap(41,57);
     mapa_cerraduras=create_bitmap(74,57);
    }
  }
 ini_partida();
 cargar_hab();

 reproducir_sonido(30,-1);
 do
  {
   reproducir_musica(1+(dt_partida.cerraduras&1));
   tcj_comprobar();
   if(key[KEY_6] && key[KEY_F12])
    {
     trampas=1;
    }

   for(f=0;f<num_movs;f++) movs[f].mover(f);
   if(sonido_empujar) sonido_empujar--;
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   if(trampas)
     textout_ex(buffer, font, "*", 10, 10, -1, -1);
   else
    {// Se decrementa y dibuja el contador de a�os luz
     if(dec_pos_a_luz && dt_partida.cerraduras<24)
      {
       dec_pos_a_luz=0;
       decrementar_a_luz(3);
      }
     else
      dec_pos_a_luz=1;
    }
   dibujar_a_luz();
   fm_volcar();
   if(sonido_autom>0) sonido_autom--;
   if(tcj_estado[TCJ_AC3]) terminar=pausa();
  }while(!terminar && dt_partida.cerraduras<24 && dt_partida.vidas>=0 &&
         (pos_a_luz[0]<180 || pos_a_luz[1]<180 || pos_a_luz[2]<180 || pos_a_luz[3]<180));

 detener_sonidos(1);
 parar_musica();
 if(!terminar && !trampas)
  {
   if(dt_partida.cerraduras<24)
     reprogramar();
   else
     detener_nave();
   mostrar_resultado();
  }
 detener_sonidos(1);
 if(terminar) reproducir_sonido(23,PLAYMODE_PLAY);
 return (terminar||trampas||dt_partida.cerraduras==0?EST_MENU:EST_PREGUNTAR);
}

//******************************************************************************
// Funci�n decrementer_a_luz(char n)
//    Calcula las nuevas posiciones de los n�meros del marcador de a�os luz.
//******************************************************************************
void decrementar_a_luz(char n)
{
 pos_a_luz[n]--;
 if(pos_a_luz[n]==0)
  {pos_a_luz[n]=180;}
 else if(pos_a_luz[n]<180 && pos_a_luz[n]>=18*9)
  {decrementar_a_luz(n-1);}
}

//******************************************************************************
// Funci�n dibujar_a_luz()
//    Dibuja los n�meros del marcador de a�os luz.
//******************************************************************************
void dibujar_a_luz(void)
{
 int f;

 if(nuevo_marcador)
  {
   blit(fm_pared_iy(),mapa_al,206,108,0,0,46,18);
   set_trans_blender(0,0,0,128);
   for(f=0;f<3;f++)
     draw_trans_sprite(mapa_al,fm_nums_al(0),f*12,-pos_a_luz[f]);
   draw_trans_sprite(mapa_al,fm_nums_al(1),f*12,-pos_a_luz[f]);
  }
 else
  {
   for(f=0;f<3;f++)
     blit(fm_nums_al(0),buffer,0,pos_a_luz[f],A_LUZ_X_A+f*12,A_LUZ_Y_A,10,18);
   blit(fm_nums_al(1),buffer,0,pos_a_luz[f],A_LUZ_X_A+f*12,A_LUZ_Y_A,10,18);

   if(hicolor)
    {
     short *la=(short *)buffer->line[A_LUZ_Y_A+6];
     short *l0=(short *)buffer->line[A_LUZ_Y_A+7];
     short *l1=(short *)buffer->line[A_LUZ_Y_A+8];
     short *l2=(short *)buffer->line[A_LUZ_Y_A+9];
     short *l3=(short *)buffer->line[A_LUZ_Y_A+10];
     short *lb=(short *)buffer->line[A_LUZ_Y_A+11];
     for(f=A_LUZ_X_A;f<(A_LUZ_X_A+46);f++)
      {
       if(la[f]==colores_al_16[oscuridad][2][0])
        {la[f]=colores_al_16[oscuridad][2][3];}
       else if(la[f]==colores_al_16[oscuridad][0][0])
        {la[f]=colores_al_16[oscuridad][0][3];}
       else if(la[f]==colores_al_16[oscuridad][1][0])
        {la[f]=colores_al_16[oscuridad][1][3];}

       if(l0[f]==colores_al_16[oscuridad][2][0])
        {l0[f]=colores_al_16[oscuridad][2][1];}
       else if(l0[f]==colores_al_16[oscuridad][0][0])
        {l0[f]=colores_al_16[oscuridad][0][1];}
       else if(l0[f]==colores_al_16[oscuridad][1][0])
        {l0[f]=colores_al_16[oscuridad][1][1];}

       if(l1[f]==colores_al_16[oscuridad][2][0])
        {l1[f]=colores_al_16[oscuridad][2][2];}
       else if(l1[f]==colores_al_16[oscuridad][0][0])
        {l1[f]=colores_al_16[oscuridad][0][2];}
       else if(l1[f]==colores_al_16[oscuridad][1][0])
        {l1[f]=colores_al_16[oscuridad][1][2];}

       if(l2[f]==colores_al_16[oscuridad][2][0])
        {l2[f]=colores_al_16[oscuridad][2][2];}
       else if(l2[f]==colores_al_16[oscuridad][0][0])
        {l2[f]=colores_al_16[oscuridad][0][2];}
       else if(l2[f]==colores_al_16[oscuridad][1][0])
        {l2[f]=colores_al_16[oscuridad][1][2];}

       if(l3[f]==colores_al_16[oscuridad][2][0])
        {l3[f]=colores_al_16[oscuridad][2][1];}
       else if(l3[f]==colores_al_16[oscuridad][0][0])
        {l3[f]=colores_al_16[oscuridad][0][1];}
       else if(l3[f]==colores_al_16[oscuridad][1][0])
        {l3[f]=colores_al_16[oscuridad][1][1];}

       if(lb[f]==colores_al_16[oscuridad][2][0])
        {lb[f]=colores_al_16[oscuridad][2][3];}
       else if(lb[f]==colores_al_16[oscuridad][0][0])
        {lb[f]=colores_al_16[oscuridad][0][3];}
       else if(lb[f]==colores_al_16[oscuridad][1][0])
        {lb[f]=colores_al_16[oscuridad][1][3];}
      }
    }
   else
    {
     int *la=(int *)buffer->line[A_LUZ_Y_A+6];
     int *l0=(int *)buffer->line[A_LUZ_Y_A+7];
     int *l1=(int *)buffer->line[A_LUZ_Y_A+8];
     int *l2=(int *)buffer->line[A_LUZ_Y_A+9];
     int *l3=(int *)buffer->line[A_LUZ_Y_A+10];
     int *lb=(int *)buffer->line[A_LUZ_Y_A+11];
     for(f=A_LUZ_X_A;f<(A_LUZ_X_A+46);f++)
      {
       if(la[f]==colores_al_32[oscuridad][2][0])
        {la[f]=colores_al_32[oscuridad][2][3];}
       else if(la[f]==colores_al_32[oscuridad][0][0])
        {la[f]=colores_al_32[oscuridad][0][3];}
       else if(la[f]==colores_al_32[oscuridad][1][0])
        {la[f]=colores_al_32[oscuridad][1][3];}

       if(l0[f]==colores_al_32[oscuridad][2][0])
        {l0[f]=colores_al_32[oscuridad][2][1];}
       else if(l0[f]==colores_al_32[oscuridad][0][0])
        {l0[f]=colores_al_32[oscuridad][0][1];}
       else if(l0[f]==colores_al_32[oscuridad][1][0])
        {l0[f]=colores_al_32[oscuridad][1][1];}

       if(l1[f]==colores_al_32[oscuridad][2][0])
        {l1[f]=colores_al_32[oscuridad][2][2];}
       else if(l1[f]==colores_al_32[oscuridad][0][0])
        {l1[f]=colores_al_32[oscuridad][0][2];}
       else if(l1[f]==colores_al_32[oscuridad][1][0])
        {l1[f]=colores_al_32[oscuridad][1][2];}

       if(l2[f]==colores_al_32[oscuridad][2][0])
        {l2[f]=colores_al_32[oscuridad][2][2];}
       else if(l2[f]==colores_al_32[oscuridad][0][0])
        {l2[f]=colores_al_32[oscuridad][0][2];}
       else if(l2[f]==colores_al_32[oscuridad][1][0])
        {l2[f]=colores_al_32[oscuridad][1][2];}

       if(l3[f]==colores_al_32[oscuridad][2][0])
        {l3[f]=colores_al_32[oscuridad][2][1];}
       else if(l3[f]==colores_al_32[oscuridad][0][0])
        {l3[f]=colores_al_32[oscuridad][0][1];}
       else if(l3[f]==colores_al_32[oscuridad][1][0])
        {l3[f]=colores_al_32[oscuridad][1][1];}

       if(lb[f]==colores_al_32[oscuridad][2][0])
        {lb[f]=colores_al_32[oscuridad][2][3];}
       else if(lb[f]==colores_al_32[oscuridad][0][0])
        {lb[f]=colores_al_32[oscuridad][0][3];}
       else if(lb[f]==colores_al_32[oscuridad][1][0])
        {lb[f]=colores_al_32[oscuridad][1][3];}
      }
    }
  }
}

#define fm_pausa(_N_) if(!tcj_estado[TCJ_AC3]) for(f=0;f<(_N_) && !tcj_estado[TCJ_AC3];f++){ fm_volcar();tcj_comprobar();}
//******************************************************************************
// Funci�n reprogramar()
//    Secuencia de "reprogramaci�n".
//******************************************************************************
void reprogramar(void)
{
 int f,x,y;
 BITMAP *buffer2=create_bitmap(RES_X,RES_Y);
 BITMAP *txt_rep=create_bitmap(384,48);

 clear_to_color(txt_rep,makecol(255,0,255));
 clear_bitmap(buffer2);
 textout_ex(txt_rep,(FONT *)(f_fuentes[0].dat),idioma?"Reprogramming":"Reprogramando",0,-8,-1,-1);
 h=HAB_REPROGRAMANDO;
 cargar_hab();
 ism_mover_objeto(movs[0].id,ANCHO_CELDA*2+1,ANCHO_CELDA*3-3,30,CAMBIAR);
 ism_cambiar_mapa_objeto(movs[0].id,(BITMAP *)(f_robots[36].dat));
 mov_bolsillo(3);mov_bolsillo(4);mov_bolsillo(5);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 dibujar_a_luz();
 blit(buffer,buffer2,0,0,0,0,RES_X,RES_Y);
 fm_pausa(20);

 // Mensaje "Reprogramando" que se enciende y apaga cuatro veces
 set_trans_blender(0,0,0,100);
 tcj_comprobar();
 reproducir_sonido(27, PLAYMODE_PLAY);
 for(x=0;x<4  && !tcj_estado[TCJ_AC3];x++)
  {
   draw_trans_sprite(buffer,txt_rep,(RES_X-txt_rep->w)/2-5,105);
   draw_sprite(buffer,txt_rep,(RES_X-txt_rep->w)/2,100);
   fm_pausa(36)
   blit(buffer2,buffer,0,0,0,0,RES_X,RES_Y);
   fm_pausa(30);
  }

 // Entra en la prensa
 fm_pausa(250)
 if(!tcj_estado[TCJ_AC3])
 for(f=0;f<ANCHO_CELDA*5 && !tcj_estado[TCJ_AC3];f++)
  {
   ism_cambiar_dato_objeto(movs[0].id,D_X,1,SUMAR);
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   if(!f) reproducir_sonido(13, PLAYMODE_LOOP);
   fm_volcar();
   tcj_comprobar();
  }
 detener_sonidos(1);

 // Se cierran las puertas de la prensa
 fm_pausa(20)
 if(!tcj_estado[TCJ_AC3])
 for(f=0;f<ANCHO_CELDA*2 && !tcj_estado[TCJ_AC3];f++)
  {
   if(f==0) reproducir_sonido(28, PLAYMODE_PLAY);
   if(f<ANCHO_CELDA)
    {
     ism_cambiar_dato_objeto(movs[6].id,D_Y, 1,SUMAR);
     ism_cambiar_dato_objeto(movs[7].id,D_Y, 1,SUMAR);
     ism_cambiar_dato_objeto(movs[8].id,D_X,-1,SUMAR);
     ism_cambiar_dato_objeto(movs[9].id,D_X,-1,SUMAR);
    }
   else
    {
     ism_cambiar_dato_objeto(movs[6].id,D_X, 1,SUMAR);
     ism_cambiar_dato_objeto(movs[7].id,D_X, 1,SUMAR);
     ism_cambiar_dato_objeto(movs[8].id,D_Y,-1,SUMAR);
     ism_cambiar_dato_objeto(movs[9].id,D_Y,-1,SUMAR);
    }
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
  }
 detener_sonidos(1);

 // Baja la prensa hasta chocar con Nacho
 fm_pausa(20)
 f=0;
 if(!tcj_estado[TCJ_AC3])
 while(!ism_cambiar_dato_objeto(movs[10].id,D_Z,-1,SUMAR) && !tcj_estado[TCJ_AC3])
  {
   if(f==0) reproducir_sonido(28, PLAYMODE_PLAY);
   f++;
   ism_cambiar_dato_objeto(movs[11].id,D_Z,-1,SUMAR);
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
  }
 detener_sonidos(1);

 // La prensa aplasta a Nacho
 if(!tcj_estado[TCJ_AC3])
  {
   reproducir_sonido(29, PLAYMODE_LOOP);
   ism_quitar_objeto(movs[0].id);
   ism_obtener_coords_libres(7,2,&x,&y);
   movs[0].id=ism_colocar_objeto_libre(x,y,ALTURA_BASE,ANCHO_CELDA,ANCHO_CELDA,25,fm_nacho_aplastado(),fm_sombra(0));
   f=0;
   while(!f && !tcj_estado[TCJ_AC3])
    {
     fm_volcar();
     fm_volcar();
     fm_volcar();
     f=ism_cambiar_dato_objeto(movs[10].id,D_Z,-1,SUMAR);
     if(!f)
      {
       ism_cambiar_dato_objeto(movs[11].id,D_Z,-1,SUMAR);
       ism_dibujar_mundo_isom(buffer,x_org,y_org);
       fm_volcar();
       tcj_comprobar();
      }
    }
  }
 detener_sonidos(1);

 // Sube la prensa
 fm_pausa(20)
 if(!tcj_estado[TCJ_AC3])
 for(f=0;f<ALTURA_BASE*2+5 && !tcj_estado[TCJ_AC3];f++)
  {
   if(f==0) reproducir_sonido(28, PLAYMODE_PLAY);
   ism_cambiar_dato_objeto(movs[11].id,D_Z,1,SUMAR);
   ism_cambiar_dato_objeto(movs[10].id,D_Z,1,SUMAR);
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
  }
 detener_sonidos(1);

 // Se abren las puertas de la prensa
 fm_pausa(20)
 if(!tcj_estado[TCJ_AC3])
 for(f=0;f<ANCHO_CELDA*2 && !tcj_estado[TCJ_AC3];f++)
  {
   if(f==0) reproducir_sonido(28, PLAYMODE_PLAY);
   if(f<ANCHO_CELDA)
    {
     ism_cambiar_dato_objeto(movs[6].id,D_X,-1,SUMAR);
     ism_cambiar_dato_objeto(movs[7].id,D_X,-1,SUMAR);
     ism_cambiar_dato_objeto(movs[8].id,D_Y, 1,SUMAR);
     ism_cambiar_dato_objeto(movs[9].id,D_Y, 1,SUMAR);
    }
   else
    {
     ism_cambiar_dato_objeto(movs[6].id,D_Y,-1,SUMAR);
     ism_cambiar_dato_objeto(movs[7].id,D_Y,-1,SUMAR);
     ism_cambiar_dato_objeto(movs[8].id,D_X, 1,SUMAR);
     ism_cambiar_dato_objeto(movs[9].id,D_X, 1,SUMAR);
    }
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
  }
 detener_sonidos(1);

 // Sale de la prensa
 fm_pausa(20)
 if(!tcj_estado[TCJ_AC3])
 for(f=0;f<ANCHO_CELDA*5 && !tcj_estado[TCJ_AC3];f++)
  {
   if(f==0) reproducir_sonido(13, PLAYMODE_LOOP);
   ism_cambiar_dato_objeto(movs[0].id,D_Y,1,SUMAR);
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
  }
 detener_sonidos(1);

// C�e al suelo
 if(!tcj_estado[TCJ_AC3])
 for(f=0;f>-10 && !tcj_estado[TCJ_AC3];f--)
  {
   ism_cambiar_dato_objeto(movs[0].id,D_Z,f/2-1,SUMAR);
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
   if(f==-9) reproducir_sonido(14,PLAYMODE_PLAY);
  }

 if(!tcj_estado[TCJ_AC3])
 do
  {
   ism_dibujar_mundo_isom(buffer,x_org,y_org);
   fm_volcar();
   tcj_comprobar();
   reproducir_musica(0);
  }while(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2] &&
         !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4]);
 detener_sonidos(1);
 destroy_bitmap(buffer2);
 destroy_bitmap(txt_rep);
}
#undef fm_pausa(_N_)

#define fm_escribir_centrado(_T_,_Y_)\
  clear_to_color(bmptxt,ct);\
  textout_centre_ex(bmptxt, (FONT *)(f_fuentes[0].dat), _T_, RES_X/2, -8, -1, -1);\
  draw_trans_sprite(buffer,bmptxt,-5,_Y_+5);\
  draw_sprite(buffer,bmptxt,0,_Y_);

//******************************************************************************
// Funci�n detener_nave()
//    Secuencia de juego completado.
//******************************************************************************
void detener_nave(void)
{
 short f,n;
 BITMAP *buffer2;
 DATAFILE *f_fin=load_datafile("sfn.dat");

 // Fundido a negro a lo bestia
 for(f=0;f<32;f++)
  {int x,y,r,g,b,c;
   for(y=0;y<RES_Y;y++)
     for(x=0;x<RES_X;x++)
       if(c=getpixel(buffer,x,y))
        {
         r=getr(c);r=(r>8?r-8:0);
         g=getg(c);g=(g>8?g-8:0);
         b=getb(c);b=(b>8?b-8:0);
         putpixel(buffer,x,y,makecol(r,g,b));
        }
   fm_volcar();
   fm_volcar();
  }

 // Comienza la m�sica, y esperamos tres segundos en negro.
 musica_fin(f_fin[8].dat,f_fin[8].size);
 for(f=0;f<TICKS*3;f++) {reproducir_musica(100);fm_volcar();}

 // Fundido a blanco a lo bestia de la primera im�gen.
 buffer2=(BITMAP *)(f_fin[0].dat);
 for(f=0;f<248;f+=8)
  {int x,y,r,g,b,c;
   for(y=0;y<RES_Y;y++)
     for(x=0;x<RES_X;x++)
      {
       c=getpixel(buffer2,x,y);
       r=getr(c);r=(r>f?f:r);
       g=getg(c);g=(g>f?f:g);
       b=getb(c);b=(b>f?f:b);
       putpixel(buffer,x,y,makecol(r,g,b));
      }
   fm_volcar();
   fm_volcar();
  }

 // Esperamos dos segundos.
 for(f=0;f<TICKS*2;f++) {reproducir_musica(100);fm_volcar();}

 // Dibujamos el primer mensaje.
 for(f=10;f<251;f+=4)
  {
   blit(buffer2,buffer,0,0,0,0,RES_X,RES_Y);
   set_trans_blender(0,0,0,f);
   draw_trans_sprite(buffer,(BITMAP *)(f_fin[2+idioma*3].dat),80,120);
   reproducir_musica(100);
   fm_volcar();
  }
 draw_sprite(buffer,(BITMAP *)(f_fin[2+idioma*3].dat),80,120);

 // Esperamos tres segundos.
 for(f=0;f<TICKS*3;f++) {reproducir_musica(100);fm_volcar();}

 // Apagamos el primer mensaje y encendemos el segundo.
 for(f=10;f<251;f+=4)
  {
   blit(buffer2,buffer,0,0,0,0,RES_X,RES_Y);
   set_trans_blender(0,0,0,260-f);
   draw_trans_sprite(buffer,(BITMAP *)(f_fin[2+idioma*3].dat),80,120);
   set_trans_blender(0,0,0,f);
   draw_trans_sprite(buffer,(BITMAP *)(f_fin[3+idioma*3].dat),130,180);
   reproducir_musica(100);
   fm_volcar();
  }
 blit(buffer2,buffer,0,0,0,0,RES_X,RES_Y);
 draw_sprite(buffer,(BITMAP *)(f_fin[3+idioma*3].dat),130,180);

 // Esperamos tres segundos.
 for(f=0;f<TICKS*3;f++) {reproducir_musica(100);fm_volcar();}

 // Fundido a negro a lo bestia
 for(f=0;f<32;f++)
  {int x,y,r,g,b,c;
   for(y=0;y<RES_Y;y++)
     for(x=0;x<RES_X;x++)
       if(c=getpixel(buffer,x,y))
        {
         r=getr(c);r=(r>8?r-8:0);
         g=getg(c);g=(g>8?g-8:0);
         b=getb(c);b=(b>8?b-8:0);
         putpixel(buffer,x,y,makecol(r,g,b));
        }
   fm_volcar();
   fm_volcar();
  }

 // Fundido a blanco a lo bestia de la segunda im�gen.
 buffer2=(BITMAP *)(f_fin[1].dat);
 for(f=0;f<248;f+=8)
  {int x,y,r,g,b,c;
   for(y=0;y<RES_Y;y++)
     for(x=0;x<RES_X;x++)
      {
       c=getpixel(buffer2,x,y);
       r=getr(c);r=(r>f?f:r);
       g=getg(c);g=(g>f?f:g);
       b=getb(c);b=(b>f?f:b);
       putpixel(buffer,x,y,makecol(r,g,b));
      }
   fm_volcar();
   fm_volcar();
  }

 // Esperamos cuatro segundos.
 for(f=0;f<TICKS*4;f++) {reproducir_musica(100);fm_volcar();}

 // Dibujamos el tercer mensaje.
 n=(RES_X-((BITMAP *)(f_fin[4+idioma*3].dat))->w)/2;
 for(f=10;f<251;f+=4)
  {
   blit(buffer2,buffer,0,0,0,0,RES_X,RES_Y);
   set_trans_blender(0,0,0,f);
   draw_trans_sprite(buffer,(BITMAP *)(f_fin[4+idioma*3].dat),n,130);
   reproducir_musica(100);
   fm_volcar();
  }
 draw_sprite(buffer,(BITMAP *)(f_fin[4+idioma*3].dat),n,130);

 // Esperamos cinco segundos.
 for(f=0;f<TICKS*5;f++) {reproducir_musica(100);fm_volcar();}

 // Fundido a negro a lo bestia
 for(f=0;f<32;f++)
  {int x,y,r,g,b,c;
   for(y=0;y<RES_Y;y++)
     for(x=0;x<RES_X;x++)
       if(c=getpixel(buffer,x,y))
        {
         r=getr(c);r=(r>8?r-8:0);
         g=getg(c);g=(g>8?g-8:0);
         b=getb(c);b=(b>8?b-8:0);
         putpixel(buffer,x,y,makecol(r,g,b));
        }
   fm_volcar();
   fm_volcar();
  }

 // Esperamos dos segundos.
 for(f=0;f<TICKS*2;f++) {reproducir_musica(100);fm_volcar();}

 parar_musica();
 unload_datafile(f_fin);
}

//******************************************************************************
// Funci�n mostrar_resultado()
//    Muestra las puntuaciones al terminar la partida.
//******************************************************************************
void mostrar_resultado(void)
{
 short f;
 int ct=makecol(255,0,255);
 BITMAP *bmptxt=create_bitmap(RES_X,48);

 blit((BITMAP *)(f_esc[28].dat),buffer,0,0,0,0,RES_X,RES_Y);
 set_trans_blender(0,0,0,100);
 puntuaciones[0]=dt_partida.cerraduras;
 puntuaciones[1]=dt_partida.crionautas;
 for(puntuaciones[2]=f=0;f<HAB_REPROGRAMANDO;f++)
   if(hab[f].banderas & 0x10) puntuaciones[2]++;

 if(idioma)
  {char texto0[]="Activated cryogenic";
   char texto1[]="chambers...........00";
   char texto2[]="Cryonaughts saved..000";
   char texto3[]="Rooms explored..000";
   char texto4[]="Light years......0000";

   texto1[19]+=dt_partida.cerraduras/10;
   texto1[20]+=dt_partida.cerraduras%10;
   texto2[19]+=dt_partida.crionautas/100;
   texto2[20]+=(dt_partida.crionautas%100)/10;
   texto2[21]+=dt_partida.crionautas%10;
   texto3[16]+=puntuaciones[2]/100;
   texto3[17]+=(puntuaciones[2]%100)/10;
   texto3[18]+=puntuaciones[2]%10;
   texto4[17]+=(pos_a_luz[0]>162?0:pos_a_luz[0]/18+(pos_a_luz[0]%18?1:0));
   texto4[18]+=(pos_a_luz[1]>162?0:pos_a_luz[1]/18+(pos_a_luz[1]%18?1:0));
   texto4[19]+=(pos_a_luz[2]>162?0:pos_a_luz[2]/18+(pos_a_luz[2]%18?1:0));
   texto4[20]+=(pos_a_luz[3]>162?0:pos_a_luz[3]/18+(pos_a_luz[3]%18?1:0));
   puntuaciones[3]=(texto4[17]-'0')*1000+(texto4[18]-'0')*100+(texto4[19]-'0')*10+(texto4[20]-'0');

   fm_escribir_centrado(texto0,68);
   fm_escribir_centrado(texto1,118);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado(texto2,198);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado(texto3,278);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado(texto4,358);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
  }
 else
  {char texto0[]="C�maras criog�nicas";
   char texto1[]="activadas.............00";
   char texto2[]="Crionautas";
   char texto3[]="salvados..000";
   char texto4[]="Habitaciones";
   char texto5[]="exploradas..000";
   char texto6[]="A�os luz........0000";

   texto1[22]+=dt_partida.cerraduras/10;
   texto1[23]+=dt_partida.cerraduras%10;
   texto3[10]+=dt_partida.crionautas/100;
   texto3[11]+=(dt_partida.crionautas%100)/10;
   texto3[12]+=dt_partida.crionautas%10;
   texto5[12]+=puntuaciones[2]/100;
   texto5[13]+=(puntuaciones[2]%100)/10;
   texto5[14]+=puntuaciones[2]%10;
   texto6[16]+=(pos_a_luz[0]>162?0:pos_a_luz[0]/18+(pos_a_luz[0]%18?1:0));
   texto6[17]+=(pos_a_luz[1]>162?0:pos_a_luz[1]/18+(pos_a_luz[1]%18?1:0));
   texto6[18]+=(pos_a_luz[2]>162?0:pos_a_luz[2]/18+(pos_a_luz[2]%18?1:0));
   texto6[19]+=(pos_a_luz[3]>162?0:pos_a_luz[3]/18+(pos_a_luz[3]%18?1:0));
   puntuaciones[3]=(texto6[16]-'0')*1000+(texto6[17]-'0')*100+(texto6[18]-'0')*10+(texto6[19]-'0');

   fm_escribir_centrado(texto0,28);
   fm_escribir_centrado(texto1,78);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado(texto2,158);
   fm_escribir_centrado(texto3,208);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado(texto4,288);
   fm_escribir_centrado(texto5,338);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}

   fm_escribir_centrado(texto6,418);
   reproducir_sonido(23,PLAYMODE_PLAY);
   for(f=0;f<10;f++) {reproducir_musica(0);fm_volcar();}
  }
 destroy_bitmap(bmptxt);
 do
  {
   fm_volcar();
   reproducir_musica(0);
   tcj_comprobar();
  }
 while(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2] &&
       !tcj_estado[TCJ_AC3] && !tcj_estado[TCJ_AC4]);
}
#undef fm_escribir_centrado(_T_,_Y_)

//******************************************************************************
// Funci�n fin_juego()
//    Libera la memoria utilizada.
//******************************************************************************
void fin_juego(void)
{
 ism_destruir_todo();
 destroy_bitmap(mapa_al);
 destroy_bitmap(buffer);
 unload_datafile(f_obj);
 unload_datafile(f_esc);
 unload_datafile(f_anim);
 unload_datafile(f_robots);
 unload_datafile(f_fuentes);
 if(mapa_vidas) destroy_bitmap(mapa_vidas);
 if(mapa_cerraduras) destroy_bitmap(mapa_cerraduras);
}

//******************************************************************************
// Funci�n pausa()
//    Pausa el juego.
//******************************************************************************
char pausa(void)
{
 char opciones[4][17]={"Volver al juego","Salir del juego","Back to game","Exit game"};
 char opc=0, contador=0, pulsado=1, elegida=2;
 BITMAP *buffer2=create_bitmap(buffer->w,buffer->h);
 BITMAP *m_trans=create_bitmap(idioma?365:403,57);

 pausar_sonidos();

 set_trans_blender(0,0,0,100);
 opciones[0][16]=opciones[1][16]=opciones[2][13]=opciones[3][10]=0;

 blit(buffer,buffer2,0,0,0,0,RES_X,RES_Y);
 textout_ex(buffer2, (FONT *)(f_fuentes[1].dat), opciones[2*idioma], 118+20*idioma, 150, -1, -1);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(contador=0;contador<10;contador++)
  {
   vsync();
   acquire_screen();
   blit(buffer2,screen,0,0,0,0,640,480);
   release_screen();
   reproducir_musica(1+(dt_partida.cerraduras&1));
  }

 textout_ex(buffer2, (FONT *)(f_fuentes[1].dat), opciones[1+2*idioma], 118+20*idioma, 250, -1, -1);
 reproducir_sonido(23,PLAYMODE_PLAY);
 for(contador=0;contador<10;contador++)
  {
   vsync();
   acquire_screen();
   blit(buffer2,screen,0,0,0,0,640,480);
   release_screen();
   reproducir_musica(1+(dt_partida.cerraduras&1));
  }
 reproducir_sonido(23,PLAYMODE_PLAY);

 contador=0;
 while(elegida==2)
  {
   reproducir_musica(1+(dt_partida.cerraduras&1));
   if(contador==0 || contador==30)
    {
     blit(buffer,buffer2,0,0,0,0,RES_X,RES_Y);
     clear_to_color(m_trans,(hicolor?MASK_COLOR_16:MASK_COLOR_32));
     if(idioma)
      {
       opciones[2+opc][opc?9:12]=(contador<30?'_':0);
       opciones[2+(opc?0:1)][opc?12:9]=0;
       textout_ex(m_trans, (FONT *)(f_fuentes[0].dat), opciones[2+opc], 0, 0, -1, -1);
       draw_trans_sprite(buffer2,m_trans,133,opc?255:155);
       textout_ex(buffer2, (FONT *)(f_fuentes[opc].dat), opciones[2], 138, 150, -1, -1);
       textout_ex(buffer2, (FONT *)(f_fuentes[opc?0:1].dat), opciones[3], 138, 250, -1, -1);
      }
     else
      {
       opciones[opc][15]=(contador<30?'_':0);
       opciones[opc?0:1][15]=0;
       textout_ex(m_trans, (FONT *)(f_fuentes[0].dat), opciones[opc], 0, 0, -1, -1);
       draw_trans_sprite(buffer2,m_trans,113,opc?255:155);
       textout_ex(buffer2, (FONT *)(f_fuentes[opc].dat), opciones[0], 118, 150, -1, -1);
       textout_ex(buffer2, (FONT *)(f_fuentes[opc?0:1].dat), opciones[1], 118, 250, -1, -1);
      }
    }
   vsync();
   acquire_screen();
   blit(buffer2,screen,0,0,0,0,640,480);
   release_screen();
   contador=(contador<59?contador+1:0);
   tcj_comprobar();

   if(pulsado)
    {
     if(!tcj_estado[TCJ_ARR] && !tcj_estado[TCJ_ABJ] && !tcj_estado[TCJ_IZQ] &&
        !tcj_estado[TCJ_DER] && !tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC1] &&
        !tcj_estado[TCJ_AC2] && !tcj_estado[TCJ_AC4])
       pulsado=0;
    }
   else
    {
     if(tcj_estado[TCJ_ARR] || tcj_estado[TCJ_ABJ] || tcj_estado[TCJ_IZQ] || tcj_estado[TCJ_DER])
      {
       reproducir_sonido(23,PLAYMODE_PLAY);
       opc=(opc?0:1);
       contador=0;
       pulsado=1;
      }
     else if(tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC1] || tcj_estado[TCJ_AC2] || tcj_estado[TCJ_AC4])
      {
       if(!opc) reproducir_sonido(23,PLAYMODE_PLAY);
       elegida=opc;
      }

    }
  }

 destroy_bitmap(buffer2);
 destroy_bitmap(m_trans);
 reanudar_sonidos();
 embolsando=1;
 puede_saltar=0;
 return elegida;
}

//******************************************************************************
// Funci�n cargar_hab()
//    Carga en Isomot la habitaci�n indicada por la variable h.
//******************************************************************************
void cargar_hab(void)
 {
  int x,y;
  unsigned char rx,ry;
  unsigned short f;
  char n, i, num_astronautas=0, ind_cerr;
  BITMAP *mp_loseta=(BITMAP *)(f_esc[0].dat);
  char tipo_blq[3]={0,1,2};

  detener_sonidos(0);
  canal_llave=-1;
  sonido_autom=3;
  for(f=0;f<5;dt_telebot.id[f++]=NO_ID);
  for(f=0;f<2;dt_telebot.id_t[f++]=NO_ID);
  for(f=0;f<5;dt_telebot.tc[f++]=0);
  for(f=0;f<5;dt_telebot.ta[f++]=0);
  for(f=0;f<8;id_jamba[f++]=NO_ID);

// Se definen las dimensiones de la habitaci�n, y se colocan las paredes y el suelo.
  if(hab[h].banderas&1) // Habitaci�n cuadrada
   {unsigned char puertas=hab[h].x_p & 0x0f;

    if(hab[h].tipo==0) // Hex�gonos amarillos.
     {tipo_blq[0]=2;tipo_blq[1]=1;tipo_blq[2]=4;}
    else if(hab[h].tipo==1) // Hex�gonos azules.
     {tipo_blq[0]=2;tipo_blq[1]=1;tipo_blq[2]=4;}
    else if(hab[h].tipo==2) // Observaci�n.
     {tipo_blq[0]=6;tipo_blq[1]=4;tipo_blq[2]=7;}
    else // Motores.
     {tipo_blq[0]=3;tipo_blq[1]=1;tipo_blq[2]=5;}

    ism_establecer_rejilla(10,10,ANCHO_CELDA);
    dx=dy=1;
    x_org=X_ORG;
    y_org=Y_ORG;
    oscuridad=0;

    ism_colocar_suelo(fm_suelo(hab[h].tipo));
    puerta[0]=puerta[1]=puerta[2]=puerta[3]=0;
    ism_colocar_objeto_rejilla(1,0,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(2,0,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(3,0,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(6,0,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(7,0,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(8,0,0,1000,NULL,NULL);
    if(puertas&0x01)
     {
      ism_colocar_loseta(mp_loseta,4,0);
      ism_colocar_loseta(mp_loseta,5,0);
      puerta[0]=1;
     }
    else
     {
      ism_colocar_objeto_rejilla(4,0,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(5,0,0,1000,NULL,NULL);
     }

    ism_colocar_pared(fm_pared_sx(hab[h].tipo,puerta[0]),SUP_X,0);
    switch(hab[h].tipo)
     {
      case 0:
      case 1:
        ism_mover_pared(SUP_X,0,0,-165); break;
      case 3:
        ism_mover_pared(SUP_X,0,0,-168); break;
      default:
        ism_mover_pared(SUP_X,0,0,-162);
     }

    if(nuevo_marcador)
     {
      ism_colocar_pared(mapa_vidas,SUP_X,1);
      ism_colocar_pared(fm_pared_msx(1),SUP_X,2);
      ism_colocar_pared(fm_pared_msx(2),SUP_X,3);
      ism_colocar_pared(mapa_cerraduras,SUP_Y,1);
      ism_colocar_pared(fm_pared_msy(1),SUP_Y,2);
      ism_colocar_pared(fm_pared_msy(2),SUP_Y,3);
      ism_mover_pared(SUP_X,1,94,190);
      ism_mover_pared(SUP_X,2,-10,182);
      ism_mover_pared(SUP_X,3,97,70);
      ism_mover_pared(SUP_Y,1,124,220);
      ism_mover_pared(SUP_Y,2,58,250);
      ism_mover_pared(SUP_Y,3,88,61);
     }
    else
     {
      ism_colocar_pared(fm_nacho_giro(0,1),SUP_X,1);
      ism_mover_pared(SUP_X,1,67,163);
      ism_colocar_pared(mapa_vidas,SUP_X,2);
      ism_mover_pared(SUP_X,2,82,207);
      ism_colocar_pared(fm_cerr(),SUP_Y,1);
      ism_mover_pared(SUP_Y,1,112,216);
      ism_colocar_pared(mapa_cerraduras,SUP_Y,2);
      ism_mover_pared(SUP_Y,2,55,180);
     }

    ism_colocar_objeto_rejilla(9,1,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(9,2,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(9,3,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(9,6,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(9,7,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(9,8,0,1000,NULL,NULL);
    if(puertas&0x02)
     {
      ism_colocar_loseta(mp_loseta,9,4);
      ism_colocar_loseta(mp_loseta,9,5);
      puerta[1]=1;
     }
    else
     {
      ism_colocar_objeto_rejilla(9,4,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(9,5,0,1000,NULL,NULL);
     }
    ism_colocar_objeto_rejilla(1,9,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(2,9,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(3,9,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(6,9,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(7,9,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(8,9,0,1000,NULL,NULL);
    if(puertas&0x04)
     {
      ism_colocar_loseta(mp_loseta,4,9);
      ism_colocar_loseta(mp_loseta,5,9);
      puerta[2]=1;
     }
    else
     {
      ism_colocar_objeto_rejilla(4,9,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(5,9,0,1000,NULL,NULL);
     }
    ism_colocar_objeto_rejilla(0,1,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(0,2,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(0,3,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(0,6,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(0,7,0,1000,NULL,NULL);
    ism_colocar_objeto_rejilla(0,8,0,1000,NULL,NULL);
    if(puertas&0x08)
     {
      ism_colocar_loseta(mp_loseta,0,4);
      ism_colocar_loseta(mp_loseta,0,5);
      puerta[3]=1;
     }
    else
     {
      ism_colocar_objeto_rejilla(0,4,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(0,5,0,1000,NULL,NULL);
     }

    ism_colocar_pared(fm_pared_sy(hab[h].tipo,puerta[3]),SUP_Y,0);
    switch(hab[h].tipo)
     {
      case 0:
      case 1:
        ism_mover_pared(SUP_Y,0,129,-36); break;
      case 3:
        ism_mover_pared(SUP_Y,0,132,-36); break;
      default:
        ism_mover_pared(SUP_Y,0,126,-36);
     }

    if(nuevo_marcador)
     {
      ism_colocar_pared(fm_pared_ix(),INF_X,0);
      ism_mover_pared(INF_X,0,20,24);
      ism_colocar_pared(fm_pared_iy(),INF_Y,0);
      ism_mover_pared(INF_Y,0,144,148);
     }

   }
  else // Pasillo
   {
    if((hab[h].x_p & 0x0f)==5) // Pasillo en y
     {
      ism_establecer_rejilla(4,10,ANCHO_CELDA);
      oscuridad=0;
      dx=0;dy=1;
      x_org=X_ORG+ANCHO_CELDA*3*2;
      y_org=Y_ORG+ANCHO_CELDA*3;

      ism_colocar_suelo(fm_escenario_pasillo(0));
      ism_mover_suelo(9,9);
      ism_colocar_pared(fm_escenario_pasillo(1),SUP_Y,0);
      ism_mover_pared(SUP_Y,0,145,0);
      ism_colocar_pared(fm_escenario_pasillo(2),SUP_X,0);
      ism_mover_pared(SUP_X,0,-18,-90);

      if(nuevo_marcador)
       {
        ism_colocar_pared(mapa_vidas,SUP_X,1);
        ism_colocar_pared(fm_pared_msx(1),SUP_X,2);
        ism_colocar_pared(fm_pared_msx(2),SUP_X,3);
        ism_colocar_pared(mapa_cerraduras,SUP_Y,1);
        ism_colocar_pared(fm_pared_msy(1),SUP_Y,2);
        ism_colocar_pared(fm_pared_msy(2),SUP_Y,3);
        ism_mover_pared(SUP_X,1,94-54,190);
        ism_mover_pared(SUP_X,2,-10-54,182);
        ism_mover_pared(SUP_X,3,97-54,70);
        ism_mover_pared(SUP_Y,1,124+54,220+108);
        ism_mover_pared(SUP_Y,2,58+54,250+108);
        ism_mover_pared(SUP_Y,3,88+54,61+108);
       }
      else
       {
        ism_colocar_pared(fm_nacho_giro(0,1),SUP_X,1);
        ism_mover_pared(SUP_X,1,13,163);
        ism_colocar_pared(mapa_vidas,SUP_X,2);
        ism_mover_pared(SUP_X,2,28,163+44);
        ism_colocar_pared(fm_cerr(),SUP_Y,1);
        ism_mover_pared(SUP_Y,1,166,324);
        ism_colocar_pared(mapa_cerraduras,SUP_Y,2);
        ism_mover_pared(SUP_Y,2,166-57,324-36);
       }

      ism_colocar_loseta(mp_loseta,1,0);
      ism_colocar_loseta(mp_loseta,2,0);
      ism_colocar_loseta(mp_loseta,1,9);
      ism_colocar_loseta(mp_loseta,2,9);
      ism_colocar_objeto_rejilla(0,0,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(3,0,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(0,9,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(3,9,0,1000,NULL,NULL);

      puerta[0]=puerta[2]=1;
      puerta[1]=puerta[3]=0;

      if(nuevo_marcador)
       {
        ism_colocar_pared(fm_pared_ix(),INF_X,0);
        ism_mover_pared(INF_X,0,-34,24);
        ism_colocar_pared(fm_pared_iy(),INF_Y,0);
        ism_mover_pared(INF_Y,0,90,40);
       }
     }
    else // Pasillo en x
     {
      ism_establecer_rejilla(10,4,ANCHO_CELDA);
      oscuridad=0;
      dx=1;dy=0;
      x_org=X_ORG-ANCHO_CELDA*3*2;
      y_org=Y_ORG+ANCHO_CELDA*3;

      ism_colocar_suelo(fm_escenario_pasillo(3));
      ism_mover_suelo(9,9);
      ism_colocar_pared(fm_escenario_pasillo(4),SUP_Y,0);
      ism_mover_pared(SUP_Y,0,41,-31);
      ism_colocar_pared(fm_escenario_pasillo(5),SUP_X,0);
      ism_mover_pared(SUP_X,0,18,-127);

      if(nuevo_marcador)
       {
        ism_colocar_pared(mapa_vidas,SUP_X,1);
        ism_colocar_pared(fm_pared_msx(1),SUP_X,2);
        ism_colocar_pared(fm_pared_msx(2),SUP_X,3);
        ism_colocar_pared(mapa_cerraduras,SUP_Y,1);
        ism_colocar_pared(fm_pared_msy(1),SUP_Y,2);
        ism_colocar_pared(fm_pared_msy(2),SUP_Y,3);
        ism_mover_pared(SUP_X,1,94+54,190+108);
        ism_mover_pared(SUP_X,2,-10+54,182+108);
        ism_mover_pared(SUP_X,3,97+54,70+108);
        ism_mover_pared(SUP_Y,1,124-54,220);
        ism_mover_pared(SUP_Y,2,58-54,250);
        ism_mover_pared(SUP_Y,3,88-54,61);
       }
      else
       {
        ism_colocar_pared(fm_nacho_giro(0,1),SUP_X,1);
        ism_mover_pared(SUP_X,1,121,271);
        ism_colocar_pared(mapa_vidas,SUP_X,2);
        ism_mover_pared(SUP_X,2,121+15,271+44);
        ism_colocar_pared(fm_cerr(),SUP_Y,1);
        ism_mover_pared(SUP_Y,1,58,216);
        ism_colocar_pared(mapa_cerraduras,SUP_Y,2);
        ism_mover_pared(SUP_Y,2,58-57,216-36);
       }

      ism_colocar_loseta(mp_loseta,0,1);
      ism_colocar_loseta(mp_loseta,0,2);
      ism_colocar_loseta(mp_loseta,9,1);
      ism_colocar_loseta(mp_loseta,9,2);
      ism_colocar_objeto_rejilla(0,0,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(0,3,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(9,0,0,1000,NULL,NULL);
      ism_colocar_objeto_rejilla(9,3,0,1000,NULL,NULL);

      puerta[0]=puerta[2]=0;
      puerta[1]=puerta[3]=1;

      if(nuevo_marcador)
       {
        ism_colocar_pared(fm_pared_ix(),INF_X,0);
        ism_mover_pared(INF_X,0,-34,-84);
        ism_colocar_pared(fm_pared_iy(),INF_Y,0);
        ism_mover_pared(INF_Y,0,90,148);
       }
     }
   }

// Se colocan los objetos.
  primera_cascara=primera_mina_voladora=-1;
  num_obst=0;
  num_movs=6;
  rx=dx; ry=dy;
  for(f=(h?hab[h-1].n:0);f<hab[h].n;f++)
   {int z=ENCIMA;
    if(mapa_obj[f]>=COL00) // Cambio x/y
     {
      rx = (mapa_obj[f] & 0x07) + dx;
      ry = ((mapa_obj[f] >> 3) & 0x07) + dy;
      f++;
     }

    if(mapa_obj[f]>=ELEV0) // Cambio z
     {
      z=(mapa_obj[f] & 0x3f)*ALTURA_BASE;
      f++;
     }

    if(mapa_obj[f] <= NOR02) // Cubo est�tico normal.
     {
      ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_norm(mapa_obj[f]),(z==ENCIMA?NULL:fm_sombra(0)));
     }
    else if(mapa_obj[f] <= NOR12) // Cubo que desaparece al pisarlo.
     {
      ism_obtener_coords_libres(rx,ry,&x,&y);
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=ism_colocar_objeto_libre(x,y,z,ANCHO_CELDA,ANCHO_CELDA,ALTURA_BASE,fm_norm(mapa_obj[f]),(z==ENCIMA?NULL:fm_sombra(0)));
      movs[num_movs].m0=1;
      movs[num_movs].m1=0;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      movs[num_movs].mover=mov_desaparece;
      num_movs++;
     }
    else if(mapa_obj[f] <= NOR52) // Cubo que mueve los objetos que le caen encima.
     {char i;
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_norm(mapa_obj[f]),z==ENCIMA?NULL:fm_sombra(0));
      movs[num_movs].dir=(mapa_obj[f]<=NOR22 || (mapa_obj[f]>=NOR40 && mapa_obj[f]<=NOR42)?1:-1);
      movs[num_movs].sentmov=(mapa_obj[f]<=NOR32?D_X:D_Y);
      movs[num_movs].mover=mov_transportar;
      movs[num_movs].empujable=0;
      movs[num_movs].m1=0;
      movs[num_movs].g=-1;
      movs[num_movs].anclado=NO_ID;
      for(i=6;i<=num_movs;i++)
        if(movs[i].mover==mov_transportar)
          movs[i].m0=num_movs;
      num_movs++;
     }
    else if(mapa_obj[f] <= NOR62) // Cubo que baja al pisarlo
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_norm(mapa_obj[f]),fm_sombra(0));
      movs[num_movs].m0=1;
      movs[num_movs].m1=-1;
      movs[num_movs].mover=mov_z_pisado;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] <= NOR72) // Cubo que se mueve en x o y
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      movs[num_movs].id=ism_colocar_objeto_libre(x,y,z,ANCHO_CELDA,ANCHO_CELDA,ALTURA_BASE,fm_norm(mapa_obj[f++]),fm_sombra(0));
      if(rx-dx==(mapa_obj[f]>>4))
       {
        movs[num_movs].m0=y;
        ism_obtener_coords_libres(rx,(mapa_obj[f]&0x0f)+dy,NULL,&y);
        movs[num_movs].m1=y;
        movs[num_movs].sentmov=D_Y;
       }
      else
       {
        movs[num_movs].m0=x;
        ism_obtener_coords_libres((mapa_obj[f]>>4)+dx,ry,&x,NULL);
        movs[num_movs].m1=x;
        movs[num_movs].sentmov=D_X;
       }
      movs[num_movs].dir=(movs[num_movs].m0>movs[num_movs].m1?1:-1);
      movs[num_movs].mover=mov_xy_autom;
      if(movs[num_movs].m0>movs[num_movs].m1)
       {
        int t=movs[num_movs].m0;
        movs[num_movs].m0=movs[num_movs].m1;
        movs[num_movs].m1=t;
       }
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      movs[num_movs].g=0;
      num_movs++;
     }
    else if(mapa_obj[f] <= NOR82) // Cubo que se mueve en z
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_norm(mapa_obj[f++]),fm_sombra(0));
      movs[num_movs].m0=0;
      movs[num_movs].g=1;
      movs[num_movs].m1=mapa_obj[f]*ALTURA_BASE;
      movs[num_movs].dir=-1;
      movs[num_movs].mover=mov_z_autom;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] <= NOR92) // Cubo que se mueve en z al pisarlo
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_norm(mapa_obj[f]),fm_sombra(0));
      movs[num_movs].m1=movs[num_movs].dir=-1;
      movs[num_movs].mover=mov_zmn_pisado;
      movs[num_movs].m0=mapa_obj[++f]*ALTURA_BASE;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] <= PTYI2) // Suelo bajo puerta elevada
     {char p[16]={0,1,0,1, 4,4,5,5, 9,8,9,8, 5,5,4,4};
      char *px;
      char *py;
      char j=(mapa_obj[f]-PTYS0)/3;

      if(!dx || !dy)
       {
        p[4]=p[5]=p[14]=p[15]=1;
        p[6]=p[7]=p[12]=p[13]=2;
       }
      px=p+j*4;
      py=(j<3?px+4:p);

      ism_colocar_objeto_rejilla(px[0],py[0],0,3*ALTURA_BASE,NULL,NULL);
      ism_colocar_objeto_rejilla(px[2],py[2],0,3*ALTURA_BASE,NULL,NULL);
      ism_colocar_loseta(NULL,px[0],py[0]);
      ism_colocar_loseta(NULL,px[2],py[2]);
      for(j=0;j<4;j++)
        ism_colocar_objeto_rejilla(px[j],py[j],3*ALTURA_BASE,ALTURA_BASE,fm_norm(mapa_obj[f]),fm_sombra(0));
     }
    else if(mapa_obj[f] <= OBS04) // Obst�culo
     {
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      id_obst[num_obst]=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_obst(mapa_obj[f]),(z==ENCIMA?NULL:fm_sombra((mapa_obj[f]==OBS04?6:0))));
      num_obst++;
     }
    else if(mapa_obj[f] <= OBS05) // Mina voladora
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      movs[num_movs].id=id_obst[num_obst]=ism_colocar_objeto_rejilla(rx,ry,z,12,fm_obst(mapa_obj[f]),fm_sombra(5));
      if(primera_mina_voladora==-1)
       {
        primera_mina_voladora=num_movs;
        movs[num_movs].m0=1;
        movs[num_movs].m1=1;
       }
      else
       {
        movs[num_movs].m0=0;
        movs[primera_mina_voladora].m1++;
       }
      movs[num_movs].g=100;
      movs[num_movs].mover=mov_mina_voladora;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
     }
    else if(mapa_obj[f] <= OBS06) // C�scara
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      movs[num_movs].id=id_obst[num_obst]=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_obst(mapa_obj[f]),fm_sombra(6));
      movs[num_movs].g=100;
      movs[num_movs].m0=0;
      if(primera_cascara<0)
        movs[primera_cascara=num_movs].m1=1;
      else
        movs[primera_cascara].m1++;
      movs[num_movs].mover=mov_cascara;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
     }
    else if(mapa_obj[f] <= CERR3) // Cerradura
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=NO_ID;
      movs[num_movs].anclado=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_cerr(),NULL);
      movs[num_movs].m0=rx*18+3;
      movs[num_movs].m1=(ry+1)*18-1-3;
      movs[num_movs].g=ism_obtener_dato_objeto(movs[num_movs].anclado,D_Z)+ALTURA_BASE;
      movs[num_movs].dir=mapa_obj[f]-CERR0;
      movs[num_movs].empujable=0;
      movs[num_movs].sentmov=0;
      movs[ind_cerr=num_movs].mover=mov_cerradura;
      if(!(hab[h].banderas & 0x08))
       {
        ism_establecer_oscuridad(50);
        oscuridad=5;
       }
      sonido_interferencia=-1;
      num_movs++;
     }
    else if(mapa_obj[f] <= ASTR1) // Astronauta
     {
      if(hab[h].banderas & 0x08)
       {
        ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE*2,fm_astr(mapa_obj[f]-ASTR0,2),NULL);
       }
      else
       {
        if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
        movs[num_movs].id=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE*2,fm_astr(mapa_obj[f]-ASTR0,0),NULL);
        movs[num_movs].m0=1;
        movs[num_movs].m1=num_astronautas*5;
        movs[num_movs].sentmov=mapa_obj[f]-ASTR0;
        movs[num_movs].dir=0;
        movs[num_movs].mover=mov_astronauta;
        num_astronautas++;
        movs[num_movs].empujable=0;
        movs[num_movs].anclado=NO_ID;
        num_movs++;
       }
     }
    else if(mapa_obj[f] <= MOV11) // Bloque empujable
     {
      ism_obtener_coords_libres(rx,ry,&x,&y);
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].id=ism_colocar_objeto_libre(x,y,z,ANCHO_CELDA,ANCHO_CELDA,ALTURA_BASE,fm_mov(mapa_obj[f]),fm_sombra(0));
      movs[num_movs].g=0;
      movs[num_movs].m0=(mapa_obj[f]<=MOV10?1:0);
      movs[num_movs].mover=mov_empujable;
      movs[num_movs].empujable=1;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] == JOYST) // Joystick controlador del telebot
     {
      dt_telebot.id[0]=ism_colocar_objeto_rejilla(rx+1,ry,z,ALTURA_BASE,fm_joy(0,0),NULL);
      dt_telebot.id[1]=ism_colocar_objeto_rejilla(rx,ry+1,z,ALTURA_BASE,fm_joy(1,0),NULL);
      dt_telebot.id[2]=ism_colocar_objeto_rejilla(rx-1,ry,z,ALTURA_BASE,fm_joy(2,0),NULL);
      dt_telebot.id[3]=ism_colocar_objeto_rejilla(rx,ry-1,z,ALTURA_BASE,fm_joy(3,0),NULL);
      dt_telebot.id[4]=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_joy(-1,0),NULL);
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      movs[num_movs].mover=mov_joystick;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] == TLBOT) // Telebot
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      movs[num_movs].id=ism_colocar_objeto_libre(x+2,y-2,z,14,14,ALTURA_BASE,fm_telebot(0),NULL);
      if(ism_obtener_dato_objeto(movs[num_movs].id,D_Z))
        ism_cambiar_sombra_objeto(movs[num_movs].id,fm_sombra(1));
      movs[num_movs].g=movs[num_movs].m0=movs[num_movs].m1=movs[num_movs].sentmov=0;
      movs[num_movs].mover=mov_telebot;
      if(dt_telebot.id_t[0]==NO_ID) dt_telebot.id_t[0]=movs[num_movs].id;
      dt_telebot.id_t[1]=movs[num_movs].id;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] == RBALA) // Robot alien�gena de movimiento libre
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      id_obst[num_obst]=movs[num_movs].id=ism_colocar_objeto_libre(x+2,y-2,z,14,14,ALTURA_BASE*2,NULL,NULL);
      if(ism_obtener_dato_objeto(movs[num_movs].id,D_Z))
       {
        movs[num_movs].g=0;
        ism_cambiar_sombra_objeto(movs[num_movs].id,fm_sombra(2));
       }
      else
        movs[num_movs].g=1;
      movs[num_movs].m0=movs[num_movs].m1=movs[num_movs].dir=0;
      movs[num_movs].sentmov=rand()&3;
      movs[num_movs].mover=mov_rbal_l;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
      reproducir_sonido(2, PLAYMODE_LOOP);
     }
    else if(mapa_obj[f] <= RBALY) // Robot alien�gena de movimiento fijo en x o y
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      id_obst[num_obst]=movs[num_movs].id=ism_colocar_objeto_libre(x+2,y-2,z,14,14,ALTURA_BASE*2,NULL,NULL);
      if(ism_obtener_dato_objeto(movs[num_movs].id,D_Z))
        ism_cambiar_sombra_objeto(movs[num_movs].id,fm_sombra(2));
      movs[num_movs].sentmov=(mapa_obj[f]==RBALX?2:3);
      movs[num_movs].g=movs[num_movs].m0=movs[num_movs].m1=movs[num_movs].dir=0;
      movs[num_movs].mover=mov_rbal_f;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
      reproducir_sonido(2, PLAYMODE_LOOP);
     }
    else if(mapa_obj[f] == MINAT) // Mina terrestre
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      movs[num_movs].id=id_obst[num_obst]=ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_minat(0),NULL);
      movs[num_movs].m0=movs[num_movs].m1=movs[num_movs].dir=0;
      movs[num_movs].mover=mov_mina_terrestre;
      movs[num_movs].empujable=1;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
     }
    else if(mapa_obj[f] == SATUR) // Saturno
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      id_obst[num_obst]=movs[num_movs].id=ism_colocar_objeto_libre(x,y,z,ANCHO_CELDA,ANCHO_CELDA,ALTURA_BASE*2,fm_saturno(0),fm_sombra(3));
      movs[num_movs].mover=mov_saturno;
      movs[num_movs].m0=31;
      movs[num_movs].g=movs[num_movs].m1=0;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
     }
    else if(mapa_obj[f] == RATAM) // Rata mec�nica
     {
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      if(num_obst>=MAX_OBST) num_obst=MAX_OBST-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      if(rand()&1)
        movs[num_movs].sentmov=D_X;
      else
        movs[num_movs].sentmov=D_Y;
      movs[num_movs].dir=(rand()&1?1:-1);
      movs[num_movs].m0=(rand()%4)*3;
      movs[num_movs].m1=rand()%2;
      id_obst[num_obst]=movs[num_movs].id=ism_colocar_objeto_libre(x+1,y-1,z,16,16,20,NULL,NULL);
      if(ism_obtener_dato_objeto(movs[num_movs].id,D_Z)>0)
       {
        ism_cambiar_sombra_objeto(movs[num_movs].id,fm_sombra(7));
        movs[num_movs].g=0;
       }
      else
        movs[num_movs].g=1;
      movs[num_movs].mover=mov_ratamec;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
      num_obst++;
     }
    else if(mapa_obj[f] <= PLAS3) // Plasma
     {
      unsigned char pdsp=mapa_obj[f]-PLAS0;
      if(num_movs>=MAX_MOVS) num_movs=MAX_MOVS-1;
      ism_obtener_coords_libres(rx,ry,&x,&y);
      if(pdsp&1) x+=ANCHO_CELDA/2;
      if(pdsp&2) y+=ANCHO_CELDA/2;
      movs[num_movs].id=ism_colocar_objeto_libre(x,y,z,15,15,ALTURA_BASE,fm_plasma(0),fm_sombra(4));
      ism_cambiar_dato_objeto(movs[num_movs].id,D_TRANSP,40,CAMBIAR);
      movs[num_movs].m0=movs[num_movs].m1=movs[num_movs].g=0;
      movs[num_movs].sentmov=1;
      movs[num_movs].dir=-1;
      movs[num_movs].mover=mov_plasma;
      movs[num_movs].empujable=0;
      movs[num_movs].anclado=NO_ID;
      num_movs++;
     }
    else if(mapa_obj[f] <= ROBS4) // Robot obsoleto.
     {
      ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,fm_robot_obsoleto(mapa_obj[f]),(mapa_obj[f]<ROBS3?fm_sombra(4):NULL));
     }
    else if(mapa_obj[f] == NOR03) // Cubo oxidado.
     {
      ism_colocar_objeto_rejilla(rx,ry,z,ALTURA_BASE,(BITMAP *)(f_obj[4].dat),NULL);
     }
    else if(mapa_obj[f] == PTCRR) // Puerta cerrada.
     {
      ism_colocar_pared((BITMAP *)(f_esc[29].dat),SUP_X,0);
      ism_mover_pared(SUP_X,0,0,-162);
      ism_obtener_coords_libres(4,0,&x,&y);
      ism_colocar_objeto_libre(x,y+7,0,7,7,95,fm_puertacrr(0),NULL);
      ism_colocar_objeto_libre(x+30,y+7,0,7,7,95,fm_puertacrr(1),NULL);
      ism_colocar_objeto_libre(x,y+7,ENCIMA,37,7,16,fm_puertacrr(2),NULL);
     }
   }

  // Se colocan las llaves.
  for(i=-1,f=1,n=(hab[h].banderas & 0x07);f<3;f++,n-=2)
   {
    if(n>=2)
     {
      do{i++;}while(llave[i].hab!=h);
      movs[f].id=ism_colocar_objeto_libre(llave[i].x,llave[i].y,llave[i].z,12,12,
                     ALTURA_BASE,fm_llave(llave[i].tipo,0),fm_sombra_llave(llave[i].tipo));

      if(movs[f].id==ID_ERROR)
       {
        llave[i].hab=-1;
        hab[h].banderas-=2;
        movs[f].id=NO_ID;
        movs[f].m0=movs[f].m1=-1;
        reproducir_sonido(0,PLAYMODE_PLAY);
       }
      else
        movs[f].m0=movs[f].m1=i;
      if(llave[movs[f].m0].tipo==4)
       {// Es una vida extra.
        movs[f].mover=mov_vida;
        movs[f].m1=movs[f].sentmov=0;
        movs[f].dir=0;
       }
      else
        movs[f].mover=mov_llave;
      movs[f].g=0;
      movs[f].empujable=1;
      if(hab[h].banderas&0x08)
        if(llave[i].x==movs[ind_cerr].m0 && llave[i].y==movs[ind_cerr].m1 &&
           llave[i].z==movs[ind_cerr].g)
          {
           movs[f].empujable=0;
           ism_cambiar_sombra_objeto(movs[f].id,NULL);
          }
      movs[f].anclado=NO_ID;
     }
    else
     {
      movs[f].id=NO_ID;
      movs[f].mover=mov_llave;
      movs[f].g=0;
      movs[f].m0=movs[f].m1=-1;
      movs[f].empujable=1;
      movs[f].anclado=NO_ID;
     }
   }

  // Se coloca a Nacho.
  x=movs[0].m0-(dx?0:ANCHO_CELDA*3);
  y=movs[0].m1-(dy?0:ANCHO_CELDA*3);
  movs[0].id=ism_colocar_objeto_libre(x,y,ENCIMA,15,15,55,fm_nacho((movs[0].dir=dt_anim_nacho.dirini),fot_nacho[dt_anim_nacho.fot=0]),fm_sombra(4));
  movs[0].mover=mov_nacho_andar;
  movs[0].g=0;
  movs[0].empujable=1;
  movs[0].anclado=NO_ID;

  // Se le dice a los bolsillos que se coloquen si contienen algo.
  movs[3].dir=(movs[3].m0>=0?1:0);
  movs[4].dir=(movs[4].m0>=0?1:0);
  movs[5].dir=(movs[5].m0>=0?1:0);

  // Se colocan las puertas
  if(puerta[0])
   {// Puerta superior derecha
    if(dx)
     {
      ism_obtener_coords_libres(6,0,&x,&y);
      id_jamba[5]=ism_colocar_objeto_libre(x-7,y,ENCIMA,7,ANCHO_CELDA,96,fm_puertasd(hab[h].tipo,0),NULL);
      ism_obtener_coords_libres(4,0,&x,NULL);
      id_jamba[4]=ism_colocar_objeto_libre(x,y,ENCIMA,7,ANCHO_CELDA,96,NULL,NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA*2,ANCHO_CELDA,1000,fm_puertasd(hab[h].tipo,1),NULL);
     }
    else
     {
      ism_obtener_coords_libres(3,0,&x,&y);
      id_jamba[5]=ism_colocar_objeto_libre(x-7,y,ENCIMA,7,ANCHO_CELDA,74,fm_puertasd(4,0),NULL);
      ism_obtener_coords_libres(1,0,&x,NULL);
      id_jamba[4]=ism_colocar_objeto_libre(x,y,ENCIMA,7,ANCHO_CELDA,74,NULL,NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA*2,ANCHO_CELDA,1000,fm_puertasd(4,1),NULL);
     }
   }

  if(puerta[1])
   {// Puerta inferior derecha
    if(dy)
     {
      ism_obtener_coords_libres(9,3,&x,&y);
      id_jamba[2]=ism_colocar_objeto_libre(x,y+7,ENCIMA,7,7,96,fm_puertay(hab[h].tipo,0),NULL);
      ism_obtener_coords_libres(9,5,NULL,&y);
      id_jamba[3]=ism_colocar_objeto_libre(x,y,ENCIMA,7,7,96,fm_puertay(hab[h].tipo,1),NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,7,ANCHO_CELDA*2,1000,fm_puertay(hab[h].tipo,2),NULL);
     }
    else
     {
      ism_obtener_coords_libres(9,0,&x,&y);
      id_jamba[2]=ism_colocar_objeto_libre(x,y+7,ENCIMA,8,7,74,fm_puertap(1,0),NULL);
      ism_obtener_coords_libres(9,2,NULL,&y);
      id_jamba[3]=ism_colocar_objeto_libre(x,y,ENCIMA,8,7,74,fm_puertap(1,1),NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,8,ANCHO_CELDA*2,1000,fm_puertap(1,2),NULL);
     }
   }

  if(puerta[2])
   {// Puerta inferior izquierda
    if(dx)
     {
      ism_obtener_coords_libres(6,8,&x,&y);
      id_jamba[7]=ism_colocar_objeto_libre(x-7,y+=7,ENCIMA,7,7,96,fm_puertax(hab[h].tipo,1),NULL);
      ism_obtener_coords_libres(4,8,&x,NULL);
      id_jamba[6]=ism_colocar_objeto_libre(x,y,ENCIMA,7,7,96,fm_puertax(hab[h].tipo,0),NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA*2,7,1000,fm_puertax(hab[h].tipo,2),NULL);
     }
    else
     {
      ism_obtener_coords_libres(3,8,&x,&y);
      id_jamba[7]=ism_colocar_objeto_libre(x-7,y+=8,ENCIMA,7,8,74,fm_puertap(0,1),NULL);
      ism_obtener_coords_libres(1,8,&x,NULL);
      id_jamba[6]=ism_colocar_objeto_libre(x,y,ENCIMA,7,8,74,fm_puertap(0,0),NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA*2,8,1000,fm_puertap(0,2),NULL);
     }
   }

  if(puerta[3])
   {// Puerta superior izquierda
    if(dy)
     {
      ism_obtener_coords_libres(1,3,&x,&y);
      id_jamba[0]=ism_colocar_objeto_libre(x-=ANCHO_CELDA,y+7,ENCIMA,ANCHO_CELDA,7,96,NULL,NULL);
      ism_obtener_coords_libres(1,5,NULL,&y);
      id_jamba[1]=ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA,7,96,fm_puertasi(hab[h].tipo,0),NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA,36,1000,fm_puertasi(hab[h].tipo,1),NULL);
     }
    else
     {
      ism_obtener_coords_libres(1,0,&x,&y);
      id_jamba[0]=ism_colocar_objeto_libre(x-=ANCHO_CELDA,y+7,ENCIMA,ANCHO_CELDA,7,74,NULL,NULL);
      ism_obtener_coords_libres(1,2,NULL,&y);
      id_jamba[1]=ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA,7,74,fm_puertasi(4,0),NULL);
      ism_colocar_objeto_libre(x,y,ENCIMA,ANCHO_CELDA,36,1000,fm_puertasi(4,1),NULL);
     }
   }

  clear_bitmap(buffer);

  // Marcamos la habitaci�n como visitada.
  (hab[h].banderas)|=0x10;
 }

//******************************************************************************
// Funci�n mov_transportar(...)
//    Funci�n de movimiento de los cubos que mueven los objetos que tienen
//    encima.
//    m0: �ndice del �ltimo (se encarga del sonido).
//    m1: en el �ltimo, indica si hay que reproducir el sonido.
//    g: en el �ltimo, canal del sonido.
//******************************************************************************
void mov_transportar(char f)
{
 char n;

 if(movs[0].anclado==movs[f].id)
  {
   movs[movs[f].m0].m1=1;
   // �nicamente muevo a Nacho si no ha colisionado.
   if(movs[0].sentmov<2)
     if(ism_cambiar_dato_objeto(movs[0].id,movs[f].sentmov,movs[f].dir,SUMAR))
      {
       for(n=0;n<num_obst;n++)
         if(ism_colisionado_con(id_obst[n]))
          {
           mov_nacho_explotar(1);
           n=num_obst;
          }
       if(movs[0].mover!=mov_nacho_explotar)
         mov_empujar(f,movs[f].sentmov, movs[f].dir);
      }
  }

 // Muevo el resto de los objetos.
 for(n=1;n<num_movs;n++)
  {
   if(movs[n].anclado==movs[f].id)
    {
     movs[movs[f].m0].m1=1;
     if(ism_cambiar_dato_objeto(movs[n].id,movs[f].sentmov,movs[f].dir,SUMAR))
       mov_empujar(f,movs[f].sentmov, movs[f].dir);
    }
  }

 // Si es el �ltimo, gestiono el sonido
 if(f==movs[f].m0)
  {
   if(movs[f].m1)
    {
     if(movs[f].g<0) movs[f].g=reproducir_sonido(13, PLAYMODE_LOOP);
     movs[f].m1=0;
    }
   else
    {
     if(movs[f].g>=0)
      {
       detener_sonido(movs[f].g);
       movs[f].g=-1;
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_z_pisado(...)
//    Funci�n de movimiento de los cubos que bajan al pisarlos.
//******************************************************************************
void mov_z_pisado(char f)
{
 if(movs[f].m0) // Est� activo.
  {
   if(movs[f].id==movs[0].anclado) // Tiene anclado a Nacho.
    {
     // Bajo el cubo.
     if(ism_cambiar_dato_objeto(movs[f].id,D_Z,-1,SUMAR))
      {
       if(ism_colisionado_con(ID_SUELO))
        { // Si ha llegado al suelo, desactivo el movimiento y quito la sombra.
         movs[f].m0=0;
         ism_cambiar_sombra_objeto(movs[f].id,NULL);
        }
       if(movs[f].m1>=0)
        {
         detener_sonido((char)(movs[f].m1));
         movs[f].m1=-1;
        }
      }
     else
      {// Bajo a Nacho y reproduzco el sonido.
       ism_cambiar_dato_objeto(movs[0].id,D_Z,-1,SUMAR);
       if(movs[f].m1<0) movs[f].m1=reproducir_sonido(13, PLAYMODE_LOOP);
      }
    }
   else
    {
     if(movs[f].m1>=0)
      {
       detener_sonido((char)(movs[f].m1));
       movs[f].m1=-1;
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_z_autom(...)
//    Funci�n de movimiento de los cubos que se mueven en el eje z.
//******************************************************************************
void mov_z_autom(char f)
{
 char n,i=-1;

 // Si est� en el tope m�ximo empieza a bajar.
 if(ism_obtener_dato_objeto(movs[f].id,D_Z)>=movs[f].m1)
  {
   movs[f].g++;
   movs[f].dir=-1;
  }

 // Si est� subiendo, subo antes los objetos anclados.
 if(movs[f].dir==1)
  {
   for(n=0;n<num_movs;n++)
    {
     if(movs[f].id==movs[n].anclado)
      {
       if(!ism_cambiar_dato_objeto(movs[n].id,D_Z,1,SUMAR)) i=n;
       n=num_movs; // Tan solo muevo uno.
      }
    }
  }

 // Realizo el movimiento.
 if(ism_cambiar_dato_objeto(movs[f].id,D_Z,movs[f].dir,SUMAR))
  {
   if(movs[f].dir==-1) movs[f].g++;
   movs[f].dir=1;
   if(i>=0) ism_cambiar_dato_objeto(movs[i].anclado,D_Z,-1,SUMAR);
  }
 else if(movs[f].g)
  {
   if(movs[f].g==1 && sonido_autom==0)
    {
     reproducir_sonido(19,PLAYMODE_PLAY);
     sonido_autom=3;
    }
   movs[f].g=0;
  }

 // Si est� bajando, bajo despu�s los objetos anclados.
 if(movs[f].dir==-1)
  {
   for(n=0;n<num_movs;n++)
    {
     if(movs[f].id==movs[n].anclado)
       ism_cambiar_dato_objeto(movs[n].id,D_Z,-1,SUMAR);
    }
  }
}

//******************************************************************************
// Funci�n mov_zmn_pisado(...)
//    Funci�n de movimiento de los cubos que suben y bajan al pisarlos.
//******************************************************************************
void mov_zmn_pisado(char f)
{
 if(movs[f].id==movs[0].anclado)
  {
   if(movs[f].dir>0)
    {// Est� subiendo.
     if(!ism_cambiar_dato_objeto(movs[0].id,D_Z,1,SUMAR))
      {
       if(ism_cambiar_dato_objeto(movs[f].id,D_Z,1,SUMAR))
         ism_cambiar_dato_objeto(movs[0].id,D_Z,-1,SUMAR);
       else
         movs[f].dir=(ism_obtener_dato_objeto(movs[f].id,D_Z)>=movs[f].m0?-1:1);
      }
    }
   else
    {// Est� bajando.
     if(ism_cambiar_dato_objeto(movs[f].id,D_Z,-1,SUMAR))
       movs[f].dir=1;
     else
       ism_cambiar_dato_objeto(movs[0].id,D_Z,-1,SUMAR);
    }
   if(movs[f].m1<0) movs[f].m1=reproducir_sonido(13, PLAYMODE_LOOP);
  }
 else
  {
   if(movs[f].m1>=0)
    {
     detener_sonido((char)(movs[f].m1));
     movs[f].m1=-1;
    }
  }
}

//******************************************************************************
// Funci�n mov_xy_autom(...)
//    Funci�n de movimiento de los cubos que se mueven en los ejes x o y.
//******************************************************************************
void mov_xy_autom(char f)
{
 char n;
 int dt=ism_obtener_dato_objeto(movs[f].id,movs[f].sentmov);

 // Si he llegado a alguno de los l�mites, lo cambio de sentido.
 if(dt<=movs[f].m0 && movs[f].dir==-1)
  {
   if(movs[f].g && sonido_autom==0)
    {
     reproducir_sonido(19,PLAYMODE_PLAY);
     sonido_autom=3;
    }
   movs[f].dir=1;
  }
 if(dt>=movs[f].m1 && movs[f].dir==1)
  {
   if(movs[f].g && sonido_autom==0)
    {
     reproducir_sonido(19,PLAYMODE_PLAY);
     sonido_autom=3;
    }
   movs[f].dir=-1;
  }

 if(movs[f].g==0) movs[f].g=1;

 // Si lo consigo mover, muevo tambi�n los anclados.
 if(!ism_cambiar_dato_objeto(movs[f].id, movs[f].sentmov, movs[f].dir, SUMAR))
   for(n=0;n<num_movs;n++)
     if(movs[f].id==movs[n].anclado)
       ism_cambiar_dato_objeto(movs[n].id, movs[f].sentmov, movs[f].dir, SUMAR);
}

//******************************************************************************
// Funci�n mov_joystick(...)
//    Funci�n de movimiento del joystick controlador del telebot.
//******************************************************************************
void mov_joystick(char f)
{
 char ac0,ac1,n;
 for(ac0=ac1=n=0;n<4;n++)
  {
   if(dt_telebot.tc[n]!=dt_telebot.ta[n])
     ism_cambiar_mapa_objeto(dt_telebot.id[n],fm_joy((n+dt_telebot.ta[n]*4),0));
   ac0+=dt_telebot.tc[n];
   ac1+=dt_telebot.ta[n];
  }

 ac0+=dt_telebot.tc[4];
 ac1+=dt_telebot.ta[4];
 if(ac0==0 && ac1!=0)
  {ism_id t=dt_telebot.id_t[0];
   dt_telebot.id_t[0]=dt_telebot.id_t[1];
   dt_telebot.id_t[1]=t;
   movs[f].m0=reproducir_sonido(6, PLAYMODE_LOOP);
  }
 else if(ac0!=0 && ac1==0)
   detener_sonido((char)movs[f].m0);

 for(n=0;n<5;n++) dt_telebot.tc[n]=dt_telebot.ta[n];
}

//******************************************************************************
// Funci�n mov_desaparece(...)
//    Funci�n de movimiento del cubo que desaparece al pisarlo.
//******************************************************************************
void mov_desaparece(char f)
{
 if(movs[f].m0) // A�n no ha desaparecido.
 if(movs[f].id==movs[0].anclado || movs[f].m1>0)
  { // Est� siendo (o ha sido) pisado.
   if(movs[f].m1<2) reproducir_sonido(1,PLAYMODE_PLAY);
   movs[f].m1+=10;
   if(movs[f].m1<100)
    {
     ism_cambiar_dato_objeto(movs[f].id,D_TRANSP,movs[f].m1,CAMBIAR);
    }
   else
    {
     movs[f].m0=0;
     ism_quitar_objeto(movs[f].id);
    }
  }
}

//******************************************************************************
// Funci�n mov_mina_voladora(...)
//    Funci�n de movimiento de las minas voladoras.
//******************************************************************************
void mov_mina_voladora(char f)
{
 if(movs[f].m0) // Tiene que elegir la siguiente mina en subir.
  {char i=f,n=(char)(rand()%movs[f].m1);
   while(n) if(movs[++i].mover==mov_mina_voladora) n--;
   movs[i].g=1;
   movs[f].m0=0;
  }

 if(movs[f].g<100)
  {// Est� activa.
   if(movs[f].g==1)
    { // Est� subiendo.
     if(ism_cambiar_dato_objeto(movs[f].id,D_Z,1,SUMAR))
      {
       movs[f].g=0;
       if(ism_colisionado_con(movs[0].id)) mov_nacho_explotar(1);
      }
     else if(ism_obtener_dato_objeto(movs[f].id,D_Z)>=ALTURA_BASE*4)
      {
       movs[f].g=0;
      }
    }
   else
    {// Est� bajando.
     if(ism_cambiar_dato_objeto(movs[f].id,D_Z,movs[f].g/2-1,SUMAR))
      {
       if(movs[f].g<-1) reproducir_sonido(8+rand()%4,PLAYMODE_PLAY);
       if(movs[f].g) while(!ism_cambiar_dato_objeto(movs[f].id,D_Z,-1,SUMAR));
       movs[f].g=100;
       movs[primera_mina_voladora].m0=1;
       if(ism_colisionado_con(movs[0].id)) mov_nacho_explotar(1);
      }
     else
       movs[f].g--;
    }
  }
}

//******************************************************************************
// Funci�n mov_mina_terrestre(...)
//    Funci�n de movimiento de las minas terrestres.
//******************************************************************************
void mov_mina_terrestre(char f)
{
 if(movs[f].m0) // La mina est� activa.
  {
   if(movs[f].m1==0)
    {int x,y,z;
     ism_obtener_coords_objeto(movs[f].id,&x,&y,&z);
     ism_obtener_coords_libres(x,y,&x,&y);
     ism_quitar_objeto(movs[f].id);
     movs[f].id=ism_colocar_objeto_libre(x,y,z,ANCHO_CELDA,ANCHO_CELDA,ALTURA_BASE,NULL,NULL);
     reproducir_sonido(0,PLAYMODE_PLAY);
     // Lo ponemos como no empujable, para que si se ha
     // chocado con dos minas, tan solo explote esta.
     movs[f].empujable=0;
    }

   if(movs[f].dir)
    {
     movs[f].dir--;
    }
   else
    {
     movs[f].dir=1;

     movs[f].m1++;
     if(movs[f].m1<8)
      {
       ism_cambiar_mapa_objeto(movs[f].id,fm_minat(movs[f].m1));
       ism_cambiar_dato_objeto(movs[f].id,D_TRANSP,8,SUMAR);
      }
     else if(movs[f].m1<10)
      {
       ism_cambiar_mapa_objeto(movs[f].id,fm_plasma((movs[f].m1==8?1:0)));
      }
     else
      {
       ism_quitar_objeto(movs[f].id);
       movs[f].id=NO_ID;
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_telebot(...)
//    Funci�n de movimiento del robot teledirigido.
//******************************************************************************
void mov_telebot(char f)
{
 static char mover=0;
 char n;

 // Gravedad.
 if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<-1) reproducir_sonido(8+rand()%4,PLAYMODE_PLAY);
     if(movs[f].g<0) while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
     if(ism_colisionado_con(ID_SUELO))
      {
       ism_cambiar_sombra_objeto(movs[f].id,NULL);
       movs[f].g=1;
      }
     else
      {
       ism_id id=ism_extraer_id_blq();
       char n;
       for(n=0;n<num_movs;n++)
         if(id==movs[n].id)
          {// Comprobamos si ha caido sobre una mina.
           if(movs[n].mover==mov_mina_terrestre) movs[n].m0=1;
           n=num_movs;
          }
       movs[f].g=0;
      }
    }
   else
    movs[f].g--;
  }

 if(movs[f].id==dt_telebot.id_t[0])
  {
   unsigned char dir=0;
   char c=0;
   for(n=0;n<4;n++)
     if(dt_telebot.tc[n])
      {
       c++;
       dir=n;
      }
   // Cambiamos el fotograma.
   if(c || dt_telebot.tc[4])
    {
     if(movs[f].m1==3)
      {
       movs[f].m1=0;
       if(movs[f].m0==3)
         movs[f].m0=0;
       else
         movs[f].m0++;
      }
     else
       movs[f].m1++;

     if(movs[f].m1 == 0)
       ism_cambiar_mapa_objeto(dt_telebot.id_t[0],fm_telebot(movs[f].m0));

     // Movemos el telebot
     if(c=1 && dt_telebot.tc[4]==0)
      {
       movs[f].sentmov=(dir&1?D_Y:D_X);
       movs[f].dir=(dir<2?1:-1);
       if(mover)
        {
         mover=0;
         if(!ism_cambiar_dato_objeto(movs[f].id, movs[f].sentmov, movs[f].dir, SUMAR))
          {
           for(n=0;n<num_movs;n++) // Si tiene alg�n objeto anclado, lo movemos tambi�n.
            {
             if(movs[f].id==movs[n].anclado)
               if(ism_cambiar_dato_objeto(movs[n].id, movs[f].sentmov, movs[f].dir, SUMAR))
                 mov_empujar(f,movs[f].sentmov, movs[f].dir*2);
            }
          }
         else
           mov_empujar(f,movs[f].sentmov, movs[f].dir);
        }
       else
         mover=1;
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_rbal_l(...)
//    Funci�n de movimiento del robot alien�gena (dalek) de movimiento libre.
//    m0: temporizador de fotogramas.
//    m1: indicador de secuencia de giro.
//    sentmov: direcci�n de movimiento [0,1,2,3].
//    dir: fotograma.
//******************************************************************************
void mov_rbal_l(char f)
{
  // Gravedad.
  if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<-1) reproducir_sonido(8+rand()%4,PLAYMODE_PLAY);
     do
      {
       if(ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR))
        if(ism_colisionado_con(ID_SUELO))
         {
          ism_cambiar_sombra_objeto(movs[f].id,NULL);
          movs[f].g=1;
         }
        else
          movs[f].g=0;
      }while(movs[f].g<0);
    }
   else
     movs[f].g--;
  }

 // Se controla el cambio de fotograma.
 if(movs[f].m0==0)
  {
   // cambiar mapa
   if(movs[f].dir==3)
     movs[f].dir=0;
   else
     movs[f].dir++;
   if(movs[f].m1>1)
    {// Est� parado por haber chocado con algo. Hay que girar el robot.
     if(rand()&1)
      {
       ism_cambiar_mapa_objeto(movs[f].id,fm_robotal(movs[f].sentmov,4));
       movs[f].sentmov=(movs[f].sentmov+1)&3;
      }
     else
      {
       movs[f].sentmov=(movs[f].sentmov+3)&3;
       ism_cambiar_mapa_objeto(movs[f].id,fm_robotal(movs[f].sentmov,4));
      }
     movs[f].m1--;
    }
   else
    {
     ism_cambiar_mapa_objeto(movs[f].id,fm_robotal(movs[f].sentmov,movs[f].dir));
     movs[f].m1=0;
    }
   movs[f].m0=3;
  }
 else
  {
   movs[f].m0--;
  }

 // Se controla el movimiento.
 if(movs[f].m1==0)
  {// No est� parado por haber chocado con algo
   unsigned char dato;
   int valor;
   if(ism_cambiar_dato_objeto(movs[f].id,dato=(movs[f].sentmov&1?D_Y:D_X),valor=(movs[f].sentmov>1?1:-1), SUMAR))
    {// Cambia de direcci�n.
     movs[f].m0=3;
     movs[f].m1=2;
     mov_empujar(f,dato,valor*2);
    }
   else
    {char n;
     for(n=0;n<num_movs;n++) // Si tiene alg�n objeto anclado, lo movemos tambi�n.
      {
       if(movs[f].id==movs[n].anclado)
         ism_cambiar_dato_objeto(movs[n].id,(movs[f].sentmov&1?D_Y:D_X),(movs[f].sentmov>1?1:-1), SUMAR);
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_rbal_f(...)
//    Funci�n de movimiento del robot alien�gena (dalek) de movimiento fijo.
//    m0: temporizador de fotogramas.
//    m1: indicador de secuencia de giro.
//    sentmov: direcci�n de movimiento [0,1,2,3].
//    dir: fotograma.
//******************************************************************************
void mov_rbal_f(char f)
{
 // Gravedad.
 if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<-1) reproducir_sonido(8+rand()%4,PLAYMODE_PLAY);
     if(movs[f].g<0) while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
     if(ism_colisionado_con(ID_SUELO))
      {
       ism_cambiar_sombra_objeto(movs[f].id,NULL);
       movs[f].g=1;
      }
     else
      {
       movs[f].g=0;
      }
    }
   else
    movs[f].g--;
  }

 // Se controla el cambio de fotograma.
 if(movs[f].m0==0)
  {
   // cambiar mapa
   if(movs[f].dir==3)
     movs[f].dir=0;
   else
     movs[f].dir++;
   if(movs[f].m1>1)
    {// Est� parado por haber chocado con algo. Hay que girar el robot.
     if(movs[f].m1&1)
      {
       ism_cambiar_mapa_objeto(movs[f].id,fm_robotal(movs[f].sentmov,4));
      }
     else
      {
       movs[f].sentmov=(movs[f].sentmov+1)&3;
       ism_cambiar_mapa_objeto(movs[f].id,fm_robotal(movs[f].sentmov,movs[f].dir));
      }
     movs[f].m1--;
    }
   else
    {
     ism_cambiar_mapa_objeto(movs[f].id,fm_robotal(movs[f].sentmov,movs[f].dir));
     movs[f].m1=0;
    }
   movs[f].m0=3;
  }
 else
  {
   movs[f].m0--;
  }

 // Se controla el movimiento.
 if(movs[f].m1==0)
  {// No est� parado por haber chocado con algo
   unsigned char dato;
   int valor;
   if(ism_cambiar_dato_objeto(movs[f].id,dato=(movs[f].sentmov&1?D_Y:D_X),valor=(movs[f].sentmov>1?1:-1),SUMAR))
    {// Cambia de direcci�n.
     movs[f].m0=0;
     movs[f].m1=4;
     mov_empujar(f,dato,valor*2);
    }
   else
    {char n;
     for(n=0;n<num_movs;n++) // Si tiene alg�n objeto anclado, lo movemos tambi�n.
      {
       if(movs[f].id==movs[n].anclado)
         ism_cambiar_dato_objeto(movs[n].id,(movs[f].sentmov&1?D_Y:D_X),(movs[f].sentmov>1?1:-1),SUMAR);
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_ratamec(...)
//    Funci�n de movimiento de la rata mec�nica.
//******************************************************************************
void mov_ratamec(char f)
{
 char n;

 // Gravedad.
 if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<-1) reproducir_sonido(14,PLAYMODE_PLAY);
     if(movs[f].g<0) while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
     if(ism_colisionado_con(ID_SUELO))
      {
       ism_cambiar_sombra_objeto(movs[f].id,NULL);
       movs[f].g=1;
      }
     else
      {
       movs[f].g=0;
      }
    }
   else
    movs[f].g--;
  }

 // Movimiento.
 if((rand()%25)==0)
  {// Cambio aleatorio de direcci�n.
   n=1;
  }
 else
  {
   if(movs[f].m1)
     n=ism_cambiar_dato_objeto(movs[f].id, movs[f].sentmov, movs[f].dir, SUMAR);
   else
     n=0;
   if(n) mov_empujar(f,movs[f].sentmov,movs[f].dir);
  }

 if(n) // Si tiene que cambiar de sentido...
  {
   movs[f].sentmov=(movs[f].sentmov==D_X?D_Y:D_X);
   if(!(rand()%4)) movs[f].m1=(movs[f].m1<0?0:-1);
   movs[f].dir*=(rand()&1?1:-1);
   reproducir_sonido(5,PLAYMODE_PLAY);
  }

 // Se controla el cambio de fotograma.
 if(movs[f].m0==11)
   movs[f].m0=0;
 else
   movs[f].m0++;
 n=movs[f].m0%3;
 if((movs[f].dir==2 || movs[f].dir==-2) && n==2)
  {
   if(movs[f].m0==11)
     movs[f].m0=0;
   else
     movs[f].m0++;
   n=0;
  }

 if(n==0)
  {
   if(movs[f].sentmov==D_X)
     n=(movs[f].dir<0?3:1);
   else
     n=(movs[f].dir<0?0:2);
   ism_cambiar_mapa_objeto(movs[f].id,fm_ratamec(n,(movs[f].m0/3)));
  }

 if(movs[f].m1>=0) movs[f].m1=(movs[f].m1?0:1);
}

//******************************************************************************
// Funci�n mov_saturno(...)
//    Funci�n de movimiento del enemigo saturno.
//******************************************************************************
void mov_saturno(char f)
{
 int xs,ys,xn,yn;

 // Gravedad.
 if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<-1) reproducir_sonido(14,PLAYMODE_PLAY);
     if(movs[f].g<0) while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
     if(ism_colisionado_con(ID_SUELO))
       movs[f].g=1;
     else
       movs[f].g=0;
    }
   else
    movs[f].g--;
  }

 // Cambio de fotograma.
 if(movs[f].m0==8*4-1)
  {
   movs[f].m0=0;
  }
 else
  {
   movs[f].m0++;
  }
  if(movs[f].m0%4==0)
   {char ft;
    ism_cambiar_mapa_objeto(movs[f].id,fm_saturno(ft=movs[f].m0/4));
    if(ft==0 || ft==4) reproducir_sonido(3,PLAYMODE_PLAY);
   }

 if(movs[0].id!=NO_ID)
  {
   if(movs[f].m1==0)
    {// Movimiento.
     ism_obtener_coords_objeto(movs[f].id,&xs,&ys,NULL);
     ism_obtener_coords_objeto(movs[0].id,&xn,&yn,NULL);
     if(xs>xn)
       xs=-1;
     else if(xs<xn)
       xs=1;
     else
       xs=0;
     if(ys>yn)
       ys=-1;
     else if(ys<yn)
       ys=1;
     else
       ys=0;
     if(xs) if(ism_cambiar_dato_objeto(movs[f].id,D_X,xs,SUMAR))
       mov_empujar(f,D_X,xs);
     if(ys) if(ism_cambiar_dato_objeto(movs[f].id,D_Y,ys,SUMAR))
       mov_empujar(f,D_Y,ys);
     movs[f].m1=2;
    }
   else
     movs[f].m1--;
  }
}

//******************************************************************************
// Funci�n mov_plasma(...)
//    Funci�n de movimiento del enemigo plasma.
//******************************************************************************
void mov_plasma(char f)
{
 char n,i;
 int dif;
 unsigned char dato;

 // Gravedad.
 if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<0) while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
     if(ism_colisionado_con(ID_SUELO))
       movs[f].g=1;
     else
       movs[f].g=0;
    }
   else
    movs[f].g--;
  }

 // Cambio de fotograma.
 movs[f].sentmov+=movs[f].dir;
 if(movs[f].sentmov==0)
  {
   movs[f].dir=1;
   reproducir_sonido(4,PLAYMODE_PLAY);
  }
 else if(movs[f].sentmov==14)
  {
   movs[f].dir=-1;
  }
 if((movs[f].sentmov&1)==0)
   ism_cambiar_mapa_objeto(movs[f].id,fm_plasma(movs[f].sentmov/2));

 // Movimiento.
 if(movs[f].id==movs[0].anclado)
  {// Nacho est� encima.
   for(n=0,dato=D_X;n<2;n++,dato=D_Y)
    {char mover_n=0;
     dif=ism_obtener_dato_objeto(movs[0].id,dato)-ism_obtener_dato_objeto(movs[f].id,dato);
     if(dif==0)
      {
       dif=(dato==D_X?movs[f].m0:movs[f].m1);
       mover_n=1;
      }
     else if(dif>1)
      {
       dif=1;
      }
     else if(dif<-1)
      {
       dif=-1;
      }
     if(i=ism_cambiar_dato_objeto(movs[f].id,dato,dif,SUMAR))
      {
       mov_empujar(f,dato,dif);
       i=ism_cambiar_dato_objeto(movs[f].id,dato,dif,SUMAR);
      }
     else
       if ( dato == D_X )
       {
        movs[f].m0 = ( dif < 0 ? -1 : 1 );
        }
       else
       {
        movs[f].m1 = ( dif < 0 ? -1 : 1 );
      }
     if(!i && mover_n)
       for(i=0;i<3;i++)
         if(movs[f].id==movs[i].anclado)
           ism_cambiar_dato_objeto(movs[i].id,dato,dif,SUMAR);
    }
  }
 else
  { // Nacho no est� encima.
   if(movs[0].id!=NO_ID)
     for(n=0,dato=D_X;n<2;n++)
      {
       if(n>=1) dato=D_Y;
       dif=ism_obtener_dato_objeto(movs[0].id,dato)-ism_obtener_dato_objeto(movs[f].id,dato);
       if(dif)
        {
         dif=(dif>0?1:-1);
         if(i=ism_cambiar_dato_objeto(movs[f].id,dato,dif,SUMAR))
          {
           movs[0].sentmov=0;
           mov_empujar(f,dato,dif);
           i=ism_cambiar_dato_objeto(movs[f].id,dato,dif,SUMAR);
          }
         else
           if (dato==D_X)
           {
             movs[f].m0=dif;
           }
           else
           {
             movs[f].m1=dif;
           }
         if(!i)
           for(i=1;i<num_movs;i++)
             if(movs[f].id==movs[i].anclado)
                ism_cambiar_dato_objeto(movs[i].id,dato,dif,SUMAR);
        }
      }
  }
}

//******************************************************************************
// Funci�n mov_cascara(...)
//    Mueve las c�scaras que caen del techo.
//******************************************************************************
void mov_cascara(char f)
{
 if(movs[f].g<=0)
  {// Est� cayendo.
   if(ism_cambiar_dato_objeto(movs[f].id,D_Z,movs[f].g/2-1,SUMAR))
    {// Ha chocado con algo.
     if(movs[f].g<-1) reproducir_sonido(12,PLAYMODE_PLAY);
     if(movs[f].g) while(!ism_cambiar_dato_objeto(movs[f].id,D_Z,-1,SUMAR));
     movs[f].g=101;
     movs[primera_cascara].m0=1;
     if(ism_colisionado_con(movs[0].id)) mov_nacho_explotar(1);
    }
   movs[f].g--;
  }
 else if(movs[f].m0==1)
  {// Es la primera y tiene que elegir una que caiga.
   char n=(char)(rand()%movs[f].m1);
   movs[f].m0=2;
   while(n) if(movs[++f].mover==mov_cascara) n--;
   movs[f].g=0;
  }
}

//******************************************************************************
// Funci�n mov_cerradura(...)
//    Controla la cerradura.
//******************************************************************************
void mov_cerradura(char f)
{
 char n;
 char llave_movida=0;
 static char transparencia=40;
 static char dirtransp=1;

 movs[f].empujable=(movs[f].empujable?0:1);
 // Atrae las llaves del mismo tipo.
 for(n=1;n<3;n++)
  {
   if(movs[n].id!=NO_ID && movs[n].empujable)
     if(llave[movs[n].m0].tipo==movs[f].dir)
      {char mov=0;
       if(movs[f].m0!=llave[movs[n].m0].x)
        {
         if(ism_cambiar_dato_objeto(movs[n].id,D_X,mov=(movs[f].m0>llave[movs[n].m0].x?1:-1)*movs[f].empujable,SUMAR))
           mov_empujar(n,D_X, mov*2);
         else
           llave_movida=1;
        }
       if(movs[f].m1!=llave[movs[n].m0].y)
        {
         if(ism_cambiar_dato_objeto(movs[n].id,D_Y,mov=(movs[f].m1>llave[movs[n].m0].y?1:-1)*movs[f].empujable,SUMAR))
           mov_empujar(n,D_Y, mov*2);
         else
           llave_movida=1;
        }
       if(!mov && movs[f].empujable && movs[f].g==llave[movs[n].m0].z)
        {// Contacto.
         movs[n].empujable=0;
         ism_cambiar_sombra_objeto(movs[n].id,NULL);
         if(!(hab[h].banderas&0x08)) abrir_cerradura();
        }
      }
  }

 // Compruebo si he de encender o apagar el sonido
 if(llave_movida)
  {
   if(canal_llave<0) canal_llave=reproducir_sonido(13, PLAYMODE_LOOP);
  }
 else
  {
   if(canal_llave>=0)
    {
     detener_sonido(canal_llave);
     canal_llave=-1;
    }
  }

 // Compruebo si he de encender el holograma.
 if(movs[1].id==NO_ID && movs[2].id==NO_ID && movs[f].id==NO_ID)
  {
   movs[f].id=ism_colocar_objeto_libre(movs[f].m0,movs[f].m1,movs[f].g,12,12,0,fm_llave(movs[f].dir,0),NULL);
   if(movs[f].id!=ID_ERROR)
    {
     ism_establecer_oscuridad(oscuridad=0);
     movs[f].sentmov=100+((unsigned char)(rand()%156));
    }
   else
    movs[f].id=NO_ID;
  }

 // Compruebo si he de apagar el holograma.
 if((movs[1].id!=NO_ID || movs[2].id!=NO_ID) && movs[f].id!=NO_ID)
  {
   if(!(hab[h].banderas&0x08))
    {
     ism_establecer_oscuridad(50);
     oscuridad=5;
    }
   ism_quitar_objeto(movs[f].id);
   movs[f].id=NO_ID;
   if(sonido_interferencia>=0)
    {
     detener_sonido(sonido_interferencia);
     sonido_interferencia=-1;
    }
  }

 // Control de interferencias y transparencia del holograma
 if(movs[f].id!=NO_ID)
  {
   if(transparencia==40) dirtransp=1;
   if(transparencia==80) dirtransp=-1;
   transparencia+=dirtransp;
   ism_cambiar_dato_objeto(movs[f].id,D_TRANSP,transparencia,CAMBIAR);

   if(movs[f].sentmov==0)
    {int fot=rand()%10;
     ism_cambiar_mapa_objeto(movs[f].id,fm_llave(movs[f].dir,fot));
     if(fot==0)
      { // Apago la interferencia.
       if(sonido_interferencia>=0)
        {
         detener_sonido(sonido_interferencia);
         sonido_interferencia=-1;
        }
       ism_establecer_oscuridad(oscuridad=0);
       movs[f].sentmov=100+((unsigned char)(rand()%156));
      }
     else
      { // Si estoy encendiendo la interferencia reproduzco el sonido.
       ism_establecer_oscuridad(5*(oscuridad=1+(rand()%4)));
       if(sonido_interferencia<0) sonido_interferencia=reproducir_sonido(20,PLAYMODE_LOOP);
       movs[f].sentmov=5;
      }
    }
   else
     movs[f].sentmov--;
  }
}

//******************************************************************************
// Funci�n mov_bolsillo(...)
//    Controla los bolsillos, que se dibujan como paredes inferiores.
//******************************************************************************
void mov_bolsillo(char f)
{
 if(movs[f].m0!=movs[f].m1)
  {
   if(f<5) movs[f+1].m1= movs[f].m0<0?-2:movs[f].m0 ;
   if(movs[f].m1>=0)
    {
     movs[f].m0=movs[f].m1;
     if(f==3 && primera_cascara>0)
       if(movs[primera_cascara].m0<2)
         movs[primera_cascara].m0=1;
    }
   else
     movs[f].m0=movs[f].m1=-1;
   movs[f].dir=1;
  }
 if(movs[f].dir)
  {
   char p=f-(dx&&dy?1:2);
   if(nuevo_marcador)
    {
     ism_colocar_pared(movs[f].m0<0?NULL:fm_bolsillo(llave[movs[f].m0].tipo),INF_X,p);
     ism_mover_pared(INF_X,p,(dx&&dy?-4:-40)+(f-3)*5+5,(dy?-13:-121)+(f==5?1:0));
    }
   else
    {
     ism_colocar_pared(movs[f].m0<0?NULL:fm_llave(llave[movs[f].m0].tipo,0),INF_X,p);
     ism_mover_pared(INF_X,p,(dx&&dy?-4:-40),(dy?-20:-128));
    }
   movs[f].dir=0;
  }
}

//******************************************************************************
// Funci�n mov_llave(...)
//    Mueve las llaves.
//******************************************************************************
void mov_llave(char f)
{
 // Si cambio el objeto...
 if(movs[f].m0!=movs[f].m1)
 {
  if(movs[f].m0<0)
   {
    if(movs[f].m1<0)
     { // No cojo y no suelto.
      movs[f].m0=movs[f].m1=-1;
      movs[3].m1=-2;
     }
    else
     { // No cojo y suelto.
      int x,y,z;
      llave[movs[f].m0=movs[f].m1].hab=h;
      movs[3].m1=-2;
      ism_obtener_coords_objeto(movs[0].id,&x,&y,&z);
      movs[f].id=ism_colocar_objeto_libre(x+1,y-1,z-ALTURA_BASE,12,12,ALTURA_BASE,
                                          fm_llave(llave[movs[f].m0].tipo,0),
                                          fm_sombra_llave(llave[movs[f].m0].tipo));
      movs[f].g=0;
      hab[h].banderas+=2;
     }
   }
  else
   {
    if(movs[f].m1<0)
     { // Cojo y no suelto.
      ism_quitar_objeto(movs[f].id);
      movs[f].id=movs[f].anclado=NO_ID;
      movs[f].m0=movs[f].m1=llave[movs[3].m1=movs[f].m0].hab=-1;
      hab[h].banderas-=2;
     }
    else
     { // Cojo y suelto.
      if(llave[movs[f].m0].tipo!=llave[movs[f].m1].tipo)
       {
        ism_cambiar_mapa_objeto(movs[f].id,fm_llave(llave[movs[f].m1].tipo,0));
        ism_cambiar_sombra_objeto(movs[f].id,fm_sombra_llave(llave[movs[f].m1].tipo));
       }
      llave[movs[3].m1=movs[f].m0].hab=-1;
      llave[movs[f].m0=movs[f].m1].hab=h;
     }
   }
 }

 if(movs[f].id!=NO_ID)
  {
   // Gravedad.
   if(movs[f].g<1)
    {
     if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
      {// Ha chocado con algo mientras ca�a.
       if(movs[f].g<-1 && movs[f].empujable) reproducir_sonido(14+rand()%3,PLAYMODE_PLAY);
       if(movs[f].g<0)
         while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
       if(ism_colisionado_con(ID_SUELO))
        { // Si ha chocado con el suelo le desactivo la gravedad y elimino el ancla.
         movs[f].g=1;
         movs[f].anclado=NO_ID;
        }
       else
        { // Anclo la llave.
         movs[f].g=0;
         anclar(f);
        }
      }
     else
      {// Ha ca�do sin problemas
       movs[f].g--;
       movs[f].anclado=NO_ID;
      }
    }
   // Actualizo las coordenadas x e y
   ism_obtener_coords_objeto(movs[f].id,&llave[movs[f].m0].x,&llave[movs[f].m0].y,&llave[movs[f].m0].z);
  }
}

//******************************************************************************
// Funci�n mov_vida(...)
//    Mueve las vidas extra.
//******************************************************************************
void mov_vida(char f)
{
 if(movs[f].m1)
  {// Est� desapareciendo.
   if(movs[f].sentmov==0)
    {
     char cv[2]={0,0};
     dt_partida.vidas++;
     if(nuevo_marcador)
      {BITMAP *mpt=create_bitmap(19,10);
       clear_to_color(mpt,makecol(255,0,255));
       cv[0]=LETRA_CERR_0+dt_partida.vidas;
       textout_ex(mpt, (FONT *)(f_fuentes[1].dat), cv, 0, -8, -1, -1);
       blit(fm_pared_msx(0),mapa_vidas,35,72,35,72,19,10);
       set_trans_blender(0,0,0,192);
       draw_trans_sprite(mapa_vidas,mpt,35,72);
       ism_mover_pared(SUP_X,1,0,0);
       destroy_bitmap(mpt);
      }
     else
      {
       clear_bitmap(mapa_vidas);
       cv[0]=LETRA_VDAS_0+dt_partida.vidas;
       textout_ex(mapa_vidas, (FONT *)(f_fuentes[0].dat), cv, 0, 0, -1, -1);
       ism_mover_pared(SUP_X,2,0,0);
      }
     // Hemos llamado a ism_mover_pared, no para moverla realmente, sino para
     // incluirla en el rect�ngulo sucio del siguiente fotograma.
     reproducir_sonido(22,PLAYMODE_PLAY);
    }
   if((movs[f].sentmov+=15)>=100)
    {
     if(primera_cascara>0)
       if(movs[primera_cascara].m0<2)
         movs[primera_cascara].m0=1;
     ism_quitar_objeto(movs[f].id);
     llave[movs[f].m0].hab=-1;
     hab[h].banderas-=2;
     movs[f].id=NO_ID;
     movs[f].m0=movs[f].m1=-1;
     movs[f].mover=mov_llave;
    }
   else
    {
     ism_cambiar_dato_objeto(movs[f].id,D_TRANSP,movs[f].sentmov,CAMBIAR);
    }
  }

 if(movs[f].mover==mov_vida)
  {
   // Gravedad.
   if(movs[f].g<1)
    {
     if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
      {// Ha chocado con algo mientras ca�a.
       if(movs[f].g<-1) reproducir_sonido(14+rand()%3,PLAYMODE_PLAY);
       if(movs[f].g<0)
         while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
       if(ism_colisionado_con(ID_SUELO))
        { // Si ha chocado con el suelo le desactivo la gravedad, elimino el ancla y quito la sombra.
         movs[f].g=1;
         movs[f].anclado=NO_ID;
         ism_cambiar_sombra_objeto(movs[f].id,NULL);
        }
       else
        { // Anclo la vida.
         movs[f].g=0;
         anclar(f);
        }
      }
     else
      {// Ha ca�do sin problemas
       movs[f].g--;
       movs[f].anclado=NO_ID;
      }
    }
   // Actualizo las coordenadas x e y
   ism_obtener_coords_objeto(movs[f].id,&llave[movs[f].m0].x,&llave[movs[f].m0].y,&llave[movs[f].m0].z);
   // Se cambia el mapa
   if(movs[f].dir==5)
    {
     ism_cambiar_mapa_objeto(movs[f].id,fm_llave(4,rand()%4));
     movs[f].dir=0;
    }
   else
     movs[f].dir++;
  }
}

//******************************************************************************
// Funci�n mov_empujable(...)
//    Mueve los objetos empujables.
//******************************************************************************
void mov_empujable(char f)
{
 // Gravedad.
 if(movs[f].g<1)
  {
   if(ism_cambiar_dato_objeto(movs[f].id, D_Z, movs[f].g/2-1, SUMAR))
    {
     if(movs[f].g<-1) reproducir_sonido(8+rand()%4,PLAYMODE_PLAY);
     if(movs[f].g<0) while(!ism_cambiar_dato_objeto(movs[f].id, D_Z, -1, SUMAR));
     if(ism_colisionado_con(ID_SUELO))
      {
       ism_cambiar_sombra_objeto(movs[f].id,NULL);
       movs[f].g=1;
       if(movs[f].m0) movs[f].anclado=NO_ID;
      }
     else
      {
       movs[f].g=0;
       if(movs[f].m0) anclar(f);
      }
    }
   else
    {
     movs[f].g--;
     if(movs[f].m0) movs[f].anclado=NO_ID;
    }
  }
}

//******************************************************************************
// Funci�n mov_astronauta(...)
//    Funci�n de animaci�n de las c�psulas criog�nicas.
//******************************************************************************
void mov_astronauta(char f)
{
 if(movs[f].m0)
  {
   if(movs[f].m1==120)
    {
     movs[f].m1=0;
     ism_cambiar_mapa_objeto(movs[f].id,fm_astr(movs[f].sentmov,movs[f].dir=(movs[f].dir?0:1)));
    }
   else
     movs[f].m1++;
  }
}

//******************************************************************************
// Funci�n mov_nacho_andar(...)
//    Mueve a Nacho cuando est� andando.
//******************************************************************************
void mov_nacho_andar(char f)
{
 int colision=0;
 unsigned char dato;
 int valor=0;

 for(f=0;f<5;dt_telebot.ta[f++]=0);

 // Gravedad
 if(colision=ism_cambiar_dato_objeto(movs[0].id, D_Z, movs[0].g/2-1, SUMAR))
  {
   if(movs[0].g<-1) reproducir_sonido(14+rand()%3,PLAYMODE_PLAY);
   if(movs[0].g)
    {
     while(!(colision=ism_cambiar_dato_objeto(movs[0].id, D_Z, -1, SUMAR)));
     movs[0].g=0;
    }

   if(anclar(0)) mov_nacho_explotar(1);

   if(movs[0].mover!=mov_nacho_explotar)
    {
     if( (tcj_estado[TCJ_AC0] || tcj_estado[TCJ_AC2]) &&
        !(tcj_estado[TCJ_IZQ] && movs[0].dir!=2) &&
        !(tcj_estado[TCJ_ABJ] && movs[0].dir!=1) &&
        !(tcj_estado[TCJ_ARR] && movs[0].dir!=3) &&
        !(tcj_estado[TCJ_DER] && movs[0].dir!=0))
      {// Comprobamos si empezamos a saltar
       if(puede_saltar)
        {
         movs[0].mover=mov_nacho_saltar;
         movs[0].g=25;
         reproducir_sonido(26,PLAYMODE_PLAY);
        }
      }
     else
       puede_saltar=1;
     // Comprobamos el joystick controlador del telebot.
     if(dt_telebot.id[0]!=NO_ID)
       for(f=0;f<5;f++)
         dt_telebot.ta[f]=ism_colisionado_con(dt_telebot.id[f]);
    }
  }
 else
  {
   movs[0].g--;
   movs[0].anclado=NO_ID;
  }

 if(movs[0].mover==mov_nacho_andar)
  {
   movs[0].sentmov=0;
   colision=0;
   if(tcj_estado[TCJ_IZQ])
    {
     if(movs[0].dir!=2)
      {
       movs[0].mover=mov_nacho_girar;
       movs[0].dir=(movs[0].dir==3?10:movs[0].dir+4);
       dt_anim_nacho.fot=(movs[0].dir>7?2:0);
       dt_anim_nacho.cont=0;
      }
     else
      {
       if(dt_anim_nacho.fot==0) dt_anim_nacho.fot=1;
       if(colision=ism_cambiar_dato_objeto(movs[0].id,dato=D_X,valor=-1,SUMAR))
         movs[0].sentmov=2;
       else
         movs[0].sentmov=1;
      }
    }
   if(tcj_estado[TCJ_ABJ])
    {
     if(movs[0].dir!=1)
      {
       movs[0].mover=mov_nacho_girar;
       movs[0].dir=(movs[0].dir==2?9:movs[0].dir+4);
       dt_anim_nacho.fot=(movs[0].dir>7?2:0);
       dt_anim_nacho.cont=0;
      }
     else
      {
       if(dt_anim_nacho.fot==0) dt_anim_nacho.fot=1;
       if(colision=ism_cambiar_dato_objeto(movs[0].id,dato=D_Y,valor=1,SUMAR))
         movs[0].sentmov=2;
       else
         movs[0].sentmov=1;
      }
    }
   if(tcj_estado[TCJ_ARR])
    {
     if(movs[0].dir!=3)
      {
       movs[0].mover=mov_nacho_girar;
       movs[0].dir=(movs[0].dir==0?11:movs[0].dir+4);
       dt_anim_nacho.fot=(movs[0].dir>7?2:0);
       dt_anim_nacho.cont=0;
      }
     else
      {
       if(dt_anim_nacho.fot==0) dt_anim_nacho.fot=1;
       if(colision=ism_cambiar_dato_objeto(movs[0].id,dato=D_Y,valor=-1,SUMAR))
         movs[0].sentmov=2;
       else
         movs[0].sentmov=1;
      }
    }
   if(tcj_estado[TCJ_DER])
    {
     if(movs[0].dir!=0)
      {
       movs[0].mover=mov_nacho_girar;
       movs[0].dir=(movs[0].dir==1?8:movs[0].dir+4);
       dt_anim_nacho.fot=(movs[0].dir>7?2:0);
       dt_anim_nacho.cont=0;
      }
     else
      {
       if(dt_anim_nacho.fot==0) dt_anim_nacho.fot=1;
       if(colision=ism_cambiar_dato_objeto(movs[0].id,dato=D_X,valor=1,SUMAR))
         movs[0].sentmov=2;
       else
         movs[0].sentmov=1;
      }
    }

   if(colision)
    {unsigned char cx;
     // Comprobamos si se sale de la habitaci�n
     movs[0].sentmov=3;
     if(ism_colisionado_con(ID_PARED_SUP_X) && puerta[0])
      {
       movs[0].m0=ism_obtener_dato_objeto(movs[0].id,D_X)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       movs[0].m1=10*ANCHO_CELDA-1;
       dt_anim_nacho.dirini=movs[0].dir;
       cx=hab[h].x_p & 0xf0;
       do{h--;}while((hab[h].x_p & 0xf0)!=cx);
       cargar_hab();
       valor=0;
      }
     if(ism_colisionado_con(ID_PARED_INF_Y) && puerta[1])
      {
       movs[0].m0=0;
       movs[0].m1=ism_obtener_dato_objeto(movs[0].id,D_Y)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       dt_anim_nacho.dirini=movs[0].dir;
       h++;
       cargar_hab();
       valor=0;
      }
     if(ism_colisionado_con(ID_PARED_INF_X) && puerta[2])
      {
       movs[0].m0=ism_obtener_dato_objeto(movs[0].id,D_X)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       movs[0].m1=15;
       dt_anim_nacho.dirini=movs[0].dir;
       cx=hab[h].x_p & 0xf0;
       do{h++;}while((hab[h].x_p & 0xf0)!=cx);
       cargar_hab();
       valor=0;
      }
     if(ism_colisionado_con(ID_PARED_SUP_Y) && puerta[3])
      {
       movs[0].m0=10*ANCHO_CELDA-15;
       movs[0].m1=ism_obtener_dato_objeto(movs[0].id,D_Y)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       dt_anim_nacho.dirini=movs[0].dir;
       h--;
       cargar_hab();
       valor=0;
      }
     if(valor)
      {
       if(ism_num_colisiones()==1)
        {char f,n; unsigned char dte; // Comprobamos las jambas de las puertas.
         if(dato==D_X)
          {f=0;dte=D_Y;}
         else
          {f=4;dte=D_X;}
         for(n=0;n<4;f++,n++)
           if(ism_colisionado_con(id_jamba[f]))
            {
             if(ism_cambiar_dato_objeto(movs[0].id,dte,valor=(f&1?-1:1),SUMAR))
               mov_empujar(0,dte,valor);
             n=10;
            }
         if(n<10) mov_empujar(0,dato,valor);
        }
       else
         mov_empujar(0,dato,valor);
      }
    }
  }

 if(embolsando && !tcj_estado[TCJ_AC1] && !tcj_estado[TCJ_AC2]) embolsando=0;
 if(!embolsando && (tcj_estado[TCJ_AC1] || tcj_estado[TCJ_AC2]))
  {
   embolsando=1;
   embolsar();
  }

 if(movs[0].mover!=mov_nacho_girar && movs[0].mover!=mov_nacho_explotar)
   mov_nacho_fotograma();
}

//******************************************************************************
// Funci�n mov_nacho_saltar(...)
//    Mueve a Nacho cuando est� saltando.
//******************************************************************************
void mov_nacho_saltar(char f)
{
 int colision=0;
 char valor=0;
 unsigned char dato;
 static char planeando=0;

 if(dt_telebot.id[0]!=NO_ID && movs[0].g==25)
   for(f=0;f<5;dt_telebot.ta[f++]=0);

 // Gravedad
 if(movs[0].g)
   if(ism_cambiar_dato_objeto(movs[0].id, D_Z, (movs[0].g>0?(movs[0].g>19?6:movs[0].g/4):movs[0].g/2), SUMAR))
    {
     while(!ism_cambiar_dato_objeto(movs[0].id, D_Z, (movs[0].g>0?1:-1), SUMAR));
     if(movs[0].g<-1) reproducir_sonido(14+rand()%3,PLAYMODE_PLAY);
     movs[0].mover=mov_nacho_andar;
     if(movs[0].g>0)
      {ism_id id=ism_extraer_id_blq();
       for(f=6;f<num_movs;f++) // Comprobamos si hemos chocado con un objeto que desaparece.
         if(movs[f].id==id && movs[f].mover==mov_desaparece && movs[f].m1==0)
          {
           movs[f].m1=1;
           f=MAX_MOVS+1;
          }
       if(f<=MAX_MOVS)
         for(f=0;f<num_obst;f++) // Comprobamos si hemos chocado con un obst�culo.
           if(id_obst[f]==id) mov_nacho_explotar(1);
      }
     movs[0].g=0;
    }

 movs[0].anclado=NO_ID;

 if(movs[0].mover==mov_nacho_saltar)
  {// No ha chocado con nada.
   if(!planeando)
    {
     if(!tcj_estado[TCJ_AC0] && !tcj_estado[TCJ_AC2] && movs[0].g>19)
       movs[0].g=18;
     else
       movs[0].g--;
     if(movs[0].g==0) planeando=4;
    }
   else
     planeando--;

   // Avanzo en la direcci�n del salto
   colision=ism_cambiar_dato_objeto(movs[0].id,dato=(movs[0].dir&1?D_Y:D_X),valor=(movs[0].dir<2?1:-1),SUMAR);

   if(colision)
    {unsigned char cx;
     // Comprobamos si se sale de la habitaci�n
     if(ism_colisionado_con(ID_PARED_SUP_X) && puerta[0])
      {
       movs[0].m0=ism_obtener_dato_objeto(movs[0].id,D_X)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       movs[0].m1=10*ANCHO_CELDA-1;
       cx=hab[h].x_p & 0xf0;
       do{h--;}while((hab[h].x_p & 0xf0)!=cx);
       dt_anim_nacho.dirini=movs[0].dir;
       cargar_hab();
       colision=0;
      }
     if(ism_colisionado_con(ID_PARED_INF_Y) && puerta[1])
      {
       movs[0].m0=0;
       movs[0].m1=ism_obtener_dato_objeto(movs[0].id,D_Y)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       h++;
       dt_anim_nacho.dirini=movs[0].dir;
       cargar_hab();
       colision=0;
      }
     if(ism_colisionado_con(ID_PARED_INF_X) && puerta[2])
      {
       movs[0].m0=ism_obtener_dato_objeto(movs[0].id,D_X)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       movs[0].m1=15;
       cx=hab[h].x_p & 0xf0;
       do{h++;}while((hab[h].x_p & 0xf0)!=cx);
       dt_anim_nacho.dirini=movs[0].dir;
       cargar_hab();
       colision=0;
      }
     if(ism_colisionado_con(ID_PARED_SUP_Y) && puerta[3])
      {
       movs[0].m0=10*ANCHO_CELDA-15;
       movs[0].m1=ism_obtener_dato_objeto(movs[0].id,D_Y)+(hab[h].banderas&1?0:3*ANCHO_CELDA);
       h--;
       dt_anim_nacho.dirini=movs[0].dir;
       cargar_hab();
       colision=0;
      }
     if(colision) mov_empujar(0,dato,valor);
    }
  }

 if(movs[0].mover!=mov_nacho_explotar)
  {
   if(dt_anim_nacho.fot==0) dt_anim_nacho.fot=1;
   mov_nacho_fotograma();
  }
}

//******************************************************************************
// Funci�n mov_nacho_fotograma(...)
//    Cambia el fotograma de Nacho si est� andando o saltando.
//******************************************************************************
void mov_nacho_fotograma(void)
{
 if(dt_anim_nacho.fot)
  {
   if(dt_anim_nacho.fot==1)
    {reproducir_sonido(24,PLAYMODE_PLAY);}
   else if(dt_anim_nacho.fot==8)
    {reproducir_sonido(25,PLAYMODE_PLAY);}
   ism_cambiar_mapa_objeto(movs[0].id,fm_nacho(movs[0].dir,fot_nacho[dt_anim_nacho.fot]));
   dt_anim_nacho.fot=(dt_anim_nacho.fot==16?0:dt_anim_nacho.fot+1);
  }
}

//******************************************************************************
// Funci�n mov_nacho_girar(...)
//    Mueve a Nacho cuando est� girando.
//******************************************************************************
void mov_nacho_girar(char f)
{

 for(f=0;f<5;dt_telebot.ta[f++]=0);

 // Gravedad
 if(ism_cambiar_dato_objeto(movs[0].id, D_Z, movs[0].g/2-1, SUMAR))
  {
   if(movs[0].g<-1) reproducir_sonido(14+rand()%3,PLAYMODE_PLAY);
   if(movs[0].g)
    {
     while(!(ism_cambiar_dato_objeto(movs[0].id, D_Z, -1, SUMAR)));
     movs[0].g=0;
    }

   if(anclar(0)) mov_nacho_explotar(1);

   if(movs[0].mover!=mov_nacho_explotar)
    {
     // Comprobamos el joystick controlador del telebot.
     if(dt_telebot.id[0]!=NO_ID)
       for(f=0;f<5;f++)
         dt_telebot.ta[f]=ism_colisionado_con(dt_telebot.id[f]);
    }
  }
 else
  {
   movs[0].g--;
   movs[0].anclado=NO_ID;
  }

 if(movs[0].mover!=mov_nacho_explotar)
  {
   if(dt_anim_nacho.cont)
    {
     dt_anim_nacho.cont--;
    }
   else
    {
     dt_anim_nacho.cont=2;
     // Cambia el fotograma.
     if(movs[0].dir>7)
      { //Girando en el sentido contrario a las agujas del reloj.
       if(dt_anim_nacho.fot==-1)
        { // Se acaba el giro
         movs[0].mover=mov_nacho_andar;
         movs[0].dir=movs[0].dir-8;
         ism_cambiar_mapa_objeto(movs[0].id,fm_nacho(movs[0].dir,fot_nacho[dt_anim_nacho.fot=0]));
         ism_cambiar_sombra_objeto(movs[0].id,fm_sombra(4));
        }
       else
        {
         ism_cambiar_mapa_objeto(movs[0].id,fm_nacho_giro(movs[0].dir%4,dt_anim_nacho.fot));
         ism_cambiar_sombra_objeto(movs[0].id,fm_nacho_sgiro(dt_anim_nacho.fot));
         if(dt_anim_nacho.fot==1) reproducir_sonido(24,PLAYMODE_PLAY);
         dt_anim_nacho.fot--;
        }
      }
     else
      { //Girando en el sentido de las agujas del reloj.
       if(dt_anim_nacho.fot==3)
        { // Se acaba el giro
         movs[0].mover=mov_nacho_andar;
         movs[0].dir=(movs[0].dir-3)%4;
         ism_cambiar_mapa_objeto(movs[0].id,fm_nacho(movs[0].dir,fot_nacho[dt_anim_nacho.fot=0]));
         ism_cambiar_sombra_objeto(movs[0].id,fm_sombra(4));
        }
       else
        {
         ism_cambiar_mapa_objeto(movs[0].id,fm_nacho_giro(movs[0].dir%4,dt_anim_nacho.fot));
         ism_cambiar_sombra_objeto(movs[0].id,fm_nacho_sgiro(dt_anim_nacho.fot));
         if(dt_anim_nacho.fot==1) reproducir_sonido(25,PLAYMODE_PLAY);
         dt_anim_nacho.fot++;
        }
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_nacho_explotar(...)
//    Secuencia de la explosi�n de Nacho.
//******************************************************************************
void mov_nacho_explotar(char f)
{
 if(f)
  {
   if(movs[0].mover!=mov_nacho_explotar)
    {
     if(movs[0].mover==mov_nacho_girar)
       movs[0].dir%=4;
     else
       dt_anim_nacho.fot=0;
     dt_anim_nacho.cont=-1;
     movs[0].mover=mov_nacho_explotar;
     movs[0].empujable=0;
    }
  }
 else
  {
   dt_anim_nacho.cont++;
   if(dt_anim_nacho.cont<60)
    {
     if(dt_anim_nacho.cont<41 && dt_anim_nacho.cont%10==0)
       reproducir_sonido(1,PLAYMODE_PLAY);
     if(dt_anim_nacho.fot==3)
      {
       movs[0].dir=(movs[0].dir==3?0:movs[0].dir+1);
       dt_anim_nacho.fot=0;
       ism_cambiar_mapa_objeto(movs[0].id,fm_nacho(movs[0].dir,fot_nacho[0]));
       ism_cambiar_sombra_objeto(movs[0].id,fm_sombra(4));
      }
     else
      {
       ism_cambiar_mapa_objeto(movs[0].id,fm_nacho_giro(movs[0].dir,dt_anim_nacho.fot));
        ism_cambiar_sombra_objeto(movs[0].id,fm_nacho_sgiro(dt_anim_nacho.fot));
       dt_anim_nacho.fot++;
      }
     if(dt_anim_nacho.cont>50) ism_cambiar_dato_objeto(movs[0].id,D_TRANSP,10,SUMAR);
    }
   else if(dt_anim_nacho.cont==60)
    {
     ism_quitar_objeto(movs[0].id);
     movs[0].id=NO_ID;
    }
   else if(dt_anim_nacho.cont==120)
    {
     detener_sonidos(1);
     parar_musica();
     for(f=0;f<60;f++) fm_volcar();
     if(!trampas) dt_partida.vidas--;
     num_movs=0;
     if(dt_partida.vidas>=0)
      {
       char cv[2]={0,0};
       if(nuevo_marcador)
        {BITMAP *mpt=create_bitmap(19,10);
         clear_to_color(mpt,makecol(255,0,255));
         cv[0]=LETRA_CERR_0+dt_partida.vidas;
         textout_ex(mpt, (FONT *)(f_fuentes[1].dat), cv, 0, -8, -1, -1);
         blit(fm_pared_msx(0),mapa_vidas,35,72,35,72,19,10);
         set_trans_blender(0,0,0,192);
         draw_trans_sprite(mapa_vidas,mpt,35,72);
         ism_mover_pared(SUP_X,1,0,0);
         destroy_bitmap(mpt);
        }
       else
        {
         clear_bitmap(mapa_vidas);
         cv[0]=LETRA_VDAS_0+dt_partida.vidas;
         textout_ex(mapa_vidas, (FONT *)(f_fuentes[0].dat), cv, 0, 0, -1, -1);
        }
       cargar_hab();
      }
    }
  }
}

//******************************************************************************
// Funci�n mov_empujar(...)
//    Mueve objetos empujables y llaves que hayan sido empujadas.
//******************************************************************************
void mov_empujar(char i, unsigned char dato, int valor)
{
 ism_id id;
 int colision;
 char f,n;

 colision=ism_num_colisiones();
 for(f=n=0;f<num_movs;f++)
   if(movs[f].empujable)
     if(ism_colisionado_con(movs[f].id))
       n++;
 if(n==colision) // �nicamente empujo si todos los objetos con los que he colisionado son empujables.
  {// Buscamos un objeto empujable con el que haya colisionado.
   id=ism_extraer_id_blq();
   for(f=0;f<num_movs;f++)
     if(id==movs[f].id) break;

   if(movs[f].mover==mov_mina_terrestre)
    { // Activo la explosi�n de la mina.
     movs[f].m0=1;
     if(i==0) mov_nacho_explotar(1);
    }
   else if(f!=0 || movs[0].sentmov<2)
    { // A Nacho �nicamente se le empuja si no ha colisionado.
     if(i==0 && movs[f].mover==mov_vida)
       movs[f].m1=1; // Si he chocado contra una vida extra, la activo.
     if(f==0 && (movs[i].mover==mov_rbal_l  || movs[i].mover==mov_saturno ||
                 movs[i].mover==mov_rbal_f  || movs[i].mover==mov_ratamec))
       mov_nacho_explotar(1);
     if(colision=ism_cambiar_dato_objeto(id,dato,valor,SUMAR))
       if(valor==2 || valor==-2)
         if(!(colision=ism_cambiar_dato_objeto(id,dato,valor/=2,SUMAR)))
           valor/=2;
     if(colision)
      {
       mov_empujar(f,dato,valor);
      }
     else
      {// Si tiene algo anclado, lo muevo tambi�n.
       if(sonido_empujar==0)
        {
         reproducir_sonido(7,PLAYMODE_PLAY);
         sonido_empujar=3;
        }
       for(f=0;f<num_movs; f++)
         if(id==movs[f].anclado)
            ism_cambiar_dato_objeto(movs[f].id,dato,valor,SUMAR);
      }
    }
  }
 else if(i==0)
  {// El que empuja es Nacho. Compruebo si todos los objetos con los que ha
   // colisionado son obst�culos.
   for(f=n=0;f<num_obst;f++)
     if(ism_colisionado_con(id_obst[f]))
       n++;
   if(n==colision) mov_nacho_explotar(1);
  }
}

//******************************************************************************
// Funci�n anclar(...)
//    Ancla el objeto indicado. Devuelve 1 si se ancla a un obst�culo.
//******************************************************************************
char anclar(char f)
{
 char n,encontrado=0;
 ism_id id, primer_id=NO_ID, id_a=NO_ID;

 // Busco si hay m�s de un mov_z_autom. Si los hay, anclo con el primero que
 // vaya hacia arriba, o con el �ltimo si todos van hacia abajo.
 for(n=6;n<num_movs;n++)
  {
   if(movs[n].mover==mov_z_autom)
    {
     if(ism_colisionado_con(movs[n].id))
       if(movs[n].dir>0)
        {
         movs[f].anclado=movs[n].id;
         return 0;
        }
       else
         id_a=movs[n].id;
    }
  }
 if(id_a!=NO_ID)
  {
   movs[f].anclado=id_a;
   return 0;
  }

 // Compruebo cada uno de los objetos con los que he colisionado.
 primer_id=id=(ism_colisionado_con(movs[f].anclado)?movs[f].anclado:ism_extraer_id_blq());
 do
  {
   // Busco entre los que tienen funci�n de movilidad.
   for(n=0;n<num_movs;n++)
     if(movs[n].id==id)
      {
       movs[f].anclado=id;
       if(movs[n].mover==mov_mina_terrestre)
        { // Activo la explosi�n de la mina.
         movs[n].m0=1;
         return 1;
        }
       if(f==0)
        {
         if(movs[n].mover==mov_vida)
           movs[n].m1=1; // Si anclo a Nacho con una vida extra, la activo.
         else if(movs[n].mover==mov_cascara || movs[n].mover==mov_rbal_l  ||
                 movs[n].mover==mov_rbal_f  || movs[n].mover==mov_ratamec ||
                 movs[n].mover==mov_saturno || movs[n].mover==mov_mina_voladora)
           return 1; // Si anclo a Nacho con unobst�culo m�vil, lo mato.
        }
       return 0;
      }

   // Si a�n no he encontrado ning�n desconocido, busco entre los obst�culos.
   if(id_a==NO_ID)
    {
     for(n=0;!encontrado && n<num_obst;n++)
       if(id_obst[n]==id) encontrado=1;
     // El objeto es desconocido.
     if(!encontrado) id_a=id;
    }

   if((id=ism_extraer_id_blq())==primer_id)
     id=ism_extraer_id_blq();
  }while(id!=NO_ID);

 // �nicamente he encontrado obst�culos.
 if(id_a==NO_ID)
  {
   movs[f].anclado=primer_id;
   return 1;
  }

 // Se ancla con un desconocido.
 movs[f].anclado=id_a;
 return 0;
}

//******************************************************************************
// Funci�n embolsar(...)
//    Controla El coger/soltar objetos.
//******************************************************************************
void embolsar(void)
{
 int x,y,z;
 char n,f;

 // Colocamos un objeto ficticio para comprobar si colisiona con alguna llave.
 ism_obtener_coords_objeto(movs[0].id,&x,&y,&z);
 ism_colocar_objeto_libre(x-4,y+4,z?z-1:0,23,23,55,NULL,NULL);
 // Miro a ver si ha colisionado con alguna llave
 for(n=-1,f=1;f<3 && n==-1;f++)
   if(movs[f].mover==mov_llave && movs[f].id!=NO_ID && movs[f].empujable)
     if(ism_colisionado_con(movs[f].id))
       n=f;

 if(n>=0)
  {
   movs[n].m1=movs[5].m0; // Cojo un objeto
   reproducir_sonido(17,PLAYMODE_PLAY);
  }
 else
  {
   if(movs[5].m0==-1)
    {
     movs[3].m1=-2; // Ni cojo ni suelto; desplazo los bolsillos.
     if(movs[3].m0>=0 || movs[4].m0>=0 || movs[5].m0>=0) reproducir_sonido(17,PLAYMODE_PLAY);
    }
   else
     if(movs[1].id==NO_ID || movs[2].id==NO_ID) // Hay sitio donde soltar la llave 5.
      {
       if(!ism_cambiar_dato_objeto(movs[0].id,D_Z,ALTURA_BASE,SUMAR))
        {
         movs[movs[1].id==NO_ID?1:2].m1=movs[5].m0;
         reproducir_sonido(17,PLAYMODE_PLAY);
        }
       else
        {// No he podido levantar a Nacho.
         reproducir_sonido(18,PLAYMODE_PLAY);
        }
      }
     else
      {// No hay sitio en la habitaci�n.
       reproducir_sonido(18,PLAYMODE_PLAY);
      }
  }
}

//******************************************************************************
// Funci�n ini_partida()
//    Se inicializa el mapa para empezar una partida.
//******************************************************************************
void ini_partida(void)
{
 unsigned char f,n,i;
 char ph[4]={5,42,90,113};
 char ntipo=(char)(rand()%4);
 char cv[2]={0,0};

 movs[0].dir=0;
 dt_partida.crionautas=0;
 dt_partida.cerraduras=0;
 dt_partida.vidas=4;
 cv[0]=LETRA_CERR_0;
 if(nuevo_marcador)
  {BITMAP *mpt=create_bitmap(19,10);
   clear_to_color(mpt,makecol(255,0,255));
   blit(fm_pared_msx(0),mapa_vidas,0,0,0,0,mapa_vidas->w,mapa_vidas->h);
   blit(fm_pared_msy(0),mapa_cerraduras,0,0,0,0,mapa_cerraduras->w,mapa_cerraduras->h);
   set_trans_blender(0,0,0,192);
   textout_ex(mpt, (FONT *)(f_fuentes[1].dat), cv, 0, -8, -1, -1);
   draw_trans_sprite(mapa_cerraduras,mpt,42,70);
   cv[0]=LETRA_CERR_0+dt_partida.vidas;
   clear_to_color(mpt,makecol(255,0,255));
   textout_ex(mpt, (FONT *)(f_fuentes[1].dat), cv, 0, -8, -1, -1);
   draw_trans_sprite(mapa_vidas,mpt,35,72);
   destroy_bitmap(mpt);
  }
 else
  {
   clear_bitmap(mapa_vidas);
   clear_bitmap(mapa_cerraduras);
   textout_ex(mapa_cerraduras, (FONT *)(f_fuentes[0].dat), cv, 0, 0, -1, -1);
   cv[0]=LETRA_VDAS_0+dt_partida.vidas;
   textout_ex(mapa_vidas, (FONT *)(f_fuentes[0].dat), cv, 0, 0, -1, -1);
  }

 h=ph[rand()%4];

 if(key[KEY_F1]) h=ph[0];
 if(key[KEY_F2]) h=ph[1];
 if(key[KEY_F3]) h=ph[2];
 if(key[KEY_F4]) h=ph[3];

 movs[0].m0=ANCHO_CELDA*5-6;
 movs[0].m1=ANCHO_CELDA*5+7;
 movs[3].m0=movs[4].m0=movs[5].m0=movs[3].m1=movs[4].m1=movs[5].m1=-1;
 pos_a_luz[0]=6*18;
 pos_a_luz[1]=pos_a_luz[2]=pos_a_luz[3]=10*18; // 600'0 a�os luz (una hora, dado que cada a�o luz equivale a seis segundos)
 dec_pos_a_luz=0;
 for(f=0;f<HAB_REPROGRAMANDO;f++) hab[f].banderas = hab[f].banderas & 0x01;

 for(f=0;f<NUM_LLAVES;f++)
  {
   llave[f].tipo=ntipo;
   if((f%9)==8) ntipo=(ntipo==3?0:ntipo+1);
   llave[f].x=(((llave[f].xiyi & 0x70)>>4))*ANCHO_CELDA+3+(llave[f].xiyi & 0x80?ANCHO_CELDA/2:0);
   llave[f].y=( (llave[f].xiyi & 0x07) +1) *ANCHO_CELDA-3+(llave[f].xiyi & 0x08?ANCHO_CELDA/2:0)-1;
   if(hab[llave[f].habi].banderas) // Hab cuadrada
    {
     llave[f].x+=ANCHO_CELDA;
     llave[f].y+=ANCHO_CELDA;
    }
   else if(hab[llave[f].habi].x_p & 0x01) // Pasillo en eje y
    {
     llave[f].y+=ANCHO_CELDA;
    }
   else // Pasillo en eje x
    {
     llave[f].x+=ANCHO_CELDA;
    }
   llave[f].z=llave[f].zi*ALTURA_BASE;
   llave[f].hab=llave[f].habi;
   hab[llave[f].hab].banderas+=2;
  }
 // Se colocan las vidas extras
 i=(unsigned char)((rand()%4)*(NUM_LLAVES/4));
 f=(unsigned char)(rand()%(NUM_LLAVES/4+1));
 n=f+(f&1?-1:+1);
 llave[i+(f==NUM_LLAVES/4?0:f)].tipo=llave[i+(n==NUM_LLAVES/4?0:n)].tipo=4;
}

//******************************************************************************
// Funci�n abrir_cerradura()
//    Secuencia de activaci�n de las c�maras criog�nicas.
//******************************************************************************
void abrir_cerradura(void)
{
 char f;
 char cv[2]={0,0};

 // Quito el holograma de la cerradura.
 for(f=0;f<num_movs;f++)
   if(movs[f].mover==mov_cerradura)
    {
     if(movs[f].id!=NO_ID) ism_quitar_objeto(movs[f].id);
     f=num_movs;
    }

 pausar_sonidos();
 parar_musica();
 reproducir_sonido(21,PLAYMODE_PLAY);

 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 for(f=0;f<8;f++) fm_volcar();

 ism_establecer_oscuridad(0);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=0;
 dibujar_a_luz();
 for(f=0;f<5;f++) fm_volcar();

 ism_establecer_oscuridad(50);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=5;
 dibujar_a_luz();
 for(f=0;f<9;f++) fm_volcar();

 ism_establecer_oscuridad(0);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=0;
 dibujar_a_luz();
 for(f=0;f<5;f++) fm_volcar();

 ism_establecer_oscuridad(50);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=5;
 dibujar_a_luz();
 for(f=0;f<35;f++) fm_volcar();

 ism_establecer_oscuridad(0);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=0;
 dibujar_a_luz();
 for(f=0;f<5;f++) fm_volcar();

 ism_establecer_oscuridad(50);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=5;
 dibujar_a_luz();
 for(f=0;f<39;f++) fm_volcar();

 for(f=6;f<num_movs;f++)
   if(movs[f].mover==mov_astronauta)
    {
     movs[f].m0=0;
     ism_cambiar_mapa_objeto(movs[f].id,fm_astr(movs[f].sentmov,2));
     dt_partida.crionautas++;
    }

 cv[0]=LETRA_CERR_0+dt_partida.cerraduras+1;
 if(nuevo_marcador)
  {BITMAP *mpt=create_bitmap(19,10);
   clear_to_color(mpt,makecol(255,0,255));
   textout_ex(mpt, (FONT *)(f_fuentes[1].dat), cv, 0, -8, -1, -1);
   blit(fm_pared_msy(0),mapa_cerraduras,42,70,42,70,19,10);
   set_trans_blender(0,0,0,192);
   draw_trans_sprite(mapa_cerraduras,mpt,42,70);
   destroy_bitmap(mpt);
  }
 else
  {
   clear_bitmap(mapa_cerraduras);
   textout_ex(mapa_cerraduras, (FONT *)(f_fuentes[0].dat), cv, 0, 0, -1, -1);
  }

 ism_establecer_oscuridad(0);
 ism_dibujar_mundo_isom(buffer,x_org,y_org);
 oscuridad=0;
 dibujar_a_luz();
 for(f=0;f<90;f++) fm_volcar();

 hab[h].banderas+=8;
 if(++dt_partida.cerraduras==24)
   num_movs=0;
 else
  {
   reproducir_sonido(31,-1);
   reanudar_sonidos();
  }
}
