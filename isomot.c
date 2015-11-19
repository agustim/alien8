// Isomot v 1.0
// Motor isométrico para Allegro
// por Ignacio Pérez Gil (15/12/2001 - 17/05/2008)

// isomot.c
// Fichero de código de Isomot

// UN CASADO SE RÍE DEL ADÚLTERO QUE LE PAGA
// EL GOZAR CON SUSTO LO QUE A ÉL LE SOBRA
//
//   Dícenme, don Jerónimo, que dices
// que me pones los cuernos con Ginesa;
// yo digo que me pones casa y mesa;
// y en la mesa, capones y perdices.
//
//   Yo hallo que me pones los tapices
// cuando el calor por el otubre cesa;
// por ti mi bolsa, no mi testa, pesa,
// aunque con molde de oro me la rices.
//
//   Este argumento es fuerte y es agudo:
// tú imaginas ponerme los cuernos; de obra
// yo, porque lo imaginas, te desnudo.
//
//   Más cuerno es el que paga que el que cobra;
// ergo, aquel que me paga, es el cornudo,
// lo que de mi mujer a mí me sobra.
//
//                     D. Francisco de Quevedo y Villegas

#include <stdlib.h>
#include <allegro.h>
#include "isomot.h"

#if (ALLEGRO_VERSION<4) || \
    (ALLEGRO_VERSION==4 && ALLEGRO_SUB_VERSION<1) || \
    (ALLEGRO_VERSION==4 && ALLEGRO_SUB_VERSION==1 && ALLEGRO_WIP_VERSION<13)
#define set_clip_rect set_clip
#endif

#define PRIMER_ID_L 7
#define PRIMER_ID_R 8
#define ULT_ID_L 0xffffffff
#define ULT_ID_R 0xfffffffe
#define fm_error(_a_)     {error=_a_;return -1;}
#define fm_id_error(_a_)  {error=_a_;return ID_ERROR;}
#define fm_compr_ini()    {error=0;if(no_inic){error=1;return -1;}}
#define fm_id_compr_ini() {error=0;if(no_inic){error=1;return ID_ERROR;}}
#ifdef ISM_ESP
#ifdef ISM_ENG
#define fm_funcion(_TIPO_,_FESP_,_FING_,_DPARS_,_NPARS_) _TIPO_ _FING_ _DPARS_ {return _FESP_ _NPARS_;} _TIPO_ _FESP_ _DPARS_
#else
#define fm_funcion(_TIPO_,_FESP_,_FING_,_DPARS_,_NPARS_) _TIPO_ _FESP_ _DPARS_
#endif
#else
#define fm_funcion(_TIPO_,_FESP_,_FING_,_DPARS_,_NPARS_) _TIPO_ _FING_ _DPARS_
#define SUP_X           HIGH_X
#define INF_X           LOW_X
#define SUP_Y           HIGH_Y
#define INF_Y           LOW_Y
#define TODAS           ALL
#define ENCIMA          TOP
#define ID_PARED_SUP_X  ID_HIGH_X_WALL
#define ID_PARED_SUP_Y  ID_HIGH_Y_WALL
#define ID_PARED_INF_X  ID_LOW_X_WALL
#define ID_PARED_INF_Y  ID_LOW_Y_WALL
#define ID_SUELO        ID_FLOOR
#define D_ALTURA        D_HEIGHT
#define D_ANCHURA_X     D_WIDTH_X
#define D_ANCHURA_Y     D_WIDTH_Y
#define SUMAR           ADD
#define ism_vaciar_mundo_isom      ism_empty_isom_world
#define ism_obtener_coords_celda   ism_get_cell_coords
#define ism_obtener_coords_libres  ism_get_free_coords
#endif

// Tipos de datos
struct infomapa {BITMAP *mapa;
                 int     despx;
                 int     despy;};

typedef struct _objeto_l {ism_id id;
                          int x;
                          int y;
                          int z;
                          int anchurax;
                          int anchuray;
                          int altura;
                          char enmascarar;
                          char sombrear;
                          char transp;
                          BITMAP *mapabase;
                          BITMAP *mapasomb;
                          BITMAP *sombra;
                          struct infomapa inf_mapa;
                          struct _objeto_l *ant;
                          struct _objeto_l *sig;} objeto_l;

typedef struct _objeto_r {ism_id id;
                          int z;
                          int altura;
                          BITMAP *mapabase;
                          char sombrear;
                          struct infomapa inf_mapa;
                          BITMAP *sombra;
                          struct _objeto_r *sig;} objeto_r;

typedef struct _elem_blq {ism_id id;
                          struct _elem_blq *sig;} elem_blq;

typedef struct _columna {BITMAP *mapa;
                         BITMAP *mapasomb;
                         int despx;
                         int despy;
                         char sombrear;
                         struct _objeto_r *objr;} columna;

struct _rejilla {int x;
                 int y;
                 int anchura;} rejilla={1,1,1};

struct {
  unsigned char tptransp; // % de la transparencia
  unsigned int ntransp;   // nº de objetos con esta transparencia
 }tb_transp[101];

struct {
  short pctj_oscuridad;   // Oscuridad de la habitación para 24 y 32 bits
  char pctj_oscuridad_32; // Oscuridad de la habitación para 15 y 16 bits
 }dt_oscluz;

#define RECT_DESC     0
#define RECT_NO_DIB   1
#define RECT_SI_DIB   2
#define RECT_DIB_TODO 3
struct {
  int x0,y0,x1,y1; // Esquinas del rectángulo sucio
  char modo;       // Modo del rectángulo sucio
 }dt_rect={0,0,0,0,RECT_DESC};

// Variables globales
int x0ant=0;
int y0ant=0;
char error=0;
char no_inic=1;
columna *suelo=NULL;
int *indices=NULL;
struct infomapa *paredx=NULL;
struct infomapa *paredy=NULL;
ism_id sig_id_r;
ism_id sig_id_l;
objeto_l *lista_objl=NULL;
objeto_l *ult_objl=NULL;
objeto_r *ult_objr=NULL;
int ult_objr_x;
int ult_objr_y;
elem_blq *pila_blq=NULL;
elem_blq *primer_blq=NULL;
void(*func_transp)(BITMAP *, BITMAP *, int, int, int)=NULL;
short pctj_sombreado=128;
char pctj_sombreado_32=16;

// Declaración de las funciones privadas del motor
int prv_modificar_pared(BITMAP *mapa, unsigned char eje, int pared, int despeje, int despaltura, short colocar_mapa);
int prv_modificar_loseta(BITMAP *mapa, int x, int y, int despx, int despy, short colocar_mapa);
int prv_mover_objeto_libre(ism_id id, int valor, int x, int y, int z, unsigned char dato, unsigned char modo);
void prv_enmascarar(objeto_l *objl, BITMAP *mapa, int xm, int ym);
void prv_sombrear_loseta(int loseta, int x, int y, BITMAP *sombra, char transp);
void prv_sombrear_objeto(objeto_r *objr, objeto_l *objl, int x, int y, BITMAP *sombra, char transp);
void prv_oscurecer_hab(BITMAP *mapa, int x0, int y0, int x1, int y1);
char prv_apilar_objeto_blq(ism_id id);
void prv_vaciar_pila_blq(void);
int prv_cambiar_transparencia_objeto(ism_id id, int valor, unsigned char modo);
void prv_introducir_transparencia(char);
void prv_quitar_transparencia(char);
void prv_ordenar_objeto(objeto_l *objl);
#define fm_incorporar_cuadricula(_M_,_X_,_Y_) if(_M_ && dt_rect.modo!=RECT_DESC && dt_rect.modo!=RECT_DIB_TODO) prv_incorporar_cuadricula(_X_,_Y_,_X_+_M_->w-1,_Y_+_M_->h-1)
void prv_incorporar_cuadricula(int x0, int y0, int x1, int y1);

// Implementación de las funciones públicas

//*********************************************************************
// Establece los valores de la rejilla y le reserva o reasigna memoria.
//*********************************************************************
fm_funcion(int, ism_establecer_rejilla, ism_set_grid, (int x, int y, int anchura), (x, y, anchura))   
{
 void *tmp;
 int f;
 
 error=0;
 if (x<1||y<1) fm_error(2);
 if (anchura<2) fm_error(5);

 if (no_inic) 
  {
   tmp=malloc(2*x*sizeof(struct infomapa));
   if(tmp)
     paredx=(struct infomapa *)tmp;
   else 
     fm_error(3); 

   tmp=malloc(2*y*sizeof(struct infomapa));
   if(tmp)
     paredy=(struct infomapa *)tmp;
   else 
     fm_error(3); 

   tmp=malloc((x*y+1)*sizeof(columna));
   if(tmp)
     suelo=(columna *)tmp;
   else
     fm_error(3); 

   for(f=0;f<x*y+1;f++)
    {
     suelo[f].objr=NULL;
     suelo[f].mapasomb=NULL;
    } 

   tmp=malloc(x*y*sizeof(int));
   if(tmp)
     indices=(int *)tmp;
   else
     fm_error(3); 
 
  }
 else 
  {
   if(rejilla.x!=x)
    {
     tmp=realloc(paredx,2*x*sizeof(struct infomapa));
     if(tmp)
       paredx=(struct infomapa *)tmp;
     else 
       fm_error(3); 
    }

   if(rejilla.y!=y)
    {
     tmp=realloc(paredy,2*y*sizeof(struct infomapa));
     if(tmp)
       paredy=(struct infomapa *)tmp;
     else 
       fm_error(3); 
    }   
 
   if((rejilla.x*rejilla.y)!=(x*y))
    {
     for(f=0;f<rejilla.x*rejilla.y+1;f++)
      {
       if(suelo[f].mapasomb)
        {
         destroy_bitmap(suelo[f].mapasomb); 
         suelo[f].mapasomb=NULL;
        } 
       while(suelo[f].objr)
        {objeto_r *objr=suelo[f].objr;
         suelo[f].objr=suelo[f].objr->sig;
         if(objr->inf_mapa.mapa)
           destroy_bitmap(objr->inf_mapa.mapa);
         free(objr);
        }
      }  

     tmp=realloc(suelo,(x*y+1)*sizeof(columna));
     if(tmp)
       suelo=(columna *)tmp;
     else
       fm_error(3); 

     if((x+y)>(rejilla.x+rejilla.y))
       for(f=rejilla.x*rejilla.y+1;f<x*y+1;f++)
        {
         suelo[f].mapasomb=NULL;
         suelo[f].objr=NULL;
        } 
       
     tmp=realloc(indices,x*y*sizeof(int));
     if(tmp)
       indices=(int *)tmp;
     else
       fm_error(3); 
    }   
  }

 if((rejilla.x!=x)||(rejilla.y!=y)||no_inic)
  {int ind=0; 
   int n;
   for(f=0;f<=(x+y-1);f++)
     for(n=y-1;n>=0;n--)
      {int tmp=n*(x-1)+f;
       if((tmp>=n*x)&&(tmp<(n+1)*x)) indices[ind++]=tmp;
       }
   }      

 no_inic=0;
 rejilla.x=x;
 rejilla.y=y;
 rejilla.anchura=anchura;
 // Llamada para inicializar los valores.
 ism_vaciar_mundo_isom();

 return 0;
}

//****************************************
// Vacía por completo el mundo isométrico.
//****************************************
fm_funcion(int, ism_vaciar_mundo_isom, ism_empty_isom_world, (void), ())
{
 objeto_r *objr=NULL;
 int f,n;

 fm_compr_ini();

 // Desasignamos las paredes del eje X, las losetas y los objetos rejilla
 for(f=0;f<rejilla.x;f++)
  {
   paredx[f].mapa=paredx[f+rejilla.x].mapa=NULL;
   paredx[f].despx=paredx[f].despy=paredx[f+rejilla.x].despx=paredx[f+rejilla.x].despy=0;
   for(n=0;n<rejilla.y;n++)
    {
     suelo[rejilla.x*n+f].mapa=NULL;
     suelo[rejilla.x*n+f].despx=suelo[rejilla.x*n+f].despy=suelo[rejilla.x*n+f].sombrear=0;
     if(suelo[rejilla.x*n+f].mapasomb)
      {
       destroy_bitmap(suelo[rejilla.x*n+f].mapasomb);
       suelo[rejilla.x*n+f].mapasomb=NULL;
      }
     while(objr=suelo[rejilla.x*n+f].objr)
      {
       suelo[rejilla.x*n+f].objr=suelo[rejilla.x*n+f].objr->sig;
       if(objr->inf_mapa.mapa)
         destroy_bitmap(objr->inf_mapa.mapa);
       free(objr);
      }
    } 
  } 

 suelo[f=rejilla.x*rejilla.y].mapa=NULL;
 suelo[f].sombrear=suelo[f].despx=suelo[f].despy=0;
 if(suelo[f].mapasomb)
  {
   destroy_bitmap(suelo[f].mapasomb);
   suelo[f].mapasomb=NULL;
  }
  
 // Desasignamos las paredes del eje Y
 for(f=0;f<rejilla.y;f++)
  {
   paredy[f].mapa=paredy[f+rejilla.y].mapa=NULL;
   paredy[f].despx=paredy[f].despy=paredy[f+rejilla.y].despx=paredy[f+rejilla.y].despy=0;
  } 

 // Borramos la lista de objetos libres
 while(lista_objl)
  {objeto_l *objl=lista_objl;
   lista_objl=lista_objl->sig;
   if(objl->inf_mapa.mapa)
     destroy_bitmap(objl->inf_mapa.mapa);
   if(objl->mapasomb)
     destroy_bitmap(objl->mapasomb);
   free(objl);
  } 

 // Se inicializa la tabla de transparencias
 for(f=0;f<101;f++)
  {
   tb_transp[f].tptransp=100;
   tb_transp[f].ntransp=0;
  }

 dt_oscluz.pctj_oscuridad=256;
 dt_oscluz.pctj_oscuridad_32=32;
 if(dt_rect.modo!=RECT_DESC) dt_rect.modo=RECT_DIB_TODO;
 sig_id_l=PRIMER_ID_L;
 sig_id_r=PRIMER_ID_R;
 prv_vaciar_pila_blq();
 ult_objr=NULL;
 ult_objl=NULL;

 return 0;
}

//**************************************************************************
// Borra el mundo isométrico y libera toda la memoria reservada por el motor
//**************************************************************************
fm_funcion(int, ism_destruir_todo, ism_destroy_all, (void), ())
{
 fm_compr_ini();
 ism_vaciar_mundo_isom();
 free(paredx);
 free(paredy);
 free(suelo);
 free(indices);
 paredx=NULL;
 paredy=NULL;
 suelo=NULL;
 indices=NULL;
 no_inic=1;
 return 0;
}

//********************************************
// Dibuja el mundo isométrico en el mapa dado.
//********************************************
fm_funcion(int, ism_dibujar_mundo_isom, ism_draw_isom_world, (BITMAP *mapa_destino, int x0, int y0), (mapa_destino, x0, y0))
{
 objeto_l *objl;
 int f,n;
 int fs,ns;
 int ls;
 char desactivar_clip=0;
 int ant_cl, ant_ct, ant_cr, ant_cb;

 fm_compr_ini();
 if(!mapa_destino) fm_error(15);

 if(!dt_oscluz.pctj_oscuridad)
  { // Si la oscuridad está al 100% y no hay luces...
   clear_bitmap(mapa_destino);
   return 0;
  }
 if(!mapa_destino->clip)
  { // Así se evita el error de memoria si se trata de dibujar fuera de mapa_destino
   desactivar_clip=1;
   set_clip_rect(mapa_destino,0,0,mapa_destino->w-1,mapa_destino->h-1);
  }
 if(((x0!=x0ant)||(y0!=y0ant)) && dt_rect.modo!=RECT_DESC) dt_rect.modo=RECT_DIB_TODO;
 if(dt_rect.modo!=RECT_DESC && dt_rect.modo!=RECT_DIB_TODO)
  {
   if(dt_rect.modo==RECT_NO_DIB)
    { // No hay que dibujar nada
     if(desactivar_clip) set_clip_rect(mapa_destino,0,0,0,0);
     return 0;
    }
   if(dt_rect.modo==RECT_SI_DIB)
    {
     if(dt_rect.x0<mapa_destino->cl) dt_rect.x0=mapa_destino->cl;
     if(dt_rect.y0<mapa_destino->ct) dt_rect.y0=mapa_destino->ct;
     if(dt_rect.x1>=mapa_destino->cr) dt_rect.x1=mapa_destino->cr-1;
     if(dt_rect.y1>=mapa_destino->cb) dt_rect.y1=mapa_destino->cb-1;
     if(dt_rect.x0>dt_rect.x1 || dt_rect.y0>dt_rect.y1)
      { // El área a dibujar queda fuera del área de recorte del mapa de destino.
       if(desactivar_clip) set_clip_rect(mapa_destino,0,0,0,0);
       return 0;
      }
     else
      {
       ant_cl=mapa_destino->cl;
       ant_ct=mapa_destino->ct;
       ant_cr=mapa_destino->cr-1;
       ant_cb=mapa_destino->cb-1;
       set_clip_rect(mapa_destino,dt_rect.x0,dt_rect.y0,dt_rect.x1,dt_rect.y1);
      }
    }
  }

 // Se comprueba si se dibuja en un punto diferente a la vez anterior
 if((x0!=x0ant)||(y0!=y0ant))
  {int nuedx=x0-x0ant;
   int nuedy=y0-y0ant;
 
   for(f=0;f<(rejilla.x<<1);f++)
    {
     paredx[f].despx+=nuedx;
     paredx[f].despy+=nuedy;
    }

   for(f=0;f<(rejilla.y<<1);f++)
    {
     paredy[f].despx+=nuedx;
     paredy[f].despy+=nuedy;
    }

   for(f=0;f<=rejilla.x*rejilla.y;f++)
    {
     suelo[f].despx+=nuedx;
     suelo[f].despy+=nuedy;
    }

   x0ant=x0;
   y0ant=y0;
  }
 
 if(is_video_bitmap(mapa_destino)) acquire_bitmap(mapa_destino);
 // Dibujamos las losetas
 ls=rejilla.x*rejilla.y;
 if(suelo[ls].sombrear && suelo[ls].mapa)
  {
   if(pctj_sombreado<256) // Las sombras están activas
    {objeto_r *objr=NULL;
     // Sombreamos con los objetos rejilla
     for(f=0;f<rejilla.x;f++)
       for(n=0;n<rejilla.y;n++)
        {
         objr=suelo[rejilla.x*n+f].objr;
         while(objr)
          {
           if(objr->sombra)
             prv_sombrear_loseta(ls,2*rejilla.anchura*(f-n)-(objr->sombra->w)/2+x0+1,
                                 rejilla.anchura*(f+n+1)-(objr->sombra->h)/2+y0-1,objr->sombra,0);
           objr=objr->sig;
          }
        }
     // Sombreamos con los objetos libres
     fs=0;
     while(tb_transp[fs].tptransp<100)
      {
       objl=lista_objl;
       ns=tb_transp[fs].ntransp;
       while(ns)
        {
         if(objl->transp==tb_transp[fs].tptransp)
          {
           if(objl->sombra)
             prv_sombrear_loseta(ls,2*(objl->x-objl->y)+x0+(objl->anchurax+objl->anchuray)-((objl->sombra->w)>>1)-1,
                                objl->x+objl->y+y0+((objl->anchurax-objl->anchuray+1)>>1)-((objl->sombra->h)>>1),
                                objl->sombra,objl->transp);
           ns--;
          }
         objl=objl->sig;
        }
       fs++;
      }
    }

   if(suelo[ls].sombrear==1 && suelo[ls].mapasomb)
    {// No se ha sombreado, así que destruimos el mapa de sombreado
     destroy_bitmap(suelo[ls].mapasomb);
     suelo[ls].mapasomb=NULL;
    }
   suelo[ls].sombrear=0;
  }

 if(suelo[ls].mapasomb)
   draw_sprite(mapa_destino, suelo[ls].mapasomb, suelo[ls].despx, suelo[ls].despy);
 else if(suelo[ls].mapa)
   draw_sprite(mapa_destino, suelo[ls].mapa, suelo[ls].despx, suelo[ls].despy);

 for(f=0;f<rejilla.x;f++)
   for(n=0;n<rejilla.y;n++)
    {int indlst=rejilla.x*n+f;
     if(suelo[indlst].sombrear)
      { // Hay que sombrear la loseta
       if(pctj_sombreado<256) // Las sombras están activas
        {objeto_r *objr=suelo[indlst].objr;
         // Sombreamos con los objetos rejilla
         while(objr)
          {
           if(objr->sombra && suelo[indlst].mapa)
             prv_sombrear_loseta(indlst,
                2*rejilla.anchura*(f-n)-(objr->sombra->w)/2+x0+1,
                rejilla.anchura*(f+n+1)-(objr->sombra->h)/2+y0-1, objr->sombra,0);
           objr=objr->sig;
          }
         // Sombreamos con los objetos libres
         fs=0;
         while(tb_transp[fs].tptransp<100)
          {
           objl=lista_objl;
           ns=tb_transp[fs].ntransp;
           while(ns)
            {
             if(objl->transp==tb_transp[fs].tptransp)
              {
               int xini=objl->x/rejilla.anchura;                    //
               int xfin=(objl->x+objl->anchurax-1)/rejilla.anchura; // Rango de columnas
               int yini=(objl->y-objl->anchuray+1)/rejilla.anchura; // intersectadas por el objeto
               int yfin=objl->y/rejilla.anchura;                    //
               if(objl->sombra && f>=xini && f<=xfin && n>=yini && n<=yfin && suelo[indlst].mapa)
                 prv_sombrear_loseta(indlst,
                    2*(objl->x-objl->y)+x0+(objl->anchurax+objl->anchuray)-((objl->sombra->w)>>1)-1,
                    objl->x+objl->y+y0+((objl->anchurax-objl->anchuray+1)>>1)-((objl->sombra->h)>>1),
                    objl->sombra,objl->transp);
               ns--;
              }
             objl=objl->sig;
            }
           fs++;
          }
        }
       if(suelo[indlst].sombrear==1 && suelo[indlst].mapasomb)
        {// No se ha sombreado, así que destruimos el mapa sombreado
         destroy_bitmap(suelo[indlst].mapasomb);
         suelo[indlst].mapasomb=NULL;
        }
       suelo[indlst].sombrear=0;
      }
     if(suelo[indlst].mapasomb)
       draw_sprite(mapa_destino, suelo[indlst].mapasomb, suelo[indlst].despx, suelo[indlst].despy);
     else if(suelo[indlst].mapa)
         draw_sprite(mapa_destino, suelo[indlst].mapa, suelo[indlst].despx, suelo[indlst].despy);
    }                 

 // Dibujamos las paredes superiores del eje X
 for(f=0;f<rejilla.x;f++)
   if (paredx[f].mapa)
     draw_sprite(mapa_destino,paredx[f].mapa, paredx[f].despx, paredx[f].despy);

 // Dibujamos las paredes superiores del eje Y
 for(n=0;n<rejilla.y;n++)
   if (paredy[n].mapa)
     draw_sprite(mapa_destino,paredy[n].mapa, paredy[n].despx, paredy[n].despy);

 // Dibujamos los objetos rejilla
 for(f=0;f<rejilla.x*rejilla.y;f++)
  {objeto_r *objr=suelo[indices[f]].objr;

   while(objr)
    {
     if(objr->sombrear)
      { // Hay que sombrear el objeto
       if(pctj_sombreado<256 && objr->mapabase) // Las sombras están activas, y el objeto tiene un mapa
        {objeto_r *objs=objr->sig;
         // Sombreamos con los objetos rejilla
         while(objs)
          {
           if(objs->sombra)
             prv_sombrear_objeto(objr, NULL,
                  objr->inf_mapa.despx+(objr->mapabase->w-objs->sombra->w)/2,
                  objr->inf_mapa.despy+objr->mapabase->h-rejilla.anchura-objr->altura-(objs->sombra->h)/2,
                  objs->sombra,0);
           objs=objs->sig;
          }
         // Sombreamos con los objetos libres 
         fs=0;
         while(tb_transp[fs].tptransp<100)
          {
           objl=lista_objl;
           ns=tb_transp[fs].ntransp;
           while(ns)
            {
             if(objl->transp==tb_transp[fs].tptransp)
              {
               int xini=objl->x/rejilla.anchura;                    //
               int xfin=(objl->x+objl->anchurax-1)/rejilla.anchura; // Rango de columnas
               int yini=(objl->y-objl->anchuray+1)/rejilla.anchura; // intersectadas por el objeto
               int yfin=objl->y/rejilla.anchura;                    //
               if(objl->sombra && objl->z>objr->z &&
                  (indices[f]%rejilla.x)>=xini && (indices[f]%rejilla.x)<=xfin &&
                  (indices[f]/rejilla.x)>=yini && (indices[f]/rejilla.x)<=yfin)
                {
                 prv_sombrear_objeto(objr, NULL,
                      2*(objl->x-objl->y)+(objl->anchurax+objl->anchuray)-((objl->sombra->w)>>1)-1,
                      objl->x+objl->y+((objl->anchurax-objl->anchuray+1)>>1)-((objl->sombra->h)>>1)-objr->z-objr->altura,
                                       objl->sombra,objl->transp);
                 }
               ns--;
              }
             objl=objl->sig;
            }
           fs++;
          }
        }
       if(objr->sombrear==1 && objr->inf_mapa.mapa)
        {// No se ha sombreado, así que destruimos el mapa sombreado
         destroy_bitmap(objr->inf_mapa.mapa);
         objr->inf_mapa.mapa=NULL;
        }
       objr->sombrear=0; 
      }
     if(objr->inf_mapa.mapa)
       draw_sprite(mapa_destino, objr->inf_mapa.mapa,
                     x0+objr->inf_mapa.despx,
                     y0+objr->inf_mapa.despy);
     else if(objr->mapabase)
       draw_sprite(mapa_destino,objr->mapabase,
                     x0+objr->inf_mapa.despx,
                     y0+objr->inf_mapa.despy);
     objr=objr->sig;
    }
  }

 // Se sombrean los objetos libres
 objl=lista_objl;
 while(objl)
  {
   if(pctj_sombreado<256 && objl->sombrear && objl->mapabase)
    {
     objeto_l *objm;
     int xini=objl->x/rejilla.anchura;                    //
     int xfin=(objl->x+objl->anchurax-1)/rejilla.anchura; // Rango de columnas
     int yini=(objl->y-objl->anchuray+1)/rejilla.anchura; // intersectadas por el objeto
     int yfin=objl->y/rejilla.anchura;                    //

     // Se sombrea con los rejilla
     for(f=yini;f<=yfin;f++)
       for(n=xini;n<=xfin;n++)
        {objeto_r *objr=suelo[f*rejilla.x+n].objr;
         while(objr)
          {
           if(objr->sombra && objr->z>objl->z)
             prv_sombrear_objeto(NULL, objl,
                   2*rejilla.anchura*(n-f)-(objr->sombra->w)/2+1,
                   rejilla.anchura*(n+f+1)-objl->z-objl->altura-(objr->sombra->h)/2-1,
                   objr->sombra,0);
           objr=objr->sig;
          }
        }

     // Se sombrea con los libres
     fs=0;
     while(tb_transp[fs].tptransp<100)
      {
       objm=lista_objl;
       ns=tb_transp[fs].ntransp;
       while(ns)
        {
         if(objm->transp==tb_transp[fs].tptransp)
          {
            if(objl->id!=objm->id && objm->sombra && objl->z<=objm->z &&
               objl->x<objm->x+objm->anchurax && objm->x<objl->x+objl->anchurax &&
               objl->y>objm->y-objm->anchuray && objm->y>objl->y-objl->anchuray)
             prv_sombrear_objeto(NULL, objl, 2*(objm->x-objm->y)+objm->anchurax+objm->anchuray-objm->sombra->w/2-1,
                                 objm->x+objm->y-objl->z-objl->altura+(objm->anchurax-objm->anchuray-objm->sombra->h)/2,
                                 objm->sombra,objm->transp);
           ns--;
          }
         objm=objm->sig;
        }
       fs++;
      }
    }     
    
   if(objl->sombrear==2)
     objl->enmascarar=1;

   if(objl->sombrear==1 && objl->mapasomb)
    {
     destroy_bitmap(objl->mapasomb);
     objl->mapasomb = NULL;
     objl->enmascarar=1;
    } 
   objl=objl->sig;
  }
 
 // Se enmascaran y dibujan los objetos libres
 objl=lista_objl;
 while(objl)
  {
   if(objl->mapabase)
    {int x1,y1,x2,y2;
     objeto_l *objm=objl->sig;

     // Se compara objl con los siguientes de la lista para comprobar las ocultaciones
     while(objm)
      {
       if(objm->mapabase) // El otro tiene un mapa asignado
       if((objm->inf_mapa.despx < objl->inf_mapa.despx+objl->mapabase->w) &&
          (objm->inf_mapa.despx+objm->mapabase->w > objl->inf_mapa.despx) &&
          (objm->inf_mapa.despy < objl->inf_mapa.despy+objl->mapabase->h) &&
          (objm->inf_mapa.despy+objm->mapabase->h > objl->inf_mapa.despy)) // Sus mapas se solapan
       if((objl->x+objl->anchurax <= objm->x) ||
          (objl->y <= objm->y-objm->anchuray) ||
          (objl->z+objl->altura <= objm->z)) // objl está detrás de objm
        {
         if(objl->enmascarar && objm->transp==0)
           prv_enmascarar(objl,objm->mapabase,objm->inf_mapa.despx,objm->inf_mapa.despy);
        } 
       else
        {
         if(objm->enmascarar && objl->transp==0)
           prv_enmascarar(objm,objl->mapabase,objl->inf_mapa.despx,objl->inf_mapa.despy);
        } 
       objm=objm->sig;
      }

     // Se compara objl con los rejilla que tiene debajo
     ism_obtener_coords_celda(objl->x,objl->y,&x1,&y1);
     ism_obtener_coords_celda(objl->x+objl->anchurax-1,objl->y-objl->anchuray+1,&x2,&y2);
     y1++;
     x2++;
     if(objl->enmascarar)
       do
        {int i=0;
         while( ((x1+i)<rejilla.x) && ((y1+i)<rejilla.y) )
          {objeto_r *objr=suelo[rejilla.x*(y1+i)+x1+i].objr;
           while(objr)
            {
             if(objr->mapabase)
             if((objr->inf_mapa.despx < objl->inf_mapa.despx+objl->mapabase->w)      &&
                (objr->inf_mapa.despx+objr->mapabase->w > objl->inf_mapa.despx) &&
                (objr->inf_mapa.despy < objl->inf_mapa.despy+objl->mapabase->h)      &&
                (objr->inf_mapa.despy+objr->mapabase->h > objl->inf_mapa.despy)) // Sus mapas se solapan
              {int x,y;
               ism_obtener_coords_libres(x1+i,y1+i,&x,&y);
               if((objl->x+objl->anchurax <= x)  ||
                  (objl->y <= y-rejilla.anchura) ||
                  (objl->z+objl->altura <= objr->z)) // objl está detrás de objr
                 prv_enmascarar(objl,objr->mapabase,objr->inf_mapa.despx,objr->inf_mapa.despy);
              }   
             objr=objr->sig;
            }
           i++;
          }
  
         if(y1!=y2)
           y1--;
         else
           x1++;
  
        }while(x1<=x2);

     if(objl->enmascarar==1 && objl->inf_mapa.mapa)
      {
       destroy_bitmap(objl->inf_mapa.mapa);
       objl->inf_mapa.mapa = NULL;
      } 
      
     // Se dibuja objl
     if(objl->transp)
      {
       if(objl->transp<100)
        {
         if(func_transp)
         func_transp((objl->inf_mapa.mapa?objl->inf_mapa.mapa:(objl->mapasomb?objl->mapasomb:objl->mapabase)),
                       mapa_destino,x0+objl->inf_mapa.despx,y0+objl->inf_mapa.despy,(int)(256-2.56*objl->transp));
         else
          {
           set_trans_blender(0,0,0,(int)(256-2.56*objl->transp));
           draw_trans_sprite(mapa_destino,(objl->inf_mapa.mapa?objl->inf_mapa.mapa:(objl->mapasomb?objl->mapasomb:objl->mapabase)),
                             x0+objl->inf_mapa.despx,y0+objl->inf_mapa.despy);
          }
        }
      }
     else
       draw_sprite(mapa_destino,(objl->inf_mapa.mapa?objl->inf_mapa.mapa:(objl->mapasomb?objl->mapasomb:objl->mapabase)),
                     x0+objl->inf_mapa.despx,y0+objl->inf_mapa.despy);
                 
     objl->sombrear=objl->enmascarar=0;
    }
   objl=objl->sig;
  }

 // Se dibujan las paredes inferiores del eje X
 for(f=rejilla.x;f<(rejilla.x<<1);f++)
   if (paredx[f].mapa)
     draw_sprite(mapa_destino,paredx[f].mapa, paredx[f].despx, paredx[f].despy);

 // Se dibujan las paredes inferiores del eje Y
 for(n=rejilla.y;n<(rejilla.y<<1);n++)
   if (paredy[n].mapa)
     draw_sprite(mapa_destino,paredy[n].mapa, paredy[n].despx, paredy[n].despy);

 if(dt_oscluz.pctj_oscuridad<256)
  {
   //Oscurecer el mapa de destino
   prv_oscurecer_hab(mapa_destino, mapa_destino->cl, mapa_destino->ct, mapa_destino->cr-1, mapa_destino->cb-1);
  }
 if(dt_rect.modo==RECT_DIB_TODO) dt_rect.modo=RECT_NO_DIB;
 if(dt_rect.modo==RECT_SI_DIB)
  {
   dt_rect.modo=RECT_NO_DIB;
   set_clip_rect(mapa_destino,ant_cl,ant_ct,ant_cr,ant_cb);
  }
 if(desactivar_clip)
   set_clip_rect(mapa_destino,0,0,0,0);
 if(is_video_bitmap(mapa_destino)) release_bitmap(mapa_destino);

 return 0;     
}

//***********************************************************
// Coloca un mapa en una o todas las paredes del eje indicado
//***********************************************************
fm_funcion(int, ism_colocar_pared, ism_set_wall, (BITMAP *mapa, unsigned char eje, int pared), (mapa, eje, pared))
{
 return prv_modificar_pared(mapa, eje, pared, 0, 0, 1);
} 

//**************************************
// Cambia el desplazamiento de una pared
//**************************************
fm_funcion(int, ism_mover_pared, ism_move_wall, (unsigned char eje, int pared, int despeje, int despaltura), (eje, pared, despeje, despaltura))
{
 return prv_modificar_pared(NULL, eje, pared, despeje, despaltura, 0);
}

//***************************************
// Coloca un mapa en una o varias losetas
//***************************************
fm_funcion(int, ism_colocar_loseta, ism_set_floor_tile, (BITMAP *mapa, int x, int y), (mapa, x, y))
{
 return prv_modificar_loseta(mapa, x, y, 0, 0, 1); 
}

//***************************
// Coloca un mapa en el suelo
//***************************
fm_funcion(int, ism_colocar_suelo, ism_set_floor, (BITMAP *mapa), (mapa))
{
 int lst=rejilla.x*rejilla.y;

 fm_compr_ini();
 fm_incorporar_cuadricula(suelo[lst].mapa,suelo[lst].despx,suelo[lst].despy);
 if(suelo[lst].mapa=mapa)
  {
   suelo[lst].despx=x0ant+(rejilla.x-rejilla.y)*rejilla.anchura-mapa->w/2+1;
   suelo[lst].despy=y0ant+((rejilla.x+rejilla.y)*rejilla.anchura-1-mapa->h)/2;
   suelo[lst].sombrear=1;
  }
 else
   suelo[lst].sombrear=0;

 fm_incorporar_cuadricula(suelo[lst].mapa,suelo[lst].despx,suelo[lst].despy);

 if(suelo[lst].mapasomb)
  {
   destroy_bitmap(suelo[lst].mapasomb);
   suelo[lst].mapasomb=NULL;
  }
 return 0;
}

//***************************************
// Cambia el desplazamiento de una loseta
//***************************************
fm_funcion(int, ism_mover_loseta, ism_move_floor_tile, (int x, int y, int despx, int despy), (x, y, despx, despy))
{
 return prv_modificar_loseta(NULL, x, y, despx, despy, 0);
}

//********************************************
// Cambia el desplazamiento del mapa del suelo
//********************************************
fm_funcion(int, ism_mover_suelo, ism_move_floor, (int despx, int despy), (despx, despy))
{
 int lst;

 fm_compr_ini();

 if(suelo[lst=rejilla.x*rejilla.y].mapa)
  {
   fm_incorporar_cuadricula(suelo[lst].mapa,suelo[lst].despx,suelo[lst].despy);
   suelo[lst].despx+=2*(despy-despx);
   suelo[lst].despy+=despx+despy;
   fm_incorporar_cuadricula(suelo[lst].mapa,suelo[lst].despx,suelo[lst].despy);
   if(pctj_sombreado<256)
     suelo[lst].sombrear=1;
  }
 return 0;
}

//********************************************************************
// Transforma las coordenadas rejilla dadas en sus equivalentes libres
//********************************************************************
fm_funcion(int, ism_obtener_coords_libres, ism_get_free_coords, (int x, int y, int *xl, int *yl), (x, y, xl, yl))
{
 fm_compr_ini();
 if((x<0)||(y<0)||(x>=rejilla.x)||(y>=rejilla.y)) fm_error(2);
 if((!xl)&&(!yl)) fm_error(11);

 if(xl) *xl=(x*rejilla.anchura);
 if(yl) *yl=((y+1)*rejilla.anchura-1);
 return 0;
}

//***********************************************************************
// Indica la celda de la rejilla en la que se encuentra un punto indicado
//***********************************************************************
fm_funcion(int, ism_obtener_coords_celda, ism_get_cell_coords, (int x, int y, int *xc, int *yc), (x, y, xc, yc))
{
 fm_compr_ini();
 if((x<0)||(y<0)||(x>=rejilla.x*rejilla.anchura)||(y>=rejilla.y*rejilla.anchura)) fm_error(2);
 if((!xc)&&(!yc)) fm_error(11);

 if(xc) *xc=x/rejilla.anchura;
 if(yc) *yc=y/rejilla.anchura;
 return 0;
}

//****************************************
// Transforma coordenadas isométricas a 2d
//****************************************
fm_funcion(int, ism_coords_iso_a_2d, ism_coords_iso_to_2d, (int xiso, int yiso, int ziso, int *x2d, int *y2d), (xiso, yiso, ziso, x2d, y2d))
{
 if((!x2d)&&(!y2d)) fm_error(11);

 if(x2d) *x2d=x0ant+2*xiso-2*yiso;
 if(y2d) *y2d=y0ant+xiso+yiso-ziso;
 return 0;
}

//****************************************
// Transforma coordenadas 2d a isométricas
//****************************************
fm_funcion(int, ism_coords_2d_a_iso, ism_coords_2d_to_iso, (int *xiso, int *yiso, int *ziso, int x2d, int y2d, int vfija, unsigned char cfija), (xiso, yiso, ziso, x2d, y2d, vfija, cfija))
{
 float x,y,z;
 if((!xiso)&&(!yiso)&&(!ziso)) fm_error(11);

 switch(cfija)
  {
   case D_X:
     x=vfija;
     y=(x0ant+2*x-x2d)/2;
     z=y0ant+x+y-y2d;
     break;
   case D_Y:
     y=vfija;
     x=(x2d-x0ant+2*y)/2;
     z=y0ant+x+y-y2d;
     break;
   case D_Z:
     z=vfija;
     y=x0ant-x2d;
     y=(y/2-y0ant+y2d+z)/2;
     x=y+(x2d-x0ant)/2;
     break;
   default:
     fm_error(13);
  }
 if(xiso) *xiso=(int)x;
 if(yiso) *yiso=(int)y;
 if(ziso) *ziso=(int)z;
 return 0;
}

//***********************************************************
// Coloca un nuevo objeto rejilla y devuelve su identificador
//***********************************************************
fm_funcion(ism_id, ism_colocar_objeto_rejilla, ism_put_grid_object, (int x, int y, int z, int altura, BITMAP *mapa, BITMAP *sombra), (x, y, z, altura, mapa, sombra))
{
 objeto_r *objr=NULL;
 objeto_r *objt=NULL;
 objeto_l *objl=NULL;
 int xl=x*rejilla.anchura;       // Coordenadas libres del objeto rejilla
 int yl=(y+1)*rejilla.anchura-1; // (para compararlas con las de los objetos libres)
 int despx,despy;

 fm_id_compr_ini();
 if(sig_id_r==ULT_ID_R) fm_id_error(17);
 if((x<0)||(y<0)||(x>=rejilla.x)||(y>=rejilla.y)||(z<ENCIMA)) fm_id_error(2);
 if(altura<0) fm_id_error(14);

 prv_vaciar_pila_blq();

 objl=lista_objl;
 if(z!=ENCIMA)
  {
   // Se recorre la lista de libres buscando bloqueos
   while(objl)
    {
     if((((objl->x+objl->anchurax)>xl)   &&  (objl->x<(xl+rejilla.anchura))) && // Intersecta en la dimensión X
         ((objl->y>(yl-rejilla.anchura)) && ((objl->y-objl->anchuray)<yl))   && // Intersecta en la dimensión Y
       (((objl->z+objl->altura)>z)       &&  (objl->z<(z+altura))))             // Intersecta en la dimensión Z
       if(!(prv_apilar_objeto_blq(objl->id))) fm_id_error(3);

     objl=objl->sig;
    }

   // Se recorren los objetos de la columna buscando bloqueos y la posición que debe ocupar el objeto
   objr=suelo[rejilla.x*y+x].objr;
   while(objr)
    {
     if((z>=(objr->z+objr->altura))||((z+altura)<=objr->z))
       {if((objr->z<z)||((objr->z==z)&&(objr->altura==0))) objt=objr;}
     else
       {if(!(prv_apilar_objeto_blq(objr->id))) fm_id_error(3);}
     objr=objr->sig;
    }
  }
 else
  {
   z=0;
   // Se recorre la lista de libres buscando los que intersecten la columna
   while(objl)
    {
     if((((objl->x+objl->anchurax)>xl)   &&  (objl->x<(xl+rejilla.anchura))) && // Intersecta en la dimensión X
         ((objl->y>(yl-rejilla.anchura)) && ((objl->y-objl->anchuray)<yl))   && // Intersecta en la dimensión Y
          (objl->z+objl->altura>z))
       z=objl->z+objl->altura;
     objl=objl->sig;
    }
   // Se recorren los objetos de la columna buscando el mayor
   objt=suelo[rejilla.x*y+x].objr;
   if(objt)
    {
     while(objt->sig)
       objt=objt->sig;
     if(objt->z+objt->altura>z) 
         z=objt->z+objt->altura;
    }     
  }

 // Si está bloqueado devuelvo el error
 if(pila_blq) fm_id_error(7);

 // Se vuelve a recorrer la lista de libres buscando solapamientos y sombreamientos,
 // ahora que conozco el valor de z y se que no está bloqueado.
 if(mapa)
  {
   despx=((rejilla.anchura*(x-y))<<1)-(mapa->w>>1)+1;
   despy=rejilla.anchura*(x+y+2)-mapa->h-z-1;
  }
 else  
   despx=despy=0;

 objl=lista_objl;
 while(objl)
  {
   if(pctj_sombreado<256 && sombra)
   if(((objl->x+objl->anchurax)>xl)   && (objl->x<(xl+rejilla.anchura)) && // Intersecta en la dimensión X
       (objl->y>(yl-rejilla.anchura)) && ((objl->y-objl->anchuray)<yl)  && // Intersecta en la dimensión Y
        objl->z<z)
    {
     if(!mapa)
       fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
     objl->sombrear=1;
    }

   if(mapa)
   if(objl->mapabase)
   if((objl->inf_mapa.despx < despx+mapa->w) &&
      (objl->inf_mapa.despx+objl->mapabase->w > despx) &&
      (objl->inf_mapa.despy < despy+mapa->h) &&
      (objl->inf_mapa.despy+objl->mapabase->h > despy)) // Sus mapas se solapan
     objl->enmascarar=1;

   objl=objl->sig;
  } 

 // Creo el objeto y lo coloco en la columna
 objr=(objeto_r *)malloc(sizeof(objeto_r));
 if(!objr) fm_id_error(3);
 objr->id=sig_id_r;
 sig_id_r+=2;
 objr->z=z;
 objr->altura=altura;
 objr->inf_mapa.mapa=NULL;
 objr->mapabase=mapa;
 objr->inf_mapa.despx=despx;
 objr->inf_mapa.despy=despy;
 fm_incorporar_cuadricula(mapa,despx+x0ant,despy+y0ant);
 fm_incorporar_cuadricula(sombra,2*rejilla.anchura*(x-y)  -(sombra->w)/2+x0ant+1,
                                   rejilla.anchura*(x+y+1)-(sombra->h)/2+y0ant-1);
 objr->sombra=sombra;
 objr->sombrear=1;
 if(pctj_sombreado<256 && sombra)
   suelo[rejilla.x*rejilla.y].sombrear=suelo[rejilla.x*y+x].sombrear=1;

 if(objt)
  {
   objr->sig=objt->sig;
   objt->sig=objr;
   // En caso necesario, se marcan los elementos a sombrear
   if(pctj_sombreado<256 && sombra)
    {
     objt=suelo[rejilla.x*y+x].objr;
     do
      {
       if(!mapa)
         fm_incorporar_cuadricula(objt->mapabase,objt->inf_mapa.despx+x0ant,objt->inf_mapa.despy+y0ant);
       objt->sombrear=1;
       objt=objt->sig;
      }while(objt->id!=objr->id);
    }
  }
 else
  {
   objr->sig=suelo[rejilla.x*y+x].objr;
   suelo[rejilla.x*y+x].objr=objr;
  }

 ult_objr=objr;
 ult_objr_x=x;
 ult_objr_y=y;
 return(objr->id);
}

//**************************************************************************
// Coloca un objeto libre en el mundo isométrico y devuelve su identificador
//**************************************************************************
fm_funcion(ism_id, ism_colocar_objeto_libre, ism_put_free_object, (int x, int y, int z, int anchurax, int anchuray, int altura, BITMAP *mapa, BITMAP *sombra), (x, y, z, anchurax, anchuray, altura, mapa, sombra))
{
 objeto_r *objr=NULL;
 objeto_l *objl=NULL;
 int xini=x/rejilla.anchura;                //
 int xfin=(x+anchurax-1)/rejilla.anchura+1; // Rango de columnas
 int yini=(y-anchuray+1)/rejilla.anchura;   // intersectadas por el objeto
 int yfin=y/rejilla.anchura+1;              //
 int despx,despy;
 int f,n;

 fm_id_compr_ini();
 if(sig_id_l==ULT_ID_L) fm_id_error(18);
 if((altura<0)||(anchurax<1)||(anchuray<1)) fm_id_error(14);
 if(z<ENCIMA) fm_id_error(2);

 prv_vaciar_pila_blq();
 if(x<0)
  {
   if(!(prv_apilar_objeto_blq(ID_PARED_SUP_Y))) fm_id_error(3);
   xini=0;
  }
 if(x+anchurax>rejilla.x*rejilla.anchura)
  {
   if(!(prv_apilar_objeto_blq(ID_PARED_INF_Y))) fm_id_error(3);
   xfin=rejilla.x;
  }
 if(y-anchuray<-1)
  {
   if(!(prv_apilar_objeto_blq(ID_PARED_SUP_X))) fm_id_error(3);
   yini=0;
  }
 if(y>(rejilla.y*rejilla.anchura-1))
  {
   if(!(prv_apilar_objeto_blq(ID_PARED_INF_X))) fm_id_error(3);
   yfin=rejilla.y;
  }
 if(xini>xfin || yini>yfin) fm_id_error(7);
 objl=lista_objl;
 if(z>ENCIMA)
  {
   // Se recorre la lista de libres buscando bloqueos
   while(objl)
    {
     if(((objl->x+objl->anchurax)>x)  &&  (objl->x<(x+anchurax))      && // Intersecta en la dimensión X
         (objl->y>(y-anchuray))       && ((objl->y-objl->anchuray)<y) && // Intersecta en la dimensión Y
        ((objl->z+objl->altura)>z)    &&  (objl->z<(z+altura)))          // Intersecta en la dimensión Z
       if(!(prv_apilar_objeto_blq(objl->id))) fm_id_error(3);
     objl=objl->sig;
    }

   // Se recorren los objetos de la columna buscando bloqueos
   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(((objr->z+objr->altura)>z)&&(objr->z<(z+altura)))
           if(!(prv_apilar_objeto_blq(objr->id))) fm_id_error(3);
         objr=objr->sig;
        }
      }
  }
 else 
  {
   z=0;
   // Se recorre la lista de libres buscando los que vayan a quedar debajo
   while(objl)
    {
     if(((objl->x+objl->anchurax)>x)  &&  (objl->x<(x+anchurax))      && // Intersecta en la dimensión X
         (objl->y>(y-anchuray))       && ((objl->y-objl->anchuray)<y) && // Intersecta en la dimensión Y
         (objl->z+objl->altura>z))
       z=objl->z+objl->altura;
     objl=objl->sig;
    }
   // Se recorren los objetos de las columnas intersectadas
   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if((objr->z+objr->altura)>z)
           z=objr->z+objr->altura;
         objr=objr->sig;
        }
      }
  }

 // Si está bloqueado devuelvo el error
 if(pila_blq) fm_id_error(7);

 // Se vuelve a recorrer la lista de libres buscando solapamientos, ahora que conozco el valor de z
 // y sé que no está bloqueado
 if(mapa)
  {
   despx=2*(x-y)+anchurax+anchuray-(mapa->w>>1)-1;
   despy=x+y+anchurax-mapa->h-z;
  }
 else  
   despx=despy=0;

 objl=lista_objl;
 while(objl)
  {
   if(objl->mapabase && mapa)
   if((objl->inf_mapa.despx < despx+mapa->w) &&
      (objl->inf_mapa.despx+objl->mapabase->w > despx) &&
      (objl->inf_mapa.despy < despy+mapa->h) &&
      (objl->inf_mapa.despy+objl->mapabase->h > despy)) // Sus mapas se solapan
     objl->enmascarar=1;
   if(pctj_sombreado<256 && sombra && objl->z<=z &&
      objl->x<=x+anchurax && x<=objl->x+objl->anchurax &&
      objl->y>=y-anchuray && y>=objl->y-objl->anchuray)
    {
     objl->sombrear=1;
     if(!mapa)
       fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
    }
   objl=objl->sig;
  } 

 // Creo el objeto y lo coloco en la lista de objetos libres
 objl=(objeto_l *)malloc(sizeof(objeto_l));
 if(!objl) fm_id_error(3);
 objl->id=sig_id_l;
 sig_id_l+=2;
 objl->x=x;
 objl->y=y;
 objl->z=z;
 objl->anchurax=anchurax;
 objl->anchuray=anchuray;
 objl->altura=altura;
 objl->mapabase=mapa;
 objl->inf_mapa.mapa=NULL;
 objl->inf_mapa.despx=despx;
 objl->inf_mapa.despy=despy;
 fm_incorporar_cuadricula(mapa,despx+x0ant,despy+y0ant);
 objl->sombra=sombra;
 objl->mapasomb=NULL;
 objl->sombrear=objl->enmascarar=1;
 objl->mapasomb=NULL;
 objl->sig=lista_objl;
 lista_objl=objl;
 objl->ant=NULL;
 prv_introducir_transparencia(objl->transp=0);
 if(objl->sig)
  {
   objl->sig->ant=objl;
   prv_ordenar_objeto(objl);
  }

 // En caso necesario, se marcan las losetas y objetos rejilla para sombrearlos
 if(pctj_sombreado<256 && sombra)
  {
   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(objr->z < z)
          {
           if(!mapa)
             fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
           objr->sombrear=1;
           objr=objr->sig;
          }
         else
           objr=NULL;
        }
       suelo[rejilla.x*n+f].sombrear=1;
      } 
   suelo[rejilla.x*rejilla.y].sombrear=1;
   fm_incorporar_cuadricula(sombra,2*(x-y)+x0ant+(anchurax+anchuray)-((sombra->w)>>1)-1,
                                   x+y+y0ant+((anchurax-anchuray+1)>>1)-((sombra->h)>>1));
  }
 
 ult_objl=objl;
 return(objl->id);
}

//***************************
// Elimina el objeto indicado
//***************************
fm_funcion(int, ism_quitar_objeto, ism_remove_object, (ism_id id), (id))
{
 BITMAP *mapa;
 objeto_l *objl=NULL;
 objeto_l *objm=NULL;
 objeto_r *objr=NULL;
 objeto_r *objs=NULL;

 int despx,despy;
 int f,n;

 fm_compr_ini();
 
 if(id&1) // Objeto libre
  {
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   if(!lista_objl) fm_error(10);
   if(ult_objl) if(ult_objl->id==id) ult_objl=NULL; 
   objl=lista_objl;
   while(objl)
    {
     if(objl->id==id) break;
     objm=objl;
     objl=objl->sig;
    }
   if(!objl) fm_error(10);
   if(objm) // No estoy quitando el primer elemento de la lista
     objm->sig=objl->sig;
   else // Estoy quitando el primer elemento de la lista
     lista_objl=objl->sig;
   mapa=objl->mapabase;  
   despx=objl->inf_mapa.despx;
   despy=objl->inf_mapa.despy;
   fm_incorporar_cuadricula(mapa,despx+x0ant,despy+y0ant);
   if(objl->inf_mapa.mapa)
     destroy_bitmap(objl->inf_mapa.mapa);
   if(objl->mapasomb)
     destroy_bitmap(objl->mapasomb);
   // En caso necesario, se marcan las losetas para sombrearlas
   if(pctj_sombreado<256 && objl->sombra)
    {
     int xini=objl->x/rejilla.anchura;                      //
     int xfin=(objl->x+objl->anchurax-1)/rejilla.anchura+1; // Rango de columnas
     int yini=(objl->y-objl->anchuray+1)/rejilla.anchura;   // intersectadas por el objeto
     int yfin=objl->y/rejilla.anchura+1;                    //
     for(f=xini;f<xfin;f++)
       for(n=yini;n<yfin;n++)
        {
         objr=suelo[rejilla.x*n+f].objr;
         while(objr)
          {
           if(objr->z < objl->z)
            {
             objr->sombrear=1;
             if(!mapa)
               fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
             objr=objr->sig;
            }
           else
             objr=NULL;
          }   
         suelo[rejilla.x*n+f].sombrear=1;
        } 
     suelo[rejilla.x*rejilla.y].sombrear=1;
     fm_incorporar_cuadricula(objl->sombra,2*(objl->x-objl->y)+x0ant+(objl->anchurax+objl->anchuray)-((objl->sombra->w)>>1)-1,
                                              objl->x+objl->y+y0ant+((objl->anchurax-objl->anchuray+1)>>1)-((objl->sombra->h)>>1));

     objm=lista_objl;
     while(objm)
      {
       if(objm->z<=objl->z &&
          objm->x<=objl->x+objl->anchurax && objl->x<=objm->x+objm->anchurax &&
          objm->y>=objl->y-objl->anchuray && objl->y>=objm->y-objm->anchuray)
         {
          if(!mapa)
            fm_incorporar_cuadricula(objm->mapabase,objm->inf_mapa.despx+x0ant,objm->inf_mapa.despy+y0ant);
          objm->sombrear=1;
         }
       objm=objm->sig;  
      }
    }    

   if(objl->transp<100) prv_quitar_transparencia(objl->transp);
   if(objl->sig) objl->sig->ant=objl->ant;
   if(objl->ant && objl->sig) prv_ordenar_objeto(objl->ant);
   free(objl);
  }
 else // Objeto rejilla
  {
   f=0;
   if(id<PRIMER_ID_R || id>=sig_id_r) fm_error(9);
   if(ult_objr) if(ult_objr->id==id)
    {
     f=rejilla.x*ult_objr_y+ult_objr_x;
     ult_objr=NULL;
    }
   do
    {
     objr=suelo[f].objr;
     objs=NULL;
     while(objr)
     {
      if(objr->id==id) break;
      objs=objr;
      objr=objr->sig;
     }
     f++;
    }while((!objr)&&(f<rejilla.x*rejilla.y));

   if(!objr) fm_error(10);
   f--;
   if(objs) // No se está borrando el primero de la columna
     objs->sig=objr->sig;
   else  // Se está borrando el primero de la columna
     suelo[f].objr=objr->sig;
   despx=objr->inf_mapa.despx;
   despy=objr->inf_mapa.despy;
   mapa=objr->mapabase;
   fm_incorporar_cuadricula(mapa,despx+x0ant,despy+y0ant);
   if(objr->sombra)
    {
     int x=f%rejilla.x;
     int y=f/rejilla.x;
     fm_incorporar_cuadricula(objr->sombra,2*rejilla.anchura*(x-y)  -(objr->sombra->w)/2+x0ant+1,
                                             rejilla.anchura*(x+y+1)-(objr->sombra->h)/2+y0ant-1);
    }
   if(objr->sombra && pctj_sombreado<256)
    {
     int xl=(f%rejilla.x)*rejilla.anchura;     // Coordenadas libres del objeto rejilla
     int yl=(f/rejilla.x+1)*rejilla.anchura-1; // (para compararlas con las de los objetos libres)

     suelo[rejilla.x*rejilla.y].sombrear=suelo[f].sombrear=1;

     if(objs)
      {objeto_r *objt=suelo[f].objr;
       while(objt->id!=objs->id)
        {
         objt->sombrear=1;
         objt=objt->sig;
        } 
       if(!mapa)
         fm_incorporar_cuadricula(objs->mapabase,objs->inf_mapa.despx+x0ant,objs->inf_mapa.despy+y0ant);
       objs->sombrear=1;
      }

     objl=lista_objl;
     while(objl)
      {
       if(((objl->x+objl->anchurax)>xl)   && (objl->x<(xl+rejilla.anchura)) && // Intersecta en la dimensión X
           (objl->y>(yl-rejilla.anchura)) && ((objl->y-objl->anchuray)<yl)  && // Intersecta en la dimensión Y
            objl->z<objr->z)
        {
         if(!mapa)
           fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
         objl->sombrear=1;
        }
       objl=objl->sig;
      }
    } 
   if(objr->inf_mapa.mapa)
     destroy_bitmap(objr->inf_mapa.mapa);
   free(objr);
  }
 // Se comprueban los solapamientos del objeto que hemos quitado
 objl=lista_objl;
 if(mapa)
   while(objl)
    {
     if(objl->mapabase)
       if((objl->inf_mapa.despx < despx+mapa->w) &&
          (objl->inf_mapa.despx+objl->mapabase->w > despx) &&
          (objl->inf_mapa.despy < despy+mapa->h) &&
          (objl->inf_mapa.despy+objl->mapabase->h > despy)) // Sus mapas se solapan
         objl->enmascarar=1;
         
     objl=objl->sig;
    }
   
 return 0; 
}

//****************************************
// Obtiene el dato solicitado de un objeto
//****************************************
fm_funcion(int, ism_obtener_dato_objeto, ism_get_object_data, (ism_id id, unsigned char dato), (id, dato))
{
 objeto_l *objl=NULL;
 objeto_l *objm=NULL;
 objeto_r *objr=NULL;

 fm_compr_ini();
 
 if(id&1)
  { // Objeto libre
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   objm=lista_objl;
   if(ult_objl) if(ult_objl->id==id)
    {
     objl=ult_objl;
     objm=NULL;
    }
   while(objm)
    {
     if(objm->id==id)
      {
       objl=objm;
       objm=NULL;
      }
     else
      {
       objm=objm->sig; 
      }
    }
   if(objl)
    {
     ult_objl=objl;
     switch(dato)
      {
       case D_X: return objl->x;
       case D_Y: return objl->y;
       case D_Z: return objl->z;
       case D_ALTURA: return objl->altura;
       case D_ANCHURA_X: return objl->anchurax;
       case D_ANCHURA_Y: return objl->anchuray;
       case D_TRANSP: return objl->transp;
       default: fm_error(8);
      }
    }
  }
 else
  { // Objeto rejilla
   int f,n;
   if(id<PRIMER_ID_R || id>=sig_id_r) fm_error(9);
   if(ult_objr) if(ult_objr->id==id)
    {
     switch(dato)
      {
       case D_X: return ult_objr_x;
       case D_Y: return ult_objr_y;
       case D_Z: return ult_objr->z;
       case D_ALTURA: return ult_objr->altura;
       case D_ANCHURA_X: return rejilla.anchura;
       case D_ANCHURA_Y: return rejilla.anchura;
       case D_TRANSP: return 0;
       default: fm_error(8);
      }
    }
   for(f=0;f<rejilla.x;f++)
     for(n=0;n<rejilla.y;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(objr->id==id)
          {
           ult_objr=objr;
           ult_objr_x=f;
           ult_objr_y=n;
           switch(dato)
            {
             case D_X: return f;
             case D_Y: return n;
             case D_Z: return objr->z;
             case D_ALTURA: return objr->altura;
             case D_ANCHURA_X: return rejilla.anchura;
             case D_ANCHURA_Y: return rejilla.anchura;
             case D_TRANSP: return 0;
             default: fm_error(8);
            }
          }
         objr=objr->sig;
        }
      }
  }
 fm_error(10);
}

//*************************************
// Obtiene las coordenadas de un objeto
//*************************************
fm_funcion(int, ism_obtener_coords_objeto, ism_get_object_coords, (ism_id id, int *dstx, int *dsty, int *dstz), (id, dstx, dsty, dstz))
{
 objeto_l *objl=NULL;
 objeto_r *objr=NULL;

 fm_compr_ini();
 if((!dstx)&&(!dsty)&&(!dstz)) fm_error(11);
 
 if(id&1)
  { // Objeto libre
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   objl=lista_objl;
   if(ult_objl) if(ult_objl->id==id) objl=ult_objl;
   while(objl)
    {
     if(objl->id==id)
      {
       ult_objl=objl;
       if(dstx) *dstx=objl->x;
       if(dsty) *dsty=objl->y;
       if(dstz) *dstz=objl->z;
       return 0;
      }
     objl=objl->sig; 
    }
  }
 else
  { // Objeto rejilla
   int f,n;
   if(id<PRIMER_ID_R || id>=sig_id_r) fm_error(9);
   if(ult_objr) if(ult_objr->id==id)
    {
     if(dstx) *dstx=ult_objr_x;
     if(dsty) *dsty=ult_objr_y;
     if(dstz) *dstz=ult_objr->z;
     return 0;
    }
   for(f=0;f<rejilla.x;f++)
     for(n=0;n<rejilla.y;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(objr->id==id)
          {
           ult_objr=objr;
           if(dstx) *dstx=ult_objr_x=f;
           if(dsty) *dsty=ult_objr_y=n;
           if(dstz) *dstz=objr->z;
           return 0;
          }
         objr=objr->sig;
        }
      }
  }
 fm_error(10);
}

//***************************************
// Cambia el dato solicitado de un objeto
//***************************************
fm_funcion(int, ism_cambiar_dato_objeto, ism_change_object_data, (ism_id id, unsigned char dato, int valor, unsigned char modo), (id, dato, valor, modo))
{
 int x,y,z,altura,despx,despy;
 objeto_r *objr=NULL;
 objeto_l *objl=NULL;

 fm_compr_ini();
 if(dato>D_TRANSP) fm_error(8);
 if(modo>SUMAR) fm_error(6);

 prv_vaciar_pila_blq();

 if(id&1)
  {
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   if(dato==D_TRANSP)
     return prv_cambiar_transparencia_objeto(id, valor, modo);
   else
     return prv_mover_objeto_libre(id,valor,0,0,0,dato,modo);
  } 
 else
  {objeto_r *objt=NULL; // Objeto de la columna tras el que debe ir tras el cambio
   objeto_r *objs=NULL; // Objeto de la columna tras el que va antes del cambio
   objeto_r *objtmp=NULL;
   int f=0;

   if(id<PRIMER_ID_R || id>=sig_id_r) fm_error(9);
   if(dato!=D_Z && dato!=D_ALTURA) fm_error(12);
   // Se busca el objeto y el que va antes
   if(ult_objr) if(ult_objr->id==id)
    {
     objr=suelo[(f=rejilla.x*(y=ult_objr_y)+(x=ult_objr_x))].objr;
     while(objr->id!=id)
      {
       objs=objr;
       objr=objr->sig;
      }
    } 
   if(!objr)
    {
     do
      {
       objr=suelo[f].objr;
       objs=NULL;
       while(objr)
        {
         if(objr->id==id) break;
         objs=objr;
         objr=objr->sig;
        }
       f++;
      }while(!objr && f<rejilla.x*rejilla.y);
     if(!objr) fm_error(10);
     f--;
     x=f%rejilla.x;
     y=f/rejilla.x;
    } 
    
   x=x*rejilla.anchura;       // Coordenadas llbres del objeto rejilla
   y=(y+1)*rejilla.anchura-1; // (para compararlas con las de los objetos libres)
   if(dato==D_Z)
    {  
     if((z=valor+objr->z*modo)<0)
       if(!(prv_apilar_objeto_blq(ID_SUELO))) fm_error(3);;
     altura=objr->altura;
    }
   else 
    {  
     z=objr->z;
     altura=valor+objr->altura*modo;
    }
  
   objl=lista_objl;
   // Se recorre la lista de libres buscando bloqueos
   while(objl)
    {
     if((((objl->x+objl->anchurax)>x)   &&  (objl->x<(x+rejilla.anchura))) && // Intersecta en la dimensión X
         ((objl->y>(y-rejilla.anchura)) && ((objl->y-objl->anchuray)<y))   && // Intersecta en la dimensión Y
       (((objl->z+objl->altura)>z)      &&  (objl->z<(z+altura))))            // Intersecta en la dimensión Z
       if(!(prv_apilar_objeto_blq(objl->id))) fm_error(3);

     objl=objl->sig;
    }

   // Se recorren los objetos de la columna buscando bloqueos y la posición que debe ocupar el objeto
   objtmp=suelo[f].objr;
   while(objtmp)
    {                 
     if(objtmp->id!=id)
       if((z>=(objtmp->z+objtmp->altura))||((z+altura)<=objtmp->z))
        {
         if((objtmp->z<z)||((objtmp->z==z)&&(objtmp->altura==0))) objt=objtmp;
        }
       else
        {
         if(!(prv_apilar_objeto_blq(objtmp->id))) fm_error(3);
        }
     objtmp=objtmp->sig;
    }

   // Si está bloqueado devuelvo el error
   if(pila_blq) fm_error(7);

   // Se vuelve a recorrer la lista de libres buscando solapamientos, ahora que sé que no está bloqueado
   if(objr->mapabase)
    {int despy2;
     int xl=(f%rejilla.x)*rejilla.anchura;     // Coordenadas libres del objeto rejilla
     int yl=(f/rejilla.x+1)*rejilla.anchura-1; // (para compararlas con las de los objetos libres)

     despx=objr->inf_mapa.despx;
     x=f%rejilla.x;
     y=f/rejilla.x;
     despy=rejilla.anchura*(x+y+2)-objr->mapabase->h-z-1;
     despy2=rejilla.anchura*(x+y+2)-objr->mapabase->h-objr->z-1;
     objl=lista_objl;
     while(objl)
      {
       if(objl->mapabase)
        {
         if((objl->inf_mapa.despx < despx+objr->mapabase->w) &&
            (objl->inf_mapa.despx+objl->mapabase->w > despx) &&
           (((objl->inf_mapa.despy < despy+objr->mapabase->h) &&
             (objl->inf_mapa.despy+objl->mapabase->h > despy)) ||
            ((objl->inf_mapa.despy < despy2+objr->mapabase->h) &&
             (objl->inf_mapa.despy+objl->mapabase->h > despy2))) &&  // Sus mapas se solapan
            !((objl->x >= xl+rejilla.anchura) ||
              (objl->y-objl->anchuray >= yl)  ||
              ((objl->z >= z+altura) && (objl->z >=objr->z+objr->altura)))) 
            objl->enmascarar=1;
        }   
       objl=objl->sig;
      } 
    }
   else  
     despx=despy=0;

   if(dato==D_Z && pctj_sombreado<256)
    {
     int xl=(f%rejilla.x)*rejilla.anchura;     // Coordenadas libres del objeto rejilla
     int yl=(f/rejilla.x+1)*rejilla.anchura-1; // (para compararlas con las de los objetos libres)
     int z1,z2;

     if(objr->z>z)
      {
       z1=z;
       z2=objr->z;
      }
     else
      {
       z1=objr->z;
       z2=z;
      }

     objl=lista_objl;
     while(objl)
      {
       if(objl->mapabase)
       if(((objl->x+objl->anchurax)>xl)   && (objl->x<(xl+rejilla.anchura)) && // Intersecta en la dimensión X
           (objl->y>(yl-rejilla.anchura)) && ((objl->y-objl->anchuray)<yl)  && // Intersecta en la dimensión Y
           (objl->z>z1 && objl->z<z2))                                         // Intersecta en la dimensión Z
        {
         objr->sombrear=objl->sombrear=1;
         fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
        }
       objl=objl->sig;
      }
    }
   fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
   fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,despy+y0ant);
   objr->inf_mapa.despy=despy;
   objr->z=z;
   objr->altura=altura;

   if(dato==D_Z)
    {
     char cuadricular_sombreados=(objr->mapabase?0:1);
     if(objs)
       objs->sig=objr->sig;
     else
       suelo[f].objr=objr->sig;  
     if(objt)
      {
       objr->sig=objt->sig;
       objt->sig=objr;
      } 
     else
      {
       objr->sig=suelo[f].objr;
       suelo[f].objr=objr;
      } 
     if(pctj_sombreado<256)
      {
       if(objs && objt) if(objs->id != objt->id)
        {
         objr->sombrear=1;
         if(objs->z > objr->z)
          {
           objr=objs;
           objs=objt;
          }
         while(objs->id != objr->id) 
          {
           objs=objs->sig;
           objs->sombrear=1;
           if(cuadricular_sombreados)
             fm_incorporar_cuadricula(objs->mapabase,objs->inf_mapa.despx+x0ant,objs->inf_mapa.despy+y0ant);
          }
        }  
       if(objs && !objt)   
        {
         objt=objr=objs;
         objs=NULL;
        }
       if(!objs && objt)
        {
         objtmp=suelo[f].objr;
         objr->sombrear=objtmp->sombrear=1;
         if(cuadricular_sombreados)
           fm_incorporar_cuadricula(objtmp->mapabase,objtmp->inf_mapa.despx+x0ant,objtmp->inf_mapa.despy+y0ant);
         while(objtmp->id != objr->id)
          {
           objtmp=objtmp->sig;
           objtmp->sombrear=1;
           if(cuadricular_sombreados)
             fm_incorporar_cuadricula(objtmp->mapabase,objtmp->inf_mapa.despx+x0ant,objtmp->inf_mapa.despy+y0ant);
          }
        }   
      }
    }
  } 
 return 0;
}

//*****************************
// Cambio del mapa de un objeto
//*****************************
fm_funcion(int, ism_cambiar_mapa_objeto, ism_change_object_map, (ism_id id, BITMAP *mapa), (id, mapa))
{
 objeto_l *objm=lista_objl;
 ism_id id_objl=NO_ID;
 BITMAP *mapaant;
 int despx0, despy0, despx1, despy1;
 
 fm_compr_ini();
 
 if(id&1) // Objeto libre
  {objeto_l *objl=NULL;
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   // Se localiza el objeto
   if(ult_objl) if(ult_objl->id==id)
     objl=ult_objl;
   if(!objl)
    {
     objl=lista_objl;
     while(objl)
      {
       if(objl->id==id) break;
       objl=objl->sig;
      }
     if(!objl) fm_error(10);
    }

   // Se le cambian el mapa y la posición
   id_objl=objl->id;
   mapaant=objl->mapabase;
   despx1=objl->inf_mapa.despx;
   despy1=objl->inf_mapa.despy;
   fm_incorporar_cuadricula(mapaant,despx1+x0ant,despy1+y0ant);
   objl->mapabase=mapa;
   objl->sombrear=objl->enmascarar=1;
   if(mapa)
    {
     despx0=objl->inf_mapa.despx=2*(objl->x-objl->y)+objl->anchurax+objl->anchuray-(mapa->w>>1)-1;
     despy0=objl->inf_mapa.despy=objl->x+objl->y+objl->anchurax-mapa->h-objl->z;
     fm_incorporar_cuadricula(mapa,despx0+x0ant,despy0+y0ant);
     // Si el nuevo mapa tiene un tamaño distinto del anterior, se destruye el mapa de enmascaramiento
     if(objl->inf_mapa.mapa)
       if(objl->inf_mapa.mapa->h!=mapa->h || objl->inf_mapa.mapa->w!=mapa->w)
        {
         destroy_bitmap(objl->inf_mapa.mapa);
         objl->inf_mapa.mapa=NULL;
        }
     // Si el nuevo mapa tiene un tamaño distinto del anterior, se destruye el mapa de sombreado
     if(objl->mapasomb)
       if(objl->mapasomb->h!=mapa->h || objl->mapasomb->w!=mapa->w)
        {
         destroy_bitmap(objl->mapasomb);
         objl->mapasomb=NULL;
        }
    }
   else
    {
     objl->inf_mapa.despx=objl->inf_mapa.despy=0;
     // Si el nuevo mapa es nulo, también se destruye el mapa de enmascaramiento
     if(objl->inf_mapa.mapa)
      {
       destroy_bitmap(objl->inf_mapa.mapa); 
       objl->inf_mapa.mapa=NULL;
      }
     // Si el nuevo mapa es nulo, también se destruye el mapa de sombreado
     if(objl->mapasomb)
      {
       destroy_bitmap(objl->mapasomb);
       objl->mapasomb=NULL;
      }
    }
   objl->enmascarar=1;
   ult_objl=objl; 
  }
 else // Objeto rejilla
  {int f=0;
   objeto_r *objr=NULL;
   int x,y;
   if(id<PRIMER_ID_R || id>=sig_id_r) fm_error(9);
   // Se busca el objeto
   if(ult_objr) if(ult_objr->id==id)
    {
     objr=ult_objr;
     x=ult_objr_x;
     y=ult_objr_y;
    } 
   if(!objr)
    {
     do
      {
       objr=suelo[f].objr;
       while(objr)
        {
         if(objr->id==id) break;
         objr=objr->sig;
        }
       f++;
      }while(!objr && f<rejilla.x*rejilla.y);
     if(!objr) fm_error(10);
     f--;
     x=f%rejilla.x;
     y=f/rejilla.x;
    }  

   // Se le coloca el mapa y el desplazamiento
   mapaant=objr->mapabase;
   despx1=objr->inf_mapa.despx;
   despy1=objr->inf_mapa.despy;
   fm_incorporar_cuadricula(mapaant,despx1+x0ant,despy1+y0ant);
   objr->mapabase=mapa;
   if(objr->inf_mapa.mapa)
    {
     destroy_bitmap(objr->inf_mapa.mapa);
     objr->inf_mapa.mapa=NULL;
    } 
   if(mapa)
    { 
     if(pctj_sombreado<256)
       objr->sombrear=1;
     despx0=objr->inf_mapa.despx=((rejilla.anchura*(x-y))<<1)-(mapa->w>>1)+1;
     despy0=objr->inf_mapa.despy=rejilla.anchura*(x+y+2)-mapa->h-objr->z-1;
     fm_incorporar_cuadricula(mapa,despx0+x0ant,despy0+y0ant);
    }
   else
     objr->inf_mapa.despx=objr->inf_mapa.despy=0;

   if(objr!=ult_objr)
    {
     ult_objr=objr;
     ult_objr_x=x;
     ult_objr_y=y;
    }
  }

 // Se recorre la lista de libres para ver cuales hay que volver a enmascarar
 while(objm)
  {
   if((objm->id!=id) && objm->mapabase)
    {
     if(mapaant)
      {
       if((objm->inf_mapa.despx < despx1+mapaant->w) &&
         (objm->inf_mapa.despx+objm->mapabase->w > despx1) &&
         (objm->inf_mapa.despy < despy1+mapaant->h) &&
         (objm->inf_mapa.despy+objm->mapabase->h > despy1))
         objm->enmascarar=1;
      }   
     else
       if(mapa)
         if((objm->inf_mapa.despx < despx0+mapa->w) &&
            (objm->inf_mapa.despx+objm->mapabase->w > despx0) &&
            (objm->inf_mapa.despy < despy0+mapa->h) &&
            (objm->inf_mapa.despy+objm->mapabase->h > despy0))
           objm->enmascarar=1;
    }   
   objm=objm->sig;
  }

 return 0;
}

//*********************************
// Cambio de la sombra de un objeto
//*********************************
fm_funcion(int, ism_cambiar_sombra_objeto, ism_change_object_shadow, (ism_id id, BITMAP *sombra), (id, sombra))
{
 int f,n;
 fm_compr_ini();
 
 if(id&1) // Objeto libre
  {objeto_l *objl=NULL;
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   // Se localiza el objeto
   if(ult_objl) if(ult_objl->id==id)
     objl=ult_objl;
   if(!objl)
    {
     objl=lista_objl;
     while(objl)
      {
       if(objl->id==id) break;
       objl=objl->sig;
      }
     if(!objl) fm_error(10);
     ult_objl=objl;
    }

   // Si es necesario, se sombrean las losetas
   if(pctj_sombreado<256)
    {
     int xini=objl->x/rejilla.anchura;                      // 
     int xfin=(objl->x+objl->anchurax-1)/rejilla.anchura+1; // Rango de columnas
     int yini=(objl->y-objl->anchuray+1)/rejilla.anchura;   // intersectadas por el objeto
     int yfin=objl->y/rejilla.anchura+1;                    //
     objeto_l *objm=lista_objl;

     for(f=xini;f<xfin;f++)
       for(n=yini;n<yfin;n++)
        {objeto_r *objr=suelo[rejilla.x*n+f].objr;
         while(objr)
          {
           if(objl->z>objr->z)
            {
             objr->sombrear=1;
             fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
             objr=objr->sig;
            }
           else
             objr=NULL; 
          }
         suelo[rejilla.x*n+f].sombrear=1;
        } 
     suelo[rejilla.x*rejilla.y].sombrear=1;
     fm_incorporar_cuadricula(sombra,2*(objl->x-objl->y)+x0ant+(objl->anchurax+objl->anchuray)-((sombra->w)>>1)-1,
                                     objl->x+objl->y+y0ant+((objl->anchurax-objl->anchuray+1)>>1)-((sombra->h)>>1));
     fm_incorporar_cuadricula(objl->sombra,2*(objl->x-objl->y)+x0ant+(objl->anchurax+objl->anchuray)-((objl->sombra->w)>>1)-1,
                                     objl->x+objl->y+y0ant+((objl->anchurax-objl->anchuray+1)>>1)-((objl->sombra->h)>>1));

     while(objm)
      {
       if(objm->id!=objl->id)
       if(objm->z<=objl->z &&
          objm->x<=objl->x+objl->anchurax && objl->x<=objm->x+objm->anchurax &&
          objm->y>=objl->y-objl->anchuray && objl->y>=objm->y-objm->anchuray)
        {
         objm->sombrear=1;
         fm_incorporar_cuadricula(objm->mapabase,objm->inf_mapa.despx+x0ant,objm->inf_mapa.despy+y0ant);
        }
       objm=objm->sig;  
      }
    }
    
   objl->sombra=sombra;
  }
 else // Objeto rejilla
  {objeto_r *objr=NULL;
   int x,y;
   f=0;
   if(id<PRIMER_ID_R || id>=sig_id_r) fm_error(9);
   // Se busca el objeto
   if(ult_objr) if(ult_objr->id==id)
    {
     objr=ult_objr;
     x=ult_objr_x;
     y=ult_objr_y;
     f=y*rejilla.x+x;
    } 
   if(!objr)
    {
     do
      {
       objr=suelo[f].objr;
       while(objr)
        {
         if(objr->id==id) break;
         objr=objr->sig;
        }
       f++;
      }while(!objr && f<rejilla.x*rejilla.y);
     if(!objr) fm_error(10);
     f--;
     x=f%rejilla.x;
     y=f/rejilla.x;
    }  

   // Se le coloca la sombra, y se indica que hay que sombrear
   if(pctj_sombreado<256)
    {objeto_r *objs=suelo[f].objr;
     objeto_l *objl=lista_objl;
     int xl=x*rejilla.anchura;       // Coordenadas libres del objeto rejilla
     int yl=(y+1)*rejilla.anchura-1; // (para compararlas con las de los objetos libres)
 
     while(objl)
      {
       if(((objl->x+objl->anchurax)>xl)   && (objl->x<(xl+rejilla.anchura)) && // Intersecta en la dimensión X
           (objl->y>(yl-rejilla.anchura)) && ((objl->y-objl->anchuray)<yl)  && // Intersecta en la dimensión Y
            objl->z<objr->z)
        {
         fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
         objl->sombrear=1;   
        }
       objl=objl->sig;
      }

     while(objr->id!=objs->id)
      {
       fm_incorporar_cuadricula(objs->mapabase,objs->inf_mapa.despx+x0ant,objs->inf_mapa.despy+y0ant);
       objs->sombrear=1;
       objs=objs->sig;
      }
     suelo[f].sombrear=1;
     suelo[rejilla.x*rejilla.y].sombrear=1;
     fm_incorporar_cuadricula(sombra,2*rejilla.anchura*(x-y)  -(sombra->w)/2+x0ant+1,
                                       rejilla.anchura*(x+y+1)-(sombra->h)/2+y0ant-1);
     fm_incorporar_cuadricula(objr->sombra,2*rejilla.anchura*(x-y)  -(objr->sombra->w)/2+x0ant+1,
                                             rejilla.anchura*(x+y+1)-(objr->sombra->h)/2+y0ant-1);
     objr->sombra=sombra;
    } 
     
   if(objr!=ult_objr)
    {
     ult_objr=objr;
     ult_objr_x=x;
     ult_objr_y=y;
    }
  }

 return 0;
}

//*************************************
// Establece el porcentaje de sombreado
//*************************************
fm_funcion(int, ism_establecer_sombras, ism_set_shadowing_percentage, (char sombras), (sombras))
{short s;
 int f;
 if(sombras>100 || sombras<0) fm_error(16);

 switch(sombras) 
  {
   case 87:
     s=32; break;
   case 93:
     s=16; break;
   case 97:
     s=8; break;
   case 98:
     s=4; break;
   default:
     s=(short)(256.0-2.56*((float)sombras)); break;
  } 

 if(pctj_sombreado!=s)
  {objeto_l *objl=lista_objl;
   pctj_sombreado=s;
   if(dt_rect.modo!=RECT_DESC) dt_rect.modo=RECT_DIB_TODO;
   pctj_sombreado_32=(char)(pctj_sombreado>>3);
   if(!no_inic)
    {
     while(objl)
      {
       objl->sombrear=1;
       objl=objl->sig;
      }

     for(f=0;f<=rejilla.x*rejilla.y;f++)
      {objeto_r *objr=suelo[f].objr;
       suelo[f].sombrear=1;
       while(objr)
        {
         objr->sombrear=1;
         objr=objr->sig;
        }
      } 
    } 
  }
 return 0;
}

//******************************************************
// Establece el porcentaje de oscuridad de la habitación
//******************************************************
fm_funcion(int, ism_establecer_oscuridad, ism_set_gloom_percentage, (char oscuridad), (oscuridad))
{short s;

 if(oscuridad>100 || oscuridad<0) fm_error(20);

 switch(oscuridad) 
  {
   case 87:
     s=32; break;
   case 93:
     s=16; break;
   case 97:
     s=8; break;
   case 98:
     s=4; break;
   default:
     s=(short)(256.0-2.56*((float)oscuridad)); break;
  } 

 if(dt_oscluz.pctj_oscuridad!=s)
  {
   dt_oscluz.pctj_oscuridad=s;
   dt_oscluz.pctj_oscuridad_32=(char)(dt_oscluz.pctj_oscuridad>>3);
   if(dt_rect.modo!=RECT_DESC) dt_rect.modo=RECT_DIB_TODO;
  } 

 return 0;
}

//******************************************
// Habilita o desabilita el rectángulo sucio
//******************************************
fm_funcion(void, ism_habilitar_rect_sucio, ism_enable_dirty_rect, (char habilitar), (habilitar))
{
 if(habilitar)
  {
   if(dt_rect.modo==RECT_DESC)
     dt_rect.modo=RECT_DIB_TODO;
  }
 else
   dt_rect.modo=RECT_DESC;
}

//****************
// Mueve un objeto
//****************
fm_funcion(int, ism_mover_objeto, ism_move_object, (ism_id id, int x, int y, int z, unsigned char modo), (id, x, y, z, modo))
{
 fm_compr_ini();
 if(modo>SUMAR) fm_error(6);

 prv_vaciar_pila_blq();

 if(id&1)
  {
   if(id<PRIMER_ID_L || id>=sig_id_l) fm_error(9);
   return prv_mover_objeto_libre(id,0,x,y,z,100,modo);
  } 
 else
   fm_error(12);
}   

//****************************************************
// Devuelve 1 si el objeto existe, 0 en caso contrario
//****************************************************
fm_funcion(int, ism_existe_objeto, ism_object_exists, (ism_id id), (id))
{
 fm_compr_ini();

 if(id&1)
  {objeto_l *objl=lista_objl;
   if(id<PRIMER_ID_L || id>=sig_id_l) return 0;
   if(ult_objl) if(ult_objl->id==id) return 1;
   while(objl)
    {
     if(objl->id==id)
      {
       ult_objl=objl;
       return 1;
      }
     objl=objl->sig;
    }
  }
 else
  {int x,y;
   if(id<PRIMER_ID_R || id>=sig_id_r) return 0;
   if(ult_objr) if(ult_objr->id==id) return 1;
   for(x=0;x<rejilla.x;x++)
     for(y=0;y<rejilla.y;y++)
      {objeto_r *objr=suelo[rejilla.x*y+x].objr;
       while(objr)
        {
         if(objr->id==id)
          {
           ult_objr=objr;
           ult_objr_x=x;
           ult_objr_y=y;
           return 1;
          }
        }
      }
  }
 return 0;
}

//***********************************************
// Extrae un identificador de la pila de bloqueos
//***********************************************
fm_funcion(ism_id, ism_extraer_id_blq, ism_get_coll_id, (void), ())
{
 ism_id id=NO_ID;

 if(pila_blq)
  {
   id=pila_blq->id;
   pila_blq=pila_blq->sig;
  }
 return id;
}

//******************************************************
// Indica si se ha colisionado con un objeto determinado
//******************************************************
fm_funcion(int, ism_colisionado_con, ism_collided_against, (ism_id id), (id))
{
 elem_blq *blq=primer_blq;

 while(blq)
  {
   if(blq->id==id) return 1;
   blq=blq->sig;
  }
 return 0;
}

//**********************************************************
// Indica el número de objetos con los que se ha colisionado
//**********************************************************
fm_funcion(int, ism_num_colisiones, ism_collisions_number, (void), ())
{
 int n=0;
 elem_blq *blq=primer_blq;

 while(blq)
  {
   n++;
   blq=blq->sig;
  }
 return n;
}

//********************************************************
// Establece la función para dibujar objetos transparentes
//********************************************************
fm_funcion(int, ism_func_transp, ism_set_transp_func, (void(*f_transp)(BITMAP *, BITMAP *, int, int, int)), (f_transp))
{
 fm_compr_ini()

 if(func_transp!=f_transp)
  {objeto_l *objl=lista_objl;
   while(objl)
    {
     if(objl->transp>0 && objl->transp<100)
       fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
     objl=objl->sig;
    }
   func_transp=f_transp;
  }
 return 0;
}

//**************************************************
// Devuelve la descripción del último error ocurrido
//**************************************************
#ifdef ISM_ESP
char *ism_desc_error(void)
{
 static char mensaje[60]; // Los mensajes tienen un máximo de 59 caracteres
 char f=0;
 char *err[]={
     /*  0 */ "Sin error",
     /*  1 */ "Rejilla no inicializada",
     /*  2 */ "Valor incorrecto de x, y o z",
     /*  3 */ "Error de manejo de memoria",
     /*  4 */ "Pared inexistente",
     /*  5 */ "Ancho de rejilla menor que 2",
     /*  6 */ "Modo de cambio del dato incorrecto",
     /*  7 */ "Colisionado con otro objeto, la pared o el suelo",
     /*  8 */ "Dato solicitado desconocido",
     /*  9 */ "Identificador incorrecto",
     /* 10 */ "Objeto inexistente",
     /* 11 */ "Variables de salida nulas",
     /* 12 */ "No se puede cambiar el dato solicitado de un objeto rejilla", //59+1
     /* 13 */ "Eje incorrecto",
     /* 14 */ "Altura y/o anchura/s incorrectas",
     /* 15 */ "Mapa de destino nulo",
     /* 16 */ "Porcentaje de sombreado incorrecto",
     /* 17 */ "Alcanzada la cantidad maxima de objetos rejilla",
     /* 18 */ "Alcanzada la cantidad maxima de objetos libres",
     /* 19 */ "Porcentaje de transparencia incorrecto",
     /* 20 */ "Porcentaje de oscuridad incorrecto"};

 do
  {
   mensaje[f]=err[error][f];
  }while(mensaje[f++]);
 return mensaje;
}
#endif
#ifdef ISM_ENG
char *ism_error_desc(void)
{
 static char mensaje[55]; // Los mensajes tienen un máximo de 54 caracteres
 char f=0;
 char *err[]={
     /*  0 */ "No error",
     /*  1 */ "Grid not initialized",
     /*  2 */ "Incorrect x, y or z value",
     /*  3 */ "Memory handling error",
     /*  4 */ "Wall does not exist",
     /*  5 */ "Grid width less than 2",
     /*  6 */ "Incorrect data change mode",
     /*  7 */ "Collided aganist another object, the wall or the floor", //54+1
     /*  8 */ "Unknown requested data",
     /*  9 */ "Wrong identifier", 
     /* 10 */ "Object does not exist",
     /* 11 */ "Null exit variables",
     /* 12 */ "Can't change the requested data of a grid object",
     /* 13 */ "Wrong axis",
     /* 14 */ "Incorrect height and or width/s",
     /* 15 */ "Null target map",
     /* 16 */ "Incorrect shadowing percentage",
     /* 17 */ "Max number of grid objects reached",
     /* 18 */ "Max number of free objects reached",
     /* 19 */ "Incorrect transparency percentage",
     /* 20 */ "Incorrect gloom percentage",
              "R Tape loading error"};

 do
  {
   mensaje[f]=err[error][f];
  }while(mensaje[f++]);
 return mensaje;
}
#endif


// Implementación de las funciones privadas

//***************************************************
// Coloca un mapa en una o varias paredes, o la mueve
//***************************************************
int prv_modificar_pared(BITMAP *mapa, unsigned char eje, int pared, int despeje, int despaltura, short colocar_mapa)
{
 int f,n;

 fm_compr_ini();
 if(pared<TODAS) fm_error(4);
 
 switch(eje)
  {
   case SUP_X:
     if(pared>=rejilla.x) fm_error(4);
     if(pared==TODAS)
       if(colocar_mapa)
         for(f=0;f<rejilla.x;f++)
          {
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
           paredx[f].mapa=mapa;
           if(mapa)
            {
             paredx[f].despx=2*f*rejilla.anchura+1+x0ant;
             paredx[f].despy=(f+1)*rejilla.anchura-mapa->h-1+y0ant;
            }
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
          }
       else
         for(f=0;f<rejilla.x;f++)
          { 
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
           paredx[f].despx+=2*despeje;
           paredx[f].despy+=despeje-despaltura;
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
          } 
     else
       if(colocar_mapa)
        {
         fm_incorporar_cuadricula(paredx[pared].mapa,paredx[pared].despx,paredx[pared].despy);
         paredx[pared].mapa=mapa;
         if(mapa)
          {
           paredx[pared].despx=2*pared*rejilla.anchura+1+x0ant;
           paredx[pared].despy=(pared+1)*rejilla.anchura-mapa->h-1+y0ant;
          }
         fm_incorporar_cuadricula(paredx[pared].mapa,paredx[pared].despx,paredx[pared].despy);
        }
       else
        {
         fm_incorporar_cuadricula(paredx[pared].mapa,paredx[pared].despx,paredx[pared].despy);
         paredx[pared].despx+=2*despeje;
         paredx[pared].despy+=despeje-despaltura;
         fm_incorporar_cuadricula(paredx[pared].mapa,paredx[pared].despx,paredx[pared].despy);
        }
     return 0;
   case INF_X:
     if(pared>=rejilla.x) fm_error(4);
     if(pared==TODAS)
       if (colocar_mapa)
         for(f=rejilla.x;f<(rejilla.x<<1);f++)
          {
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
           paredx[f].mapa=mapa;
           paredx[f].despx=rejilla.anchura*(f-rejilla.x-rejilla.y)*2+1+x0ant;
           paredx[f].despy=rejilla.anchura*(f-rejilla.x+rejilla.y)+y0ant;
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
          }
       else
         for(f=rejilla.x;f<(rejilla.x<<1);f++)
          {
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
           paredx[f].despx+=2*despeje;
           paredx[f].despy+=despeje-despaltura;
           fm_incorporar_cuadricula(paredx[f].mapa,paredx[f].despx,paredx[f].despy);
          }
     else
      {
       n=pared+rejilla.x;
       fm_incorporar_cuadricula(paredx[n].mapa,paredx[n].despx,paredx[n].despy);
       if (colocar_mapa)
        {
         paredx[pared+rejilla.x].mapa=mapa;
         paredx[n].despx=rejilla.anchura*(pared-rejilla.y)*2+1+x0ant;
         paredx[n].despy=rejilla.anchura*(pared+rejilla.y)+y0ant;
        }
       else
        {
         paredx[n].despx+=2*despeje;
         paredx[n].despy+=despeje-despaltura;
        }
       fm_incorporar_cuadricula(paredx[n].mapa,paredx[n].despx,paredx[n].despy);
      }
     return 0;
   case SUP_Y:
     if(pared>=rejilla.y) fm_error(4);
     if(pared==TODAS)
       if (colocar_mapa)
         for(f=0;f<rejilla.y;f++)
          {
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
           paredy[f].mapa=mapa;
           if(mapa)
            {
             paredy[f].despx=1-rejilla.anchura*(f+1)*2+x0ant;
             paredy[f].despy=rejilla.anchura*(f+1)-mapa->h-1+y0ant;
            }
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
          }
       else
         for(f=0;f<rejilla.y;f++)
          {
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
           paredy[f].despx-=2*despeje;
           paredy[f].despy+=despeje-despaltura;
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
          }
     else
       if (colocar_mapa)
        {
         fm_incorporar_cuadricula(paredy[pared].mapa,paredy[pared].despx,paredy[pared].despy);
         paredy[pared].mapa=mapa;
         if(mapa)
          {
           paredy[pared].despx=1-rejilla.anchura*(pared+1)*2+x0ant;
           paredy[pared].despy=rejilla.anchura*(pared+1)-mapa->h-1+y0ant;
          }
         fm_incorporar_cuadricula(paredy[pared].mapa,paredy[pared].despx,paredy[pared].despy);
        }
       else
        {
         fm_incorporar_cuadricula(paredy[pared].mapa,paredy[pared].despx,paredy[pared].despy);
         paredy[pared].despx-=2*despeje;
         paredy[pared].despy+=despeje-despaltura;
         fm_incorporar_cuadricula(paredy[pared].mapa,paredy[pared].despx,paredy[pared].despy);
        }
     return 0;
   case INF_Y:
     if(pared>=rejilla.y) fm_error(4);
     if(pared==TODAS)
       if (colocar_mapa)
         for(f=rejilla.y;f<(rejilla.y<<1);f++)
          {
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
           paredy[f].mapa=mapa;
           paredy[f].despx=rejilla.anchura*(rejilla.x-f+rejilla.y-1)*2+1+x0ant;
           paredy[f].despy=rejilla.anchura*(f+rejilla.x-rejilla.y)+y0ant;
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
          }
       else
         for(f=rejilla.y;f<(rejilla.y<<1);f++)
          {
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
           paredy[f].despx-=2*despeje;
           paredy[f].despy+=despeje-despaltura;
           fm_incorporar_cuadricula(paredy[f].mapa,paredy[f].despx,paredy[f].despy);
          }
     else
      {
       n=pared+rejilla.y;
       fm_incorporar_cuadricula(paredy[n].mapa,paredy[n].despx,paredy[n].despy);
       if (colocar_mapa)
        {
         paredy[pared+rejilla.y].mapa=mapa;
         paredy[n].despx=rejilla.anchura*(rejilla.x-pared-1)*2+1+x0ant;
         paredy[n].despy=rejilla.anchura*(pared+rejilla.x)+y0ant;
        }
       else
        {
         paredy[n].despx-=2*despeje;
         paredy[n].despy+=despeje-despaltura;
        }
       fm_incorporar_cuadricula(paredy[n].mapa,paredy[n].despx,paredy[n].despy);
      }
     return 0;
   default:
     fm_error(13);
  }   
}

//***************************************************
// Coloca un mapa en una o varias losetas, o la mueve
//***************************************************
int prv_modificar_loseta(BITMAP *mapa, int x, int y, int despx, int despy, short colocar_mapa)
{
 int xini,xfin,yini,yfin;
 int f,n,l;
 
 fm_compr_ini();

 if(x>=rejilla.x||x<TODAS||y>=rejilla.y||y<TODAS) fm_error(2);
  
 if(x==TODAS)
  {
   xini=0;
   xfin=rejilla.x;
  } 
 else
  {
   xini=x;
   xfin=x+1;
  }   

 if(y==TODAS)
  {
   yini=0;
   yfin=rejilla.y;
  } 
 else
  {
   yini=y;
   yfin=y+1;
  }   

 if(colocar_mapa) // Asignar un mapa
  {
   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {
       l=rejilla.x*n+f;
       if(suelo[l].mapa)
        {fm_incorporar_cuadricula(suelo[l].mapa,suelo[l].despx,suelo[l].despy);}
       if(suelo[l].mapa=mapa)
        {
         suelo[l].despx=rejilla.anchura*(f-n-1)*2+1+x0ant;
         suelo[l].despy=rejilla.anchura*(f+n)+y0ant;
         fm_incorporar_cuadricula(suelo[l].mapa,suelo[l].despx,suelo[l].despy);
         if(pctj_sombreado<256)
           suelo[l].sombrear=1;
        }
       else
         suelo[l].sombrear=0;
      }
  }
 else // Cambiar desplazamiento
  {
   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {
       l=rejilla.x*n+f;
       if(suelo[l].mapa)
        {fm_incorporar_cuadricula(suelo[l].mapa,suelo[l].despx,suelo[l].despy);}
       suelo[l].despx+=2*(despy-despx);
       suelo[l].despy+=despx+despy;
       if(suelo[l].mapa)
        {
         fm_incorporar_cuadricula(suelo[l].mapa,suelo[l].despx,suelo[l].despy);
         if(pctj_sombreado<256) suelo[l].sombrear=1;
        }
       else
         suelo[l].sombrear=0;
      }  
  }

 return 0;
}

//***********************************************************************
// Cambia los datos del objeto libre indicado, si no se provocan bloqueos
//***********************************************************************
int prv_mover_objeto_libre(ism_id id, int valor, int x, int y, int z, unsigned char dato, unsigned char modo)
{
 int despx0, despy0, despx1, despy1, anchurax, anchuray, altura;
 int xini, xfin, yini, yfin, xinia, xfina, yinia, yfina;
 int f,n;
 objeto_r *objr=NULL;
 objeto_l *objl=NULL;
 objeto_l *objm=NULL;
 
 prv_vaciar_pila_blq();
 // Se localiza el objeto
 objm=lista_objl;
 while(objm)
  {
   if(objm->id==id) break;
   objm=objm->sig;
  }
 if(!objm) fm_error(10);

 x+=objm->x*modo;
 y+=objm->y*modo;
 z+=objm->z*modo;
 anchurax=objm->anchurax;
 anchuray=objm->anchuray;
 altura=objm->altura;
 switch(dato)
 {
  case D_X:
    x=valor+objm->x*modo;
    if(x==(objm->x)) return 0;
    y=objm->y;
    z=objm->z;
    break;
  case D_Y:
    x=objm->x;
    y=valor+objm->y*modo;
    if(y==(objm->y)) return 0;
    z=objm->z;
    break;
  case D_Z:
    x=objm->x;
    y=objm->y;
    z=valor+objm->z*modo;
    if(z==(objm->z)) return 0;
    break;
  case D_ANCHURA_X:
    anchurax=valor+objm->anchurax*modo;
    if(anchurax<1) fm_error(2);
    if(anchurax==(objm->anchurax)) return 0;
    break;
  case D_ANCHURA_Y:
    anchuray=valor+objm->anchuray*modo;
    if(anchuray<1) fm_error(2);
    if(anchuray==(objm->anchuray)) return 0;
    break;
  case D_ALTURA:
    altura=valor+objm->altura*modo;
    if(altura<0) fm_error(2);
    if(altura==(objm->altura)) return 0;
    break;
  default: // Se cambian las tres coordenadas.
    if((x==(objm->x)) && (y==(objm->y)) && (z==(objm->z))) return 0;
    break;
 }

 xinia=objm->x/rejilla.anchura;                      // 
 xfina=(objm->x+objm->anchurax-1)/rejilla.anchura+1; // Rango de columnas
 yinia=(objm->y-objm->anchuray+1)/rejilla.anchura;   // intersectadas por el objeto
 yfina=objm->y/rejilla.anchura+1;                    // (antes del movimiento)
 xini=x/rejilla.anchura;                // 
 xfin=(x+anchurax-1)/rejilla.anchura+1; // Rango de columnas
 yini=(y-anchuray+1)/rejilla.anchura;   // intersectadas por el objeto
 yfin=y/rejilla.anchura+1;              // (después del movimiento)
 
 // Por si se choca con alguna pared...
 if(xini<0) xini=0;
 if(xfin>rejilla.x) xfin=rejilla.x;
 if(yini<0) yini=0;
 if(yfin>rejilla.y) yfin=rejilla.y;

 // Se buscan bloqueos con las paredes y el suelo
 if(x<0)
   if(!(prv_apilar_objeto_blq(ID_PARED_SUP_Y))) fm_error(3);
 if(x+anchurax>rejilla.x*rejilla.anchura)
   if(!(prv_apilar_objeto_blq(ID_PARED_INF_Y))) fm_error(3);
 if(y>=rejilla.y*rejilla.anchura)
   if(!(prv_apilar_objeto_blq(ID_PARED_INF_X))) fm_error(3);
 if(y-anchuray+1<0)
   if(!(prv_apilar_objeto_blq(ID_PARED_SUP_X))) fm_error(3);
 if(z<0)
   if(!(prv_apilar_objeto_blq(ID_SUELO))) fm_error(3);

 // Si está completamente fuera de la rejilla no hay que buscar más bloqueos
 if((xini>=rejilla.x)||(xfin<0)||(yini>=rejilla.y)||(yfin<0))
   fm_error(7);
 
 // Se recorre la lista de libres buscando bloqueos
 objl=lista_objl;
 while(objl)
  {
   if(objl->id!=id)
   if(((objl->x+objl->anchurax)>x)  &&  (objl->x<(x+anchurax))      && // Intersecta en la dimensión X
       (objl->y>(y-anchuray))       && ((objl->y-objl->anchuray)<y) && // Intersecta en la dimensión Y
      ((objl->z+objl->altura)>z)    &&  (objl->z<(z+altura)))          // Intersecta en la dimensión Z
     if(!(prv_apilar_objeto_blq(objl->id))) fm_error(3);
   objl=objl->sig;
  }

 // Se recorren los objetos de la columna buscando bloqueos
 for(f=xini;f<xfin;f++)
   for(n=yini;n<yfin;n++)
    {
     objr=suelo[rejilla.x*n+f].objr;
     while(objr)
      {
       if(((objr->z+objr->altura)>z)&&(objr->z<(z+altura)))
         if(!(prv_apilar_objeto_blq(objr->id))) fm_error(3);
       objr=objr->sig;
      }
    }

 // Si está bloqueado devuelvo el error
 if(pila_blq) fm_error(7);

 // Se vuelve a recorrer la lista de libres buscando solapamientos, ahora que se que no está bloqueado
 despx0=objm->inf_mapa.despx;
 despy0=objm->inf_mapa.despy;
 if(objm->mapabase)
  {
   despx1=2*(x-y)+anchurax+anchuray-(objm->mapabase->w>>1)-1;
   despy1=x+y+anchurax-objm->mapabase->h-z;
  }
 else  
   despx1=despy1=0;

 objl=lista_objl;
 while(objl)
  {
   if(objl->mapabase && objl->id!=objm->id)
    {
     if(objm->mapabase)
     if(((objl->inf_mapa.despx < despx0+objm->mapabase->w) &&
        (objl->inf_mapa.despx+objl->mapabase->w > despx0) &&
        (objl->inf_mapa.despy < despy0+objm->mapabase->h) &&
        (objl->inf_mapa.despy+objl->mapabase->h > despy0)) || // Su mapa se solapa con el anterior
       ((objl->inf_mapa.despx < despx1+objm->mapabase->w) &&
        (objl->inf_mapa.despx+objl->mapabase->w > despx1) &&
        (objl->inf_mapa.despy < despy1+objm->mapabase->h) &&
        (objl->inf_mapa.despy+objl->mapabase->h > despy1))) // Su mapa se solapa con el actual
       objl->enmascarar=1;

     if(pctj_sombreado<256)
     if((objl->z<=objm->z &&
        objl->x<=objm->x+objm->anchurax && objm->x<=objl->x+objl->anchurax &&
        objl->y>=objm->y-objm->anchuray && objm->y>=objl->y-objl->anchuray) ||
        (objl->z<=z &&
        objl->x<=x+anchurax && x<=objl->x+objl->anchurax &&
        objl->y>=y-anchuray && y>=objl->y-objl->anchuray))
      {
       objl->sombrear=1;
       if(!(objm->mapabase))
         fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);
      }
    }   
   objl=objl->sig;
  } 

 // Si es necesario, se ponen las losetas y objetos rejilla a sombrear
 if((objm->x!=x || objm->y!=y || objm->z!=z) && objm->sombra && pctj_sombreado<256)
  {
   for(f=xinia;f<xfina;f++)
     for(n=yinia;n<yfina;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(objr->z<objm->z)
          {
           if(!(objm->mapabase))
             fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
           objr->sombrear=1;
           objr=objr->sig;
          }
         else
           objr=NULL;  
        }
       suelo[rejilla.x*n+f].sombrear=1;
      } 
   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {
       objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(objr->z<z)
          {
           if(!(objm->mapabase))
             fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
           objr->sombrear=1;
           objr=objr->sig;
          }
         else
           objr=NULL;  
        }
       suelo[rejilla.x*n+f].sombrear=1;
      } 
   suelo[rejilla.x*rejilla.y].sombrear=1;
   fm_incorporar_cuadricula(objm->sombra,2*(x-y)+x0ant+(anchurax+anchuray)-((objm->sombra->w)>>1)-1,
                                   x+y+y0ant+((anchurax-anchuray+1)>>1)-((objm->sombra->h)>>1));
   fm_incorporar_cuadricula(objm->sombra,2*(objm->x-objm->y)+x0ant+(objm->anchurax+objm->anchuray)-((objm->sombra->w)>>1)-1,
                                   objm->x+objm->y+y0ant+((objm->anchurax-objm->anchuray+1)>>1)-((objm->sombra->h)>>1));
  }
 objm->x=x;
 objm->y=y;
 objm->z=z;
 objm->anchurax=anchurax;
 objm->anchuray=anchuray;
 objm->altura=altura;
 objm->inf_mapa.despx=despx1;
 objm->inf_mapa.despy=despy1;
 fm_incorporar_cuadricula(objm->mapabase,despx0+x0ant,despy0+y0ant);
 fm_incorporar_cuadricula(objm->mapabase,despx1+x0ant,despy1+y0ant);
 objm->sombrear=objm->enmascarar=1;
 // Reordenamos el objeto
 if(objm->ant)
  {
   objm->ant->sig=objm->sig;
  }
 else
  {
   prv_ordenar_objeto(objm);
   return 0;
  }
 if(objm->sig) objm->sig->ant=objm->ant;
 objm->ant=NULL;
 objm->sig=lista_objl;
 lista_objl->ant=objm;
 lista_objl=objm;
 prv_ordenar_objeto(objm);

 return 0;
}

#ifdef ALLEGRO_BIG_ENDIAN
#define fm_es_big_endian_y_32(_VAR_) (_VAR_==32?1:0)
#else
#define fm_es_big_endian_y_32(_VAR_) 0
#endif
// Macro para enmascarar mapas no truecolor con mapas no truecolor
#define fm_enmascarar_NT_NT(TIPO1,TIPO2)\
 {\
  unsigned TIPO1 *l1;\
  unsigned TIPO2 *l2;\
  unsigned TIPO1 ctl=(unsigned TIPO1)bitmap_mask_color(objl->mapabase);\
  unsigned TIPO2 ctm=(unsigned TIPO2)bitmap_mask_color(mapa);\
  int n2i=ni+objl->inf_mapa.despx-xm;\
  for(f1=fi, f2=fi+objl->inf_mapa.despy-ym; f1<ff; f1++, f2++)\
   {\
    l1=(unsigned TIPO1 *)mapal->line[f1];\
    l2=(unsigned TIPO2 *)mapa->line[f2];\
    for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
      if(l2[n2]!=ctm) l1[n1]=ctl;\
   }\
 }

// Macro para enmascarar mapas no truecolor con mapas truecolor
#define fm_enmascarar_NT_T(TIPO1)\
 {\
  char aumento2=(prof2==32?4:3);\
  unsigned TIPO1 *l1;\
  unsigned char *l2;\
  unsigned TIPO1 ctl=(unsigned TIPO1)bitmap_mask_color(objl->mapabase);\
  int n2i=ni+objl->inf_mapa.despx-xm;\
  n2i=n2i*aumento2+fm_es_big_endian_y_32(prof2);\
  for(f1=fi, f2=fi+objl->inf_mapa.despy-ym; f1<ff; f1++, f2++)\
   {\
    l1=(unsigned TIPO1 *)mapal->line[f1];\
    l2=(unsigned char *)mapa->line[f2];\
    for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento2)\
      if(l2[n2]!=255 || l2[n2+1]!=0 || l2[n2+2]!=255)\
        l1[n1]=ctl;\
   }\
 }

// Macro para enmascarar mapas truecolor con mapas no truecolor
#define fm_enmascarar_T_NT(TIPO2)\
 {\
  char aumento1=(prof1==32?4:3);\
  unsigned char *l1;\
  unsigned TIPO2 *l2;\
  unsigned TIPO2 ctm=(unsigned TIPO2)bitmap_mask_color(mapa);\
  int n2i=ni+objl->inf_mapa.despx-xm;\
  nf*=aumento1;\
  ni=ni*aumento1+fm_es_big_endian_y_32(prof1);\
  for(f1=fi, f2=fi+objl->inf_mapa.despy-ym; f1<ff; f1++, f2++)\
   {\
    l1=(unsigned char *)mapal->line[f1];\
    l2=(unsigned TIPO2 *)mapa->line[f2];\
    for(n1=ni, n2=n2i; n1<nf; n1+=aumento1, n2++)\
      if(l2[n2]!=ctm)\
        {l1[n1]=l1[n1+2]=255;l1[n1+1]=0;}\
   }\
 }

// Macro para enmascarar mapas truecolor con mapas truecolor
#define fm_enmascarar_T_T()\
 {\
  char aumento1=(prof1==32?4:3);\
  char aumento2=(prof2==32?4:3);\
  unsigned char *l1;\
  unsigned char *l2;\
  int n2i=ni+objl->inf_mapa.despx-xm;\
  nf*=aumento1;\
  ni=ni*aumento1+fm_es_big_endian_y_32(prof1);\
  n2i=n2i*aumento2+fm_es_big_endian_y_32(prof2);\
  for(f1=fi, f2=fi+objl->inf_mapa.despy-ym; f1<ff; f1++, f2++)\
   {\
    l1=(unsigned char *)mapal->line[f1];\
    l2=(unsigned char *)mapa->line[f2];\
    for(n1=ni, n2=n2i; n1<nf; n1+=aumento1, n2+=aumento2)\
      if(l2[n2]!=255 || l2[n2+1]!=0 || l2[n2+2]!=255)\
        {l1[n1]=l1[n1+2]=255;l1[n1+1]=0;}\
   }\
 }

//*****************************************************
// Enmascara el mapa de objl con los atos suministrados 
//*****************************************************
void prv_enmascarar(objeto_l *objl, BITMAP *mapa, int xm, int ym)
{
 int fi=ym-objl->inf_mapa.despy;
 int ni=xm-objl->inf_mapa.despx;
 int ff=ym+mapa->h-objl->inf_mapa.despy;
 int nf=xm+mapa->w-objl->inf_mapa.despx;
 int f1,f2,n1,n2;
 BITMAP *mapal=(objl->mapasomb?objl->mapasomb:objl->mapabase);
 int prof1=bitmap_color_depth(objl->mapabase);
 int prof2=bitmap_color_depth(mapa);
 char caso=(prof1==8?0:(prof1<24?3:6))+(prof2==8?0:(prof2<24?1:2));

 if(fi<0) fi=0;
 if(ni<0) ni=0;
 if(ff>mapal->h) ff=mapal->h;
 if(nf>mapal->w) nf=mapal->w;
 
 if(!objl->inf_mapa.mapa)
   objl->inf_mapa.mapa=create_bitmap_ex(bitmap_color_depth(mapal), mapal->w, mapal->h);

 if(objl->enmascarar==1)
  {
   blit(mapal,objl->inf_mapa.mapa,0,0,0,0,mapal->w,mapal->h);
   objl->enmascarar=2;
  } 

 mapal=objl->inf_mapa.mapa;
 switch(caso)
  {
   // Los casos 5 y 7 no tienen mucho sentido en Allegro, por eso están comentados.
   case 0: fm_enmascarar_NT_NT(char,char);   break;
   case 1: fm_enmascarar_NT_NT(char,short);  break;
   case 2: fm_enmascarar_NT_T(char);         break;
   case 3: fm_enmascarar_NT_NT(short,char);  break;
   case 4: fm_enmascarar_NT_NT(short,short); break;
// case 5: fm_enmascarar_NT_T(short);        break;
   case 6: fm_enmascarar_T_NT(char);         break;
// case 7: fm_enmascarar_T_NT(short);        break;
   case 8: fm_enmascarar_T_T();              break;
  }
}
#undef fm_enmascarar_NT_NT
#undef fm_enmascarar_NT_T
#undef fm_enmascarar_T_NT
#undef fm_enmascarar_T_T

// Macro para sombrear losetas hicolor con sombras no truecolor
#define fm_sombrear_l_NT_NT(TIPO)\
 {\
  int ctl=bitmap_mask_color(suelo[loseta].mapa);\
  int cts=bitmap_mask_color(sombra);\
  unsigned short *ll;\
  unsigned short *lls;\
  unsigned TIPO  *ls;\
  char pctj_sombreado_32_t=(char)(((32.0-pctj_sombreado_32)/100)*transp+pctj_sombreado_32);\
  if(pctj_sombreado_32_t==2 || pctj_sombreado_32_t==4 || pctj_sombreado_32_t==8 || pctj_sombreado_32_t==16)\
   {char d=4;\
    unsigned long color;\
    unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
    while(pctj_sombreado_32_t!=2)\
     {\
      pctj_sombreado_32_t=pctj_sombreado_32_t>>1;\
      d--;\
     }\
    for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
     {\
      ls =(unsigned TIPO  *)sombra->line[f2];\
      ll =(unsigned short *)mapal->line[f1];\
      lls=(unsigned short *)mapals->line[f1];\
      for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
        if(ll[n1]!=ctl && ll[n1]==lls[n1] && ls[n2]!=cts)\
         {\
          color=ll[n1];\
          color=(color|(color<<16)) & mascara;\
          color=(color>>d) & mascara;\
          color=color|(color>>16);\
          lls[n1]=(unsigned short)color;\
         }\
     }\
   }\
  else\
   {\
    if(pctj_sombreado_32_t)\
     {unsigned long color;\
      unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {\
        ls =(unsigned TIPO  *)sombra->line[f2];\
        ll =(unsigned short *)mapal->line[f1];\
        lls=(unsigned short *)mapals->line[f1];\
        for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
          if(ll[n1]!=ctl && ll[n1]==lls[n1] && ls[n2]!=cts)\
           {\
            color=ll[n1];\
            color=(color|(color<<16)) & mascara;\
            color=((color*pctj_sombreado_32_t)>>5) & mascara;\
            color=color|(color>>16);\
            lls[n1]=(unsigned short)color;\
           }\
       }\
     }\
    else\
     {\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {\
        ls =(unsigned TIPO  *)sombra->line[f2];\
        ll =(unsigned short *)mapal->line[f1];\
        lls=(unsigned short *)mapals->line[f1];\
        for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
          if(ll[n1]!=ctl && ll[n1]==lls[n1] && ls[n2]!=cts)\
            lls[n1]=0;\
       }\
     }\
   }\
 }      

// Macro para sombrear losetas hicolor con sombras truecolor
#define fm_sombrear_l_NT_T()\
 {\
  char aumento=(prof2==32?4:3);\
  int ctl=bitmap_mask_color(suelo[loseta].mapa);\
  unsigned short *ll;\
  unsigned short *lls;\
  unsigned char  *ls;\
  char pctj_sombreado_32_t=(char)(((32.0-pctj_sombreado_32)/100)*transp+pctj_sombreado_32);\
  n2i=n2i*aumento+fm_es_big_endian_y_32(prof2);\
  if(pctj_sombreado_32_t==2 || pctj_sombreado_32_t==4 || pctj_sombreado_32_t==8 || pctj_sombreado_32_t==16)\
   {unsigned long color;\
    unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
    char d=4;\
    while(pctj_sombreado_32_t!=2)\
     {\
      pctj_sombreado_32_t=pctj_sombreado_32_t>>1;\
      d--;\
     }\
    for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
     {\
      ls =(unsigned char  *)sombra->line[f2];\
      ll =(unsigned short *)mapal->line[f1];\
      lls=(unsigned short *)mapals->line[f1];\
      for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
        if(ll[n1]!=ctl && ll[n1]==lls[n1] && (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
         {\
          color=ll[n1];\
          color=(color|(color<<16)) & mascara;\
          color=(color>>d) & mascara;\
          color=color|(color>>16);\
          lls[n1]=(unsigned short)color;\
         }\
     }\
   }\
  else\
   {\
    if(pctj_sombreado_32_t)\
     {unsigned long color;\
      unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {\
        ls =(unsigned char  *)sombra->line[f2];\
        ll =(unsigned short *)mapal->line[f1];\
        lls=(unsigned short *)mapals->line[f1];\
        for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
          if(ll[n1]!=ctl && ll[n1]==lls[n1] && (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
           {\
            color=ll[n1];\
            color=(color|(color<<16)) & mascara;\
            color=((color*pctj_sombreado_32_t)>>5) & mascara;\
            color=color|(color>>16);\
            lls[n1]=(unsigned short)color;\
           }\
       }\
     }\
    else\
     {\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {\
        ls =(unsigned char  *)sombra->line[f2];\
        ll =(unsigned short *)mapal->line[f1];\
        lls=(unsigned short *)mapals->line[f1];\
        for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
          if(ll[n1]!=ctl && ll[n1]==lls[n1] && (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
            lls[n1]=0;\
       }\
     }\
   }\
 }      

// Macro para sombrear losetas truecolor con sombras no truecolor
#define fm_sombrear_l_T_NT(TIPO)\
 {\
  char aumento=(prof1==32?4:3);\
  int cts=bitmap_mask_color(sombra);\
  unsigned char *ll;\
  unsigned char *lls;\
  unsigned TIPO *ls;\
  int n1m1;\
  int n1m2;\
  short pctj_sombreado_t=(short)(((256.0-pctj_sombreado)/100)*transp+pctj_sombreado);\
  nf*=aumento;\
  ni=ni*aumento+fm_es_big_endian_y_32(prof1);\
  if(pctj_sombreado_t==2  || pctj_sombreado_t==4  || pctj_sombreado_t==8 || pctj_sombreado_t==16 ||\
     pctj_sombreado_t==32 || pctj_sombreado_t==64 || pctj_sombreado_t==128)\
   {\
    char d=7;\
    while(pctj_sombreado_t!=2)\
     {\
      pctj_sombreado_t=pctj_sombreado_t>>1;\
      d--;\
     }\
    for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
     {\
      ls =(unsigned TIPO *)sombra->line[f2];\
      ll =(unsigned char *)mapal->line[f1];\
      lls=(unsigned char *)mapals->line[f1];\
      for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento, n1m1+=aumento, n1m2+=aumento, n2++)\
        if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
           (ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2]) && ls[n2]!=cts)\
         {\
          lls[n1]=ll[n1]>>d;\
          lls[n1+1]=ll[n1+1]>>d;\
          lls[n1+2]=ll[n1+2]>>d;\
         }\
     }\
   }\
  else\
   {\
    if(pctj_sombreado_t)\
     {\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {unsigned short color;\
        ls =(unsigned TIPO *)sombra->line[f2];\
        ll =(unsigned char *)mapal->line[f1];\
        lls=(unsigned char *)mapals->line[f1];\
        for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento, n1m1+=aumento, n1m2+=aumento, n2++)\
          if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             (ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2]) && ls[n2]!=cts)\
           {\
            color=ll[n1]*pctj_sombreado_t;\
            lls[n1]=(unsigned char)(color>>8);\
            color=ll[n1+1]*pctj_sombreado_t;\
            lls[n1+1]=(unsigned char)(color>>8);\
            color=ll[n1+2]*pctj_sombreado_t;\
            lls[n1+2]=(unsigned char)(color>>8);\
           }\
       }\
     }\
    else\
     {\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {\
        ls =(unsigned TIPO  *)sombra->line[f2];\
        ll =(unsigned char *)mapal->line[f1];\
        lls=(unsigned char *)mapals->line[f1];\
        for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento, n1m1+=aumento, n1m2+=aumento, n2++)\
          if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             (ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2]) && ls[n2]!=cts)\
            lls[n1]=lls[n1+1]=lls[n1+2]=0;\
       }\
     }\
   }\
 }      

// Macro para sombrear losetas truecolor con sombras truecolor
#define fm_sombrear_l_T_T()\
 {\
  char aumento1=(prof1==32?4:3);\
  char aumento2=(prof2==32?4:3);\
  unsigned char *ll;\
  unsigned char *lls;\
  unsigned char *ls;\
  int n1m1;\
  int n1m2;\
  short pctj_sombreado_t=(short)(((256.0-pctj_sombreado)/100)*transp+pctj_sombreado);\
  nf*=aumento1;\
  ni=ni*aumento1+fm_es_big_endian_y_32(prof1);\
  n2i=n2i*aumento2+fm_es_big_endian_y_32(prof2);\
  if(pctj_sombreado_t==2  || pctj_sombreado_t==4  || pctj_sombreado_t==8 || pctj_sombreado_t==16 ||\
     pctj_sombreado_t==32 || pctj_sombreado_t==64 || pctj_sombreado_t==128)\
   {\
    char d=7;\
    while(pctj_sombreado_t!=2)\
     {\
      pctj_sombreado_t=pctj_sombreado_t>>1;\
      d--;\
     }\
    for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
     {\
      ls =(unsigned char *)sombra->line[f2];\
      ll =(unsigned char *)mapal->line[f1];\
      lls=(unsigned char *)mapals->line[f1];\
      for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
        if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
           (ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2]) && \
           (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
         {\
          lls[n1]=ll[n1]>>d;\
          lls[n1m1]=ll[n1m1]>>d;\
          lls[n1m2]=ll[n1m2]>>d;\
         }\
     }\
   }\
  else\
   {\
    if(pctj_sombreado_t)\
     {\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {unsigned short color;\
        ls =(unsigned char *)sombra->line[f2];\
        ll =(unsigned char *)mapal->line[f1];\
        lls=(unsigned char *)mapals->line[f1];\
        for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
          if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             (ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2]) && \
             (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
           {\
            color=ll[n1]*pctj_sombreado_t;\
            lls[n1]=(unsigned char)(color>>8);\
            color=ll[n1m1]*pctj_sombreado_t;\
            lls[n1m1]=(unsigned char)(color>>8);\
            color=ll[n1m2]*pctj_sombreado_t;\
            lls[n1m2]=(unsigned char)(color>>8);\
           }\
       }\
     }\
    else\
     {\
      for(f1=fi, f2=fi+suelo[loseta].despy-y; f1<ff; f1++, f2++)\
       {\
        ls =(unsigned char *)sombra->line[f2];\
        ll =(unsigned char *)mapal->line[f1];\
        lls=(unsigned char *)mapals->line[f1];\
        for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
          if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             (ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2]) &&\
             (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
            lls[n1]=lls[n1+1]=lls[n1+2]=0;\
       }\
     }\
   }\
 }      

//***********************************
// Oscurece una loseta con una sombra
//***********************************
void prv_sombrear_loseta(int loseta, int x, int y, BITMAP *sombra, char transp)
{
 int prof1=bitmap_color_depth(suelo[loseta].mapa);
 if(prof1>8 && transp<100)
  {
   int fi=y-suelo[loseta].despy;
   int ni=x-suelo[loseta].despx;
   int ff=y+sombra->h-suelo[loseta].despy;
   int nf=x+sombra->w-suelo[loseta].despx;
   int f1,f2,n1,n2;
   BITMAP *mapal=NULL;
   BITMAP *mapals=NULL;

   if(fi<0) fi=0;
   if(ni<0) ni=0;
   if(ff>suelo[loseta].mapa->h) ff=suelo[loseta].mapa->h;
   if(nf>suelo[loseta].mapa->w) nf=suelo[loseta].mapa->w;

   if(fi<ff && ni<nf)
    {
     int prof2=bitmap_color_depth(sombra);
     int n2i=ni+suelo[loseta].despx-x;
     char caso=(prof1<24?0:3)+(prof2==8?0:(prof2<24?1:2));

     if(!suelo[loseta].mapasomb)
       suelo[loseta].mapasomb=create_bitmap_ex(bitmap_color_depth(suelo[loseta].mapa),
                                               suelo[loseta].mapa->w,
                                               suelo[loseta].mapa->h);
     if(suelo[loseta].sombrear==1)
      {
       blit(suelo[loseta].mapa,suelo[loseta].mapasomb,0,0,0,0,suelo[loseta].mapa->w,suelo[loseta].mapa->h);
       suelo[loseta].sombrear=2;
      } 
 
     mapal=suelo[loseta].mapa;
     mapals=suelo[loseta].mapasomb;

     switch(caso)
      {
       case 0: fm_sombrear_l_NT_NT(char);  break;
       case 1: fm_sombrear_l_NT_NT(short); break;
       case 2: fm_sombrear_l_NT_T();       break;
       case 3: fm_sombrear_l_T_NT(char);   break;
       case 4: fm_sombrear_l_T_NT(short);  break;
       case 5: fm_sombrear_l_T_T();        break;
      }
    }
  }        
}

#undef fm_sombrear_l_NT_NT
#undef fm_sombrear_l_NT_T
#undef fm_sombrear_l_T_NT
#undef fm_sombrear_l_T_T

// Macro para sombrear objetos hicolor con sombras no truecolor
#define fm_sombrear_o_NT_NT(TIPO)\
{\
 unsigned short *ll, *lm, *ln, *lls;\
 unsigned TIPO  *ls;\
 char pctj_sombreado_32_t=(char)(((32.0-pctj_sombreado_32)/100)*transp+pctj_sombreado_32);\
 if(pctj_sombreado_32_t==2 || pctj_sombreado_32_t==4 || pctj_sombreado_32_t==8 || pctj_sombreado_32_t==16)\
  {unsigned long color;\
   unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
   char d=4;\
   while(pctj_sombreado_32_t!=2)\
    {\
     pctj_sombreado_32_t=pctj_sombreado_32_t>>1;\
     d--;\
    }\
   for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
    {\
     ls =(unsigned TIPO  *)sombra->line[f2];\
     ll =(unsigned short *)mapal->line[f1];\
     lls=(unsigned short *)mapals->line[f1];\
     for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
       if(ll[n1]!=ctl && ll[n1]==lls[n1] && ls[n2]!=cts)\
        {\
         color=ll[n1];\
         color=(color|(color<<16)) & mascara;\
         color=(color>>d) & mascara;\
         color=color|(color>>16);\
         lls[n1]=(unsigned short)color;\
        }\
    }\
   x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
   x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
   for(x1m1=x1+1, x2m1=x2+1;f1<ff; f1++, f2++, x1+=2, x1m1+=2, x2-=2, x2m1-=2)\
    {\
     ls =(unsigned TIPO  *)sombra->line[f2];\
     ll =(unsigned short *)mapal->line[f1];\
     lls=(unsigned short *)mapals->line[f1];\
     if(ni<x1) ni=x1;\
     if(nf>x2+2) nf=x2+2;\
     for(n1=ni, n2=ni+inf_mapa.despx-x; n1<nf; n1++, n2++)\
       if(ls[n2]!=cts)\
        {\
         if(ll[n1]!=ctl && ll[n1]==lls[n1])\
          {\
           color=ll[n1];\
           color=(color|(color<<16)) & mascara;\
           color=(color>>d) & mascara;\
           color=color|(color>>16);\
           lls[n1]=(unsigned short)color;\
          }\
         if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
           for(y2=f1+1;y2<mapal->h;y2++)\
            {\
             lm=(unsigned short *)mapal->line[y2];\
             ln=(unsigned short *)mapals->line[y2];\
             if(lm[n1]!=ctl && lm[n1]==ln[n1])\
              {\
               color=lm[n1];\
               color=(color|(color<<16)) & mascara;\
               color=(color>>d) & mascara;\
               color=color|(color>>16);\
               ln[n1]=(unsigned short)color;\
              }\
            }\
        }\
    }\
  }\
 else\
  {\
   if(pctj_sombreado_32_t)\
    {unsigned long color;\
     unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned TIPO  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
         if(ll[n1]!=ctl && ll[n1]==lls[n1] && ls[n2]!=cts)\
          {\
           color=ll[n1];\
           color=(color|(color<<16)) & mascara;\
           color=((color*pctj_sombreado_32_t)>>5) & mascara;\
           color=color|(color>>16);\
           lls[n1]=(unsigned short)color;\
          }\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     for(x1m1=x1+1, x2m1=x2+1;f1<ff; f1++, f2++, x1+=2, x1m1+=2, x2-=2, x2m1-=2)\
      {\
       ls =(unsigned TIPO  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       if(ni<x1) ni=x1;\
       if(nf>x2+2) nf=x2+2;\
       for(n1=ni, n2=ni+inf_mapa.despx-x; n1<nf; n1++, n2++)\
         if(ls[n2]!=cts)\
          {\
           if(ll[n1]!=ctl && ll[n1]==lls[n1])\
            {\
             color=ll[n1];\
             color=(color|(color<<16)) & mascara;\
             color=((color*pctj_sombreado_32_t)>>5) & mascara;\
             color=color|(color>>16);\
             lls[n1]=(unsigned short)color;\
            }\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned short *)mapal->line[y2];\
               ln=(unsigned short *)mapals->line[y2];\
               if(lm[n1]!=ctl && lm[n1]==ln[n1])\
                {\
                 color=lm[n1];\
                 color=(color|(color<<16)) & mascara;\
                 color=((color*pctj_sombreado_32_t)>>5) & mascara;\
                 color=color|(color>>16);\
                 ln[n1]=(unsigned short)color;\
                }\
              }\
          }\
      }\
    }\
   else\
    {\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned TIPO  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       for(n1=ni, n2=n2i; n1<nf; n1++, n2++)\
         if(ll[n1]!=ctl && ll[n1]==lls[n1] && ls[n2]!=cts)\
           lls[n1]=0;\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     for(x1m1=x1+1, x2m1=x2+1;f1<ff; f1++, f2++, x1+=2, x1m1+=2, x2-=2, x2m1-=2)\
      {\
       ls =(unsigned TIPO  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       if(ni<x1) ni=x1;\
       if(nf>x2+2) nf=x2+2;\
       for(n1=ni, n2=ni+inf_mapa.despx-x; n1<nf; n1++, n2++)\
         if(ls[n2]!=cts)\
          {\
           if(ll[n1]!=ctl && ll[n1]==lls[n1])\
             lls[n1]=0;\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned short *)mapal->line[y2];\
               ln=(unsigned short *)mapals->line[y2];\
               if(lm[n1]!=ctl && lm[n1]==ln[n1])\
                 ln[n1]=0;\
              }\
          }\
      }\
    }\
  }\
}

// Macro para sombrear objetos hicolor con sombras truecolor
#define fm_sombrear_o_NT_T()\
{\
 unsigned short *ll;\
 unsigned short *lm;\
 unsigned short *ln;\
 unsigned short *lls;\
 unsigned char  *ls;\
 char aumento=(prof2==32?4:3);\
 char pctj_sombreado_32_t=(char)(((32.0-pctj_sombreado_32)/100)*transp+pctj_sombreado_32);\
 n2i=n2i*aumento+fm_es_big_endian_y_32(prof2);\
 if(pctj_sombreado_32_t==2 || pctj_sombreado_32_t==4 || pctj_sombreado_32_t==8 || pctj_sombreado_32_t==16)\
  {unsigned long color;\
   unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
   char d=4;\
   while(pctj_sombreado_32_t!=2)\
    {\
     pctj_sombreado_32_t=pctj_sombreado_32_t>>1;\
     d--;\
    }\
   for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
    {\
     ls =(unsigned char  *)sombra->line[f2];\
     ll =(unsigned short *)mapal->line[f1];\
     lls=(unsigned short *)mapals->line[f1];\
     for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
       if(ll[n1]!=ctl && ll[n1]==lls[n1] && (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
        {\
         color=ll[n1];\
         color=(color|(color<<16)) & mascara;\
         color=(color>>d) & mascara;\
         color=color|(color>>16);\
         lls[n1]=(unsigned short)color;\
        }\
    }\
   x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
   x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
   for(x1m1=x1+1, x2m1=x2+1;f1<ff; f1++, f2++, x1+=2, x1m1+=2, x2-=2, x2m1-=2)\
    {\
     ls =(unsigned char  *)sombra->line[f2];\
     ll =(unsigned short *)mapal->line[f1];\
     lls=(unsigned short *)mapals->line[f1];\
     if(ni<x1) {ni=x1; n2i=(ni+inf_mapa.despx-x)*aumento+fm_es_big_endian_y_32(prof2);}\
     if(nf>x2+2) nf=x2+2;\
     for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
       if(ls[n2]<255 || ls[n2+1] || ls[n2+2]<255)\
        {\
         if(ll[n1]!=ctl && ll[n1]==lls[n1])\
          {\
           color=ll[n1];\
           color=(color|(color<<16)) & mascara;\
           color=(color>>d) & mascara;\
           color=color|(color>>16);\
           lls[n1]=(unsigned short)color;\
          }\
         if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
           for(y2=f1+1;y2<mapal->h;y2++)\
            {\
             lm=(unsigned short *)mapal->line[y2];\
             ln=(unsigned short *)mapals->line[y2];\
             if(lm[n1]!=ctl && lm[n1]==ln[n1])\
              {\
               color=lm[n1];\
               color=(color|(color<<16)) & mascara;\
               color=(color>>d) & mascara;\
               color=color|(color>>16);\
               ln[n1]=(unsigned short)color;\
              }\
            }\
        }\
    }\
  }\
 else\
  {\
   if(pctj_sombreado_32_t)\
    {unsigned long color;\
     unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned char  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
         if(ll[n1]!=ctl && ll[n1]==lls[n1] && (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
          {\
           color=ll[n1];\
           color=(color|(color<<16)) & mascara;\
           color=((color*pctj_sombreado_32_t)>>5) & mascara;\
           color=color|(color>>16);\
           lls[n1]=(unsigned short)color;\
          }\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     for(x1m1=x1+1, x2m1=x2+1;f1<ff; f1++, f2++, x1+=2, x1m1+=2, x2-=2, x2m1-=2)\
      {\
       ls =(unsigned char  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       if(ni<x1) {ni=x1; n2i=(ni+inf_mapa.despx-x)*aumento+fm_es_big_endian_y_32(prof2);}\
       if(nf>x2+2) nf=x2+2;\
       for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
         if(ls[n2]<255 || ls[n2+1] || ls[n2+2]<255)\
          {\
           if(ll[n1]!=ctl && ll[n1]==lls[n1])\
            {\
             color=ll[n1];\
             color=(color|(color<<16)) & mascara;\
             color=((color*pctj_sombreado_32_t)>>5) & mascara;\
             color=color|(color>>16);\
             lls[n1]=(unsigned short)color;\
            }\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned short *)mapal->line[y2];\
               ln=(unsigned short *)mapals->line[y2];\
               if(lm[n1]!=ctl && lm[n1]==ln[n1])\
                {\
                 color=lm[n1];\
                 color=(color|(color<<16)) & mascara;\
                 color=((color*pctj_sombreado_32_t)>>5) & mascara;\
                 color=color|(color>>16);\
                 ln[n1]=(unsigned short)color;\
                }\
              }\
          }\
      }\
    }\
   else\
    {\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned char  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
         if(ll[n1]!=ctl && ll[n1]==lls[n1] && (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
           lls[n1]=0;\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     for(x1m1=x1+1, x2m1=x2+1;f1<ff; f1++, f2++, x1+=2, x1m1+=2, x2-=2, x2m1-=2)\
      {\
       ls =(unsigned char  *)sombra->line[f2];\
       ll =(unsigned short *)mapal->line[f1];\
       lls=(unsigned short *)mapals->line[f1];\
       if(ni<x1) {ni=x1; n2i=(ni+inf_mapa.despx-x)*aumento+fm_es_big_endian_y_32(prof2);}\
       if(nf>x2+2) nf=x2+2;\
       for(n1=ni, n2=n2i; n1<nf; n1++, n2+=aumento)\
         if(ls[n2]<255 || ls[n2+1] || ls[n2+2]<255)\
          {\
           if(ll[n1]!=ctl && ll[n1]==lls[n1])\
             lls[n1]=0;\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned short *)mapal->line[y2];\
               ln=(unsigned short *)mapals->line[y2];\
               if(lm[n1]!=ctl && lm[n1]==ln[n1])\
                 ln[n1]=0;\
              }\
          }\
      }\
    }\
  }\
}

// Macro para sombrear objetos truecolor con sombras no truecolor
#define fm_sombrear_o_T_NT(TIPO)\
{\
 unsigned char *ll, *lm, *ln, *lls;\
 unsigned TIPO *ls;\
 int n1m1, n1m2;\
 char aumento1=(prof1==32?4:3);\
 short pctj_sombreado_t=(short)(((256.0-pctj_sombreado)/100)*transp+pctj_sombreado);\
 nf=nf*aumento1+fm_es_big_endian_y_32(prof1);\
 ni=ni*aumento1+fm_es_big_endian_y_32(prof1);\
 if(pctj_sombreado_t==2  || pctj_sombreado_t==4  || pctj_sombreado_t==8 || pctj_sombreado_t==16 ||\
    pctj_sombreado_t==32 || pctj_sombreado_t==64 || pctj_sombreado_t==128)\
  {\
   char d=7;\
   while(pctj_sombreado_t!=2)\
    {\
     pctj_sombreado_t=pctj_sombreado_t>>1;\
     d--;\
    }\
   for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
    {\
     ls =(unsigned TIPO *)sombra->line[f2];\
     ll =(unsigned char *)mapal->line[f1];\
     lls=(unsigned char *)mapals->line[f1];\
     for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2++)\
     {\
       if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
           ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2] && ls[n2]!=cts)\
        {\
         lls[n1]=ll[n1]>>d;\
         lls[n1m1]=ll[n1m1]>>d;\
         lls[n1m2]=ll[n1m2]>>d;\
        }\
      }\
    }\
   x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
   x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
   x1=x1*aumento1+fm_es_big_endian_y_32(prof1);\
   x2=x2*aumento1+fm_es_big_endian_y_32(prof1);\
   for(x1m1=x1+aumento1, x2m1=x2+aumento1; f1<ff; f1++, f2++, x1+=2*aumento1, x1m1+=2*aumento1, x2-=2*aumento1, x2m1-=2*aumento1)\
    {\
     ls =(unsigned TIPO *)sombra->line[f2];\
     ll =(unsigned char *)mapal->line[f1];\
     lls=(unsigned char *)mapals->line[f1];\
     if(ni<x1) {n2i+=(x1-ni)/aumento1;ni=x1;}\
     if(nf>x2+2*aumento1) nf=x2+2*aumento1;\
     for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2++)\
       if(ls[n2]!=cts)\
        {\
         if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2])\
          {\
           lls[n1]=ll[n1]>>d;\
           lls[n1m1]=ll[n1m1]>>d;\
           lls[n1m2]=ll[n1m2]>>d;\
          }\
         if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
           for(y2=f1+1;y2<mapal->h;y2++)\
            {\
             lm=(unsigned char *)mapal->line[y2];\
             ln=(unsigned char *)mapals->line[y2];\
             if((lm[n1]<255 || lm[n1m1] || lm[n1m2]<255) &&\
                 lm[n1]==ln[n1] && lm[n1m1]==ln[n1m1] && lm[n1m2]==ln[n1m2])\
              {\
               lls[n1]=ll[n1]>>d;\
               lls[n1m1]=ll[n1m1]>>d;\
               lls[n1m2]=ll[n1m2]>>d;\
              }\
            }\
        }\
    }\
  }\
 else\
  {\
   if(pctj_sombreado_t)\
    {unsigned short color;\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned TIPO *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2++)\
       {\
         if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2] && ls[n2]!=cts)\
          {\
           color=ll[n1]*pctj_sombreado_t;\
           lls[n1]=(unsigned char)(color>>8);\
           color=ll[n1m1]*pctj_sombreado_t;\
           lls[n1m1]=(unsigned char)(color>>8);\
           color=ll[n1m2]*pctj_sombreado_t;\
           lls[n1m2]=(unsigned char)(color>>8);\
          }\
        }\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     x1=x1*aumento1+fm_es_big_endian_y_32(prof1);\
     x2=x2*aumento1+fm_es_big_endian_y_32(prof1);\
     for(x1m1=x1+aumento1, x2m1=x2+aumento1; f1<ff; f1++, f2++, x1+=2*aumento1, x1m1+=2*aumento1, x2-=2*aumento1, x2m1-=2*aumento1)\
      {\
       ls =(unsigned TIPO *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       if(ni<x1) {n2i+=(x1-ni)/aumento1;ni=x1;}\
       if(nf>x2+2*aumento1) nf=x2+2*aumento1;\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2++)\
         if(ls[n2]!=cts)\
          {\
           if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
               ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2])\
            {\
             color=ll[n1]*pctj_sombreado_t;\
             lls[n1]=(unsigned char)(color>>8);\
             color=ll[n1m1]*pctj_sombreado_t;\
             lls[n1m1]=(unsigned char)(color>>8);\
             color=ll[n1m2]*pctj_sombreado_t;\
             lls[n1m2]=(unsigned char)(color>>8);\
            }\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned char *)mapal->line[y2];\
               ln=(unsigned char *)mapals->line[y2];\
               if((lm[n1]<255 || lm[n1m1] || lm[n1m2]<255) &&\
                   lm[n1]==ln[n1] && lm[n1m1]==ln[n1m1] && lm[n1m2]==ln[n1m2])\
                {\
                 color=lm[n1]*pctj_sombreado_t;\
                 ln[n1]=(unsigned char)(color>>8);\
                 color=lm[n1m1]*pctj_sombreado_t;\
                 ln[n1m1]=(unsigned char)(color>>8);\
                 color=lm[n1m2]*pctj_sombreado_t;\
                 ln[n1m2]=(unsigned char)(color>>8);\
                }\
              }\
          }\
      }\
    }\
   else\
    {\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned TIPO *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2++)\
         if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2] && ls[n2]!=cts)\
           lls[n1]=lls[n1m1]=lls[n1m2]=0;\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     x1=x1*aumento1+fm_es_big_endian_y_32(prof1);\
     x2=x2*aumento1+fm_es_big_endian_y_32(prof1);\
     for(x1m1=x1+aumento1, x2m1=x2+aumento1; f1<ff; f1++, f2++, x1+=2*aumento1, x1m1+=2*aumento1, x2-=2*aumento1, x2m1-=2*aumento1)\
      {\
       ls =(unsigned TIPO *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       if(ni<x1) {n2i+=(x1-ni)/aumento1;ni=x1;}\
       if(nf>x2+2*aumento1) nf=x2+2*aumento1;\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2++)\
         if(ls[n2]!=cts)\
          {\
           if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
               ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2])\
             lls[n1]=lls[n1m1]=lls[n1m2]=0;\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned char *)mapal->line[y2];\
               ln=(unsigned char *)mapals->line[y2];\
               if((lm[n1]<255 || lm[n1m1] || lm[n1m2]<255) &&\
                   lm[n1]==ln[n1] && lm[n1m1]==ln[n1m1] && lm[n1m2]==ln[n1m2])\
                 ln[n1]=ln[n1m1]=ln[n1m2]=0;\
              }\
          }\
      }\
    }\
  }\
} 

// Macro para sombrear objetos truecolor con sombras truecolor
#define fm_sombrear_o_T_T()\
{\
 unsigned char *ll, *lm, *ln, *lls, *ls;\
 int n1m1, n1m2;\
 char aumento1=(prof1==32?4:3), aumento2=(prof2==32?4:3);\
 short pctj_sombreado_t=(short)(((256.0-pctj_sombreado)/100)*transp+pctj_sombreado);\
 nf=nf*aumento1+fm_es_big_endian_y_32(prof1);\
 ni=ni*aumento1+fm_es_big_endian_y_32(prof1);\
 n2i=n2i*aumento2+fm_es_big_endian_y_32(prof2);\
 if(pctj_sombreado_t==2  || pctj_sombreado_t==4  || pctj_sombreado_t==8 || pctj_sombreado_t==16 ||\
    pctj_sombreado_t==32 || pctj_sombreado_t==64 || pctj_sombreado_t==128)\
  {\
   char d=7;\
   while(pctj_sombreado_t!=2)\
    {\
     pctj_sombreado_t=pctj_sombreado_t>>1;\
     d--;\
    }\
   for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
    {\
     ls =(unsigned char *)sombra->line[f2];\
     ll =(unsigned char *)mapal->line[f1];\
     lls=(unsigned char *)mapals->line[f1];\
     for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
     {\
       if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
           ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2] &&\
         (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
        {\
         lls[n1]=ll[n1]>>d;\
         lls[n1m1]=ll[n1m1]>>d;\
         lls[n1m2]=ll[n1m2]>>d;\
        }\
      }\
    }\
   x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
   x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
   x1=x1*aumento1+fm_es_big_endian_y_32(prof1);\
   x2=x2*aumento1+fm_es_big_endian_y_32(prof1);\
   for(x1m1=x1+aumento1, x2m1=x2+aumento1; f1<ff; f1++, f2++, x1+=2*aumento1, x1m1+=2*aumento1, x2-=2*aumento1, x2m1-=2*aumento1)\
    {\
     ls =(unsigned char *)sombra->line[f2];\
     ll =(unsigned char *)mapal->line[f1];\
     lls=(unsigned char *)mapals->line[f1];\
     if(ni<x1) {ni=x1;n2i=ni+(inf_mapa.despx-x)*aumento2+fm_es_big_endian_y_32(prof2);}\
     if(nf>x2+2*aumento1) nf=x2+2*aumento1;\
     for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
       if(ls[n2]<255 || ls[n2+1] || ls[n2+2]<255)\
        {\
         if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2])\
          {\
           lls[n1]=ll[n1]>>d;\
           lls[n1m1]=ll[n1m1]>>d;\
           lls[n1m2]=ll[n1m2]>>d;\
          }\
         if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
           for(y2=f1+1;y2<mapal->h;y2++)\
            {\
             lm=(unsigned char *)mapal->line[y2];\
             ln=(unsigned char *)mapals->line[y2];\
             if((lm[n1]<255 || lm[n1m1] || lm[n1m2]<255) &&\
                 lm[n1]==ln[n1] && lm[n1m1]==ln[n1m1] && lm[n1m2]==ln[n1m2])\
              {\
               ln[n1]=lm[n1]>>d;\
               ln[n1m1]=lm[n1m1]>>d;\
               ln[n1m2]=lm[n1m2]>>d;\
              }\
            }\
        }\
    }\
  }\
 else\
  {\
   if(pctj_sombreado_t)\
    {unsigned short color;\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned char *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
       {\
         if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2] &&\
           (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
          {\
           color=ll[n1]*pctj_sombreado_t;\
           lls[n1]=(unsigned char)(color>>8);\
           color=ll[n1m1]*pctj_sombreado_t;\
           lls[n1m1]=(unsigned char)(color>>8);\
           color=ll[n1m2]*pctj_sombreado_t;\
           lls[n1m2]=(unsigned char)(color>>8);\
          }\
        }\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     x1=x1*aumento1+fm_es_big_endian_y_32(prof1);\
     x2=x2*aumento1+fm_es_big_endian_y_32(prof1);\
     for(x1m1=x1+aumento1, x2m1=x2+aumento1; f1<ff; f1++, f2++, x1+=2*aumento1, x1m1+=2*aumento1, x2-=2*aumento1, x2m1-=2*aumento1)\
      {\
       ls =(unsigned char *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       if(ni<x1) {ni=x1;n2i=ni+(inf_mapa.despx-x)*aumento2+fm_es_big_endian_y_32(prof2);}\
       if(nf>x2+2*aumento1) nf=x2+2*aumento1;\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
         if(ls[n2]<255 || ls[n2+1] || ls[n2+2]<255)\
          {\
           if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
               ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2])\
            {\
             color=ll[n1]*pctj_sombreado_t;\
             lls[n1]=(unsigned char)(color>>8);\
             color=ll[n1m1]*pctj_sombreado_t;\
             lls[n1m1]=(unsigned char)(color>>8);\
             color=ll[n1m2]*pctj_sombreado_t;\
             lls[n1m2]=(unsigned char)(color>>8);\
            }\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned char *)mapal->line[y2];\
               ln=(unsigned char *)mapals->line[y2];\
               if((lm[n1]<255 || lm[n1m1] || lm[n1m2]<255) &&\
                   lm[n1]==ln[n1] && lm[n1m1]==ln[n1m1] && lm[n1m2]==ln[n1m2])\
                {\
                 color=lm[n1]*pctj_sombreado_t;\
                 ln[n1]=(unsigned char)(color>>8);\
                 color=lm[n1m1]*pctj_sombreado_t;\
                 ln[n1m1]=(unsigned char)(color>>8);\
                 color=lm[n1m2]*pctj_sombreado_t;\
                 ln[n1m2]=(unsigned char)(color>>8);\
                }\
              }\
          }\
      }\
    }\
   else\
    {\
     for(f1=fi, f2=fi+inf_mapa.despy-y; f1<medio; f1++, f2++)\
      {\
       ls =(unsigned char *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
         if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
             ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2] &&\
           (ls[n2]<255 || ls[n2+1] || ls[n2+2]<255))\
           lls[n1]=lls[n1m1]=lls[n1m2]=0;\
      }\
     x1=((mapal->w)>>1)-2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)+2*(f1-medio);\
     x2=((mapal->w)>>1)+2*anchuramayor+(objr?0:objl->anchurax-objl->anchuray)-2*(f1-medio)-2;\
     x1=x1*aumento1+fm_es_big_endian_y_32(prof1);\
     x2=x2*aumento1+fm_es_big_endian_y_32(prof1);\
     for(x1m1=x1+aumento1, x2m1=x2+aumento1; f1<ff; f1++, f2++, x1+=2*aumento1, x1m1+=2*aumento1, x2-=2*aumento1, x2m1-=2*aumento1)\
      {\
       ls =(unsigned char *)sombra->line[f2];\
       ll =(unsigned char *)mapal->line[f1];\
       lls=(unsigned char *)mapals->line[f1];\
       if(ni<x1) {ni=x1;n2i=ni+(inf_mapa.despx-x)*aumento2+fm_es_big_endian_y_32(prof2);}\
       if(nf>x2+2*aumento1) nf=x2+2*aumento1;\
       for(n1=ni, n1m1=ni+1, n1m2=ni+2, n2=n2i; n1<nf; n1+=aumento1, n1m1+=aumento1, n1m2+=aumento1, n2+=aumento2)\
         if(ls[n2]<255 || ls[n2+1] || ls[n2+2]<255)\
          {\
           if((ll[n1]<255 || ll[n1m1] || ll[n1m2]<255) &&\
               ll[n1]==lls[n1] && ll[n1m1]==lls[n1m1] && ll[n1m2]==lls[n1m2])\
             lls[n1]=lls[n1m1]=lls[n1m2]=0;\
           if(n1==x1 || n1==x1m1 || n1==x2 || n1==x2m1)\
             for(y2=f1+1;y2<mapal->h;y2++)\
              {\
               lm=(unsigned char *)mapal->line[y2];\
               ln=(unsigned char *)mapals->line[y2];\
               if((lm[n1]<255 || lm[n1m1] || lm[n1m2]<255) &&\
                   lm[n1]==ln[n1] && lm[n1m1]==ln[n1m1] && lm[n1m2]==ln[n1m2])\
                 ln[n1]=ln[n1m1]=ln[n1m2]=0;\
              }\
          }\
      }\
    }\
  }\
} 

//**********************************
// Oscurece un objeto con una sombra
//**********************************
void prv_sombrear_objeto(objeto_r *objr, objeto_l *objl, int x, int y, BITMAP *sombra, char transp)
{
 int prof1=bitmap_color_depth(objr?objr->mapabase:objl->mapabase);
 if(prof1>8 && transp<100)
  {
   struct infomapa inf_mapa=(objr?objr->inf_mapa:objl->inf_mapa);
   BITMAP *mapal=(objr?objr->mapabase:objl->mapabase);
   BITMAP *mapals=(objr?objr->inf_mapa.mapa:objl->mapasomb);
   int ctl=bitmap_mask_color(mapal);
   int cts=bitmap_mask_color(sombra);
   int fi=y-inf_mapa.despy;
   int ni=x-inf_mapa.despx;
   int ff=y+sombra->h-inf_mapa.despy;
   int nf=x+sombra->w-inf_mapa.despx;
   int anchuramayor=(objr?rejilla.anchura:(objl->anchurax>objl->anchuray?objl->anchurax:objl->anchuray));
   int medio=mapal->h-anchuramayor-(objr?objr->altura:objl->altura);
   int f1,f2,n1,n2,x1,x1m1,x2,x2m1,y2;
    
   if(fi<0) fi=0;
   if(ni<0) ni=0;
   if(ff>mapal->h) ff=mapal->h;
   if(nf>mapal->w) nf=mapal->w;
   if(ff<medio) medio=ff;
   if(ff>medio+anchuramayor) ff=medio+anchuramayor;

   if(fi<ff && ni<nf)
    {int prof2=bitmap_color_depth(sombra);
     int n2i=ni+inf_mapa.despx-x;
     char caso=(prof1<24?0:3)+(prof2==8?0:(prof2<24?1:2));
     
     if(!mapals)
      { 
       mapals=create_bitmap_ex(bitmap_color_depth(mapal), mapal->w, mapal->h);
       if(objr)
         inf_mapa.mapa=objr->inf_mapa.mapa=mapals;
       else
         objl->mapasomb=mapals;  
      } 

     if(objr)
      {
       if(objr->sombrear==1)
        {
         blit(mapal,mapals,0,0,0,0,mapal->w,mapal->h);
         objr->sombrear=2;
        } 
      }
     else   
      {
       if(objl->sombrear==1)
        {
         blit(mapal,mapals,0,0,0,0,mapal->w,mapal->h);
         objl->sombrear=2;
        } 
      }

     switch(caso)
      {
       case 0: fm_sombrear_o_NT_NT(char);  break;
       case 1: fm_sombrear_o_NT_NT(short); break;
       case 2: fm_sombrear_o_NT_T();       break;
       case 3: fm_sombrear_o_T_NT(char);   break;
       case 4: fm_sombrear_o_T_NT(short);  break; 
       case 5: fm_sombrear_o_T_T();        break;
      }
    }  
  }        
}

#undef fm_sombrear_o_NT_NT
#undef fm_sombrear_o_NT_T
#undef fm_sombrear_o_T_NT
#undef fm_sombrear_o_T_T
#undef fm_es_big_endian_y_32

//***********************
// Oscurece la habitación
//***********************
void prv_oscurecer_hab(BITMAP *mapa, int x0, int y0, int x1, int y1)
{
 int prof1=bitmap_color_depth(mapa);
 if(prof1>8 && x0<=x1 && y0<=y1)
  {
   int f,n;
   
   if(prof1<24)
    { // 15 o 16 bits
     unsigned long color;
     unsigned int mascara=(prof1==15?0x03e07c1f:0x07e0f81f);
     unsigned short *ll;
     if(dt_oscluz.pctj_oscuridad_32==2 || dt_oscluz.pctj_oscuridad_32==4 ||
        dt_oscluz.pctj_oscuridad_32==8 || dt_oscluz.pctj_oscuridad_32==16)
      {char d=4; // Sombreado potencia de dos.
       char pctj_oscuridad_32_t=dt_oscluz.pctj_oscuridad_32;
       while(pctj_oscuridad_32_t!=2)
        {
         pctj_oscuridad_32_t=pctj_oscuridad_32_t>>1;
         d--;
        }
       for(f=y0; f<=y1; f++)
        {
         ll =(unsigned short *)mapa->line[f];
         for(n=x0; n<=x1; n++)
          {
           color=ll[n];
           color=(color|(color<<16)) & mascara;
           color=(color>>d) & mascara;
           color=color|(color>>16);
           ll[n]=(unsigned short)color;
          }
        }
      }
     else
      { // Sombreado normal
       for(f=y0; f<=y1; f++)
        {
         ll =(unsigned short *)mapa->line[f];
         for(n=x0; n<=x1; n++)
          {
           color=ll[n];
           color=(color|(color<<16)) & mascara;
           color=((color*dt_oscluz.pctj_oscuridad_32)>>5) & mascara;
           color=color|(color>>16);
           ll[n]=(unsigned short)color;
          }
        }
      }  
    }    
   else
    {//24 o 32 bits
     unsigned char *ll;
     char aumento;
     if(prof1==24)
      {
       aumento=3;
       x0*=3;
       x1*=3;
      }
     else
      {
       aumento=4;
       x0*=4;
       x1*=4;
#ifdef ALLEGRO_BIG_ENDIAN
       x0++;
       x1++;
#endif
      }
     if(dt_oscluz.pctj_oscuridad==2  || dt_oscluz.pctj_oscuridad==4  ||
        dt_oscluz.pctj_oscuridad==8  || dt_oscluz.pctj_oscuridad==16 || 
        dt_oscluz.pctj_oscuridad==32 || dt_oscluz.pctj_oscuridad==64 ||
        dt_oscluz.pctj_oscuridad==128)
      {// Sombreado potencia de 2
       char d=7;
       short pctj_oscuridad_t=dt_oscluz.pctj_oscuridad;
       while(pctj_oscuridad_t!=2)
        {
         pctj_oscuridad_t=pctj_oscuridad_t>>1;
         d--;
        }
       for(f=y0; f<=y1; f++)
        {
         ll = mapa->line[f];
         for(n=x0;n<=x1;n+=aumento)
          {
           ll[n]=ll[n]>>d;
           ll[n+1]=ll[n+1]>>d;
           ll[n+2]=ll[n+2]>>d;
          }
        }
      }
     else
      {// Sombreado normal
       unsigned short color;
       for(f=y0; f<=y1; f++)
        {
         ll = mapa->line[f];
         for(n=x0;n<=x1;n+=aumento)
          {
           color=ll[n]*dt_oscluz.pctj_oscuridad;
           ll[n]=(unsigned char)(color>>8);
           color=ll[n+1]*dt_oscluz.pctj_oscuridad;
           ll[n+1]=(unsigned char)(color>>8);
           color=ll[n+2]*dt_oscluz.pctj_oscuridad;
           ll[n+2]=(unsigned char)(color>>8);
          }
        }
      }
    }
  }
}

//***************************************
// Apila un objeto en la pila de bloqueos
//***************************************
char prv_apilar_objeto_blq(ism_id id)
{
 elem_blq *blq=NULL;

 blq=(elem_blq *)malloc(sizeof(elem_blq));
 if(!blq) return 0;
 blq->id=id;
 blq->sig=pila_blq;
 primer_blq=pila_blq=blq;
 return 1;
}

//********************************
// Inicializa la lista de bloqueos
//********************************
void prv_vaciar_pila_blq(void)
{
 while(pila_blq=primer_blq)
  {
   primer_blq=primer_blq->sig;
   free(pila_blq);
  }
}

//*********************************************************
// Cambia el porcentaje de transparencia de un objeto libre
//*********************************************************
int prv_cambiar_transparencia_objeto(ism_id id, int valor, unsigned char modo)
{
 objeto_l *objl=NULL;
 char transp, enmascarar;
 
 // Se localiza el objeto
 if(ult_objl) if(ult_objl->id==id)
   objl=ult_objl;
 if(!objl)
  {
   objl=lista_objl;
   while(objl)
    {
     if(objl->id==id) break;
     objl=objl->sig;
    }
   if(!objl) fm_error(10);
   ult_objl=objl;
  }

 transp=(char)(valor+objl->transp*modo);
 if(transp<0 || transp>100) fm_error(19);
 if(objl->transp==transp) return 0;
 if(objl->mapabase && (transp==0 || objl->transp==0))
   enmascarar=1;
 else
   enmascarar=0;  
 fm_incorporar_cuadricula(objl->mapabase,objl->inf_mapa.despx+x0ant,objl->inf_mapa.despy+y0ant);

 if(objl->transp<100) prv_quitar_transparencia(objl->transp);
 if(transp<100) prv_introducir_transparencia(transp);
 if(objl->mapabase)
 if(objl->transp==0 || transp==0) // Enmascaramos los objetos libres que queden detrás.
  {objeto_l *objm=lista_objl;
   while(objm)
    {
     if((objm->id!=objl->id) && objm->mapabase)
      {
       if((objm->inf_mapa.despx < objl->inf_mapa.despx+objl->mapabase->w) &&
          (objm->inf_mapa.despx+objm->mapabase->w > objl->inf_mapa.despx) &&
          (objm->inf_mapa.despy < objl->inf_mapa.despy+objl->mapabase->h) &&
          (objm->inf_mapa.despy+objm->mapabase->h > objl->inf_mapa.despy))
         objm->enmascarar=1;
      }   
     objm=objm->sig;
    }
  }
 objl->transp=transp;
 
 // Si es necesario, se sombrean las losetas y objetos
 if(pctj_sombreado<256 && objl->sombra)
  {
   int xini=objl->x/rejilla.anchura;                      // 
   int xfin=(objl->x+objl->anchurax-1)/rejilla.anchura+1; // Rango de columnas
   int yini=(objl->y-objl->anchuray+1)/rejilla.anchura;   // intersectadas por el objeto
   int yfin=objl->y/rejilla.anchura+1;                    //
   objeto_l *objm=lista_objl;
   int f,n;

   for(f=xini;f<xfin;f++)
     for(n=yini;n<yfin;n++)
      {objeto_r *objr=suelo[rejilla.x*n+f].objr;
       while(objr)
        {
         if(objl->z>objr->z)
          {
           objr->sombrear=1;
           if(!(objl->mapabase))
             fm_incorporar_cuadricula(objr->mapabase,objr->inf_mapa.despx+x0ant,objr->inf_mapa.despy+y0ant);
           objr=objr->sig;
          }
         else
           objr=NULL; 
        }
       suelo[rejilla.x*n+f].sombrear=1;
      } 
     suelo[rejilla.x*rejilla.y].sombrear=1;
     fm_incorporar_cuadricula(objl->sombra,2*(objl->x-objl->y)+x0ant+(objl->anchurax+objl->anchuray)-((objl->sombra->w)>>1)-1,
                                     objl->x+objl->y+y0ant+((objl->anchurax-objl->anchuray+1)>>1)-((objl->sombra->h)>>1));

   while(objm)
    {
     if(objm->id!=objl->id)
      {
       if(objm->z<=objl->z &&
          objm->x<=objl->x+objl->anchurax && objl->x<=objm->x+objm->anchurax &&
          objm->y>=objl->y-objl->anchuray && objl->y>=objm->y-objm->anchuray)
        {
         objm->sombrear=1;
         if(!(objl->mapabase))
           fm_incorporar_cuadricula(objm->mapabase,objm->inf_mapa.despx+x0ant,objm->inf_mapa.despy+y0ant);
        }
       if(enmascarar)
       if(objm->mapabase)
       if((objm->inf_mapa.despx < objl->inf_mapa.despx+objl->mapabase->w) &&
          (objm->inf_mapa.despx+objm->mapabase->w > objl->inf_mapa.despx) &&
          (objm->inf_mapa.despy < objl->inf_mapa.despy+objl->mapabase->h) &&
          (objm->inf_mapa.despy+objm->mapabase->h > objl->inf_mapa.despy))
         objm->enmascarar=1;
      }   
     objm=objm->sig;  
    }
  }

 return 0;
}

//*********************************************
// Añade un objeto a la tabla de transparencias
//*********************************************
void prv_introducir_transparencia(char transp)
{
 char f=0;

 while(tb_transp[f].tptransp<transp) f++;
 if(tb_transp[f].tptransp>transp)
  {char n=f+1;
   while(tb_transp[n].tptransp<100) n++;
   do
    {
     tb_transp[n].tptransp=tb_transp[n-1].tptransp;
     tb_transp[n].ntransp=tb_transp[n-1].ntransp;
    }while(f!=(--n));
   tb_transp[f].tptransp=transp;
   tb_transp[f].ntransp=1;
  }
 else
   tb_transp[f].ntransp++;
}

//**********************************************
// Quita un objeto de la tabla de transparencias
//**********************************************
void prv_quitar_transparencia(char transp)
{
 char f=0;

 while(tb_transp[f].tptransp<transp) f++;
 if((tb_transp[f].ntransp--)==1)
   do
    {
     tb_transp[f].tptransp=tb_transp[f+1].tptransp;
     tb_transp[f].ntransp=tb_transp[f+1].ntransp;
    }while(tb_transp[++f].tptransp<100);
}

//***************************************************
// Ordena el objeto libre moviéndolo hacia la derecha
//***************************************************
void prv_ordenar_objeto(objeto_l *objl)
{
 objeto_l *objm=objl->sig;

 while(objl && objm)
   if((objl->z >= (objm->z+objm->altura)) ||
      (objl->x >= (objm->x+objm->anchurax)) ||
      ((objl->y-objl->anchuray) >= objm->y))
    {
     if(objl->sig=objm->sig) objl->sig->ant=objl;
     objm->sig=objl;
     if(objm->ant=objl->ant) objm->ant->sig=objm;
     objl->ant=objm;
     if(lista_objl==objl) lista_objl=objm;
     objm=objl->sig;
    }
   else
     objm=NULL;
}

//*********************************************
// Incorpora una cuadrícula al rectángulo sucio
//*********************************************
void prv_incorporar_cuadricula(int x0, int y0, int x1, int y1)
{
 if(dt_rect.modo==RECT_NO_DIB)
  {
   dt_rect.x0=x0;
   dt_rect.y0=y0;
   dt_rect.x1=x1;
   dt_rect.y1=y1;
   dt_rect.modo=RECT_SI_DIB;
  }
 else
  {
   if(dt_rect.x0>x0) dt_rect.x0=x0;
   if(dt_rect.y0>y0) dt_rect.y0=y0;
   if(dt_rect.x1<x1) dt_rect.x1=x1;
   if(dt_rect.y1<y1) dt_rect.y1=y1;
  }
}

#undef PRIMER_ID_L
#undef PRIMER_ID_R
#undef ULT_ID_L
#undef ULT_ID_R
#undef fm_error
#undef fm_compr_ini
#undef fm_id_error
#undef fm_id_compr_ini
#undef fm_funcion
#ifndef ISM_ESP
#undef SUP_X
#undef INF_X
#undef SUP_Y
#undef INF_Y
#undef TODAS
#undef ENCIMA
#undef ID_PARED_SUP_X
#undef ID_PARED_SUP_Y
#undef ID_PARED_INF_X
#undef ID_PARED_INF_Y
#undef ID_SUELO
#undef D_ALTURA
#undef D_ANCHURA_X
#undef D_ANCHURA_Y
#undef SUMAR
#undef ism_vaciar_mundo_isom
#undef ism_obtener_coords_celda
#undef ism_obtener_coords_libres
#undef RECT_DESC
#undef RECT_NO_DIB
#undef RECT_SI_DIB
#undef RECT_DIB_TODO
#undef fm_incorporar_cuadricula
#endif

