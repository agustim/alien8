// Isomot v 1.0
// Motor isométrico para Allegro
// por Ignacio Pérez Gil (15/12/2001 - 17/05/2008)

// isomot.h
// Fichero de cabecera de Isomot

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

#ifndef _ISOMOT_
#define _ISOMOT_

#include "ism_lng.h"
#ifndef ISM_ESP
#ifndef ISM_ENG
#define ISM_ESP
#define ISM_ENG
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Definición del tipo de datos para identificadores de objetos
typedef unsigned int ism_id;

#ifdef ISM_ESP
// Funciones y definiciones de la interface en español
int ism_dibujar_mundo_isom(BITMAP *mapa_destino, int x0, int y0);
int ism_establecer_rejilla(int x, int y, int anchura);
int ism_vaciar_mundo_isom(void);
int ism_destruir_todo(void);
#define SUP_X  0
#define INF_X  1
#define SUP_Y  2
#define INF_Y  3
#define TODAS -1
int ism_colocar_pared(BITMAP *mapa, unsigned char eje, int pared);
int ism_colocar_loseta(BITMAP *mapa, int x, int y);
int ism_colocar_suelo(BITMAP *mapa);
int ism_mover_pared(unsigned char eje, int pared, int despeje, int despaltura);
int ism_mover_loseta(int x, int y, int despx, int despy);
int ism_mover_suelo(int despx, int despy);
int ism_obtener_coords_libres(int x, int y, int *xl, int *yl);
int ism_obtener_coords_celda(int x, int y, int *xc, int *yc);
int ism_coords_iso_a_2d(int xiso, int yiso, int ziso, int *x2d, int *y2d);
int ism_coords_2d_a_iso(int *xiso, int *yiso, int *ziso, int x2d, int y2d, int vfija, unsigned char cfija);
#define ENCIMA         -1
#define NO_ID           0
#define ID_PARED_SUP_X  1
#define ID_PARED_SUP_Y  2
#define ID_PARED_INF_X  3
#define ID_PARED_INF_Y  4
#define ID_SUELO        5
#define ID_ERROR        6
ism_id ism_colocar_objeto_rejilla(int x, int y, int z, int altura, BITMAP *mapa, BITMAP *sombra);
ism_id ism_colocar_objeto_libre(int x, int y, int z, int anchurax, int anchuray, int altura, BITMAP *mapa, BITMAP *sombra);
int ism_quitar_objeto(ism_id id);
#define D_X         0
#define D_Y         1
#define D_Z         2
#define D_ALTURA    3
#define D_ANCHURA_X 4
#define D_ANCHURA_Y 5
#define D_TRANSP    6
int ism_obtener_dato_objeto(ism_id id, unsigned char dato);
int ism_obtener_coords_objeto(ism_id id, int *dstx, int *dsty, int *dstz);
#define CAMBIAR 0
#define SUMAR   1
int ism_cambiar_dato_objeto(ism_id id, unsigned char dato, int valor, unsigned char modo);
int ism_cambiar_mapa_objeto(ism_id id, BITMAP *mapa);
int ism_cambiar_sombra_objeto(ism_id id, BITMAP *sombra);
int ism_establecer_sombras(char sombras);
int ism_establecer_oscuridad(char oscuridad);
void ism_habilitar_rect_sucio(char habilitar);
int ism_mover_objeto(ism_id id, int x, int y, int z, unsigned char modo);
int ism_existe_objeto(ism_id id);
ism_id ism_extraer_id_blq(void);
int ism_colisionado_con(ism_id id);
int ism_num_colisiones(void);
int ism_func_transp(void(*f_transp)(BITMAP *, BITMAP *, int, int, int));
char *ism_desc_error(void);
#endif

#ifdef ISM_ENG
// English interface functions and definitions
int ism_draw_isom_world(BITMAP *map_dest, int x0, int y0);
int ism_set_grid(int x, int y, int width);
int ism_empty_isom_world(void);
int ism_destroy_all(void);
#define HIGH_X 0
#define LOW_X  1
#define HIGH_Y 2
#define LOW_Y  3
#define ALL   -1
int ism_set_wall(BITMAP *map, unsigned char axis, int wall);
int ism_set_floor_tile(BITMAP *map, int x, int y);
int ism_set_floor(BITMAP *map);
int ism_move_wall(unsigned char axis, int wall, int offsetaxis, int offsetheight);
int ism_move_floor_tile(int x, int y, int offsetx, int offsety);
int ism_move_floor(int offsetx, int offsety);
int ism_get_free_coords(int x, int y, int *fx, int *fy);
int ism_get_cell_coords(int x, int y, int *cx, int *cy);
int ism_coords_iso_to_2d(int xiso, int yiso, int ziso, int *x2d, int *y2d);
int ism_coords_2d_to_iso(int *xiso, int *yiso, int *ziso, int x2d, int y2d, int fixedv, unsigned char fixedc);
#define TOP            -1
#define NO_ID           0
#define ID_HIGH_X_WALL  1
#define ID_HIGH_Y_WALL  2
#define ID_LOW_X_WALL   3
#define ID_LOW_Y_WALL   4
#define ID_FLOOR        5
#define ID_ERROR        6
ism_id ism_put_grid_object(int x, int y, int z, int height, BITMAP *map, BITMAP *shadow);
ism_id ism_put_free_object(int x, int y, int z, int widthx, int widthy, int height, BITMAP *map, BITMAP *shadow);
int ism_remove_object(ism_id id);
#define D_X         0
#define D_Y         1
#define D_Z         2
#define D_HEIGHT    3
#define D_WIDTH_X   4
#define D_WIDTH_Y   5
#define D_TRANSP    6
int ism_get_object_data(ism_id id, unsigned char data);
int ism_get_object_coords(ism_id id, int *dstx, int *dsty, int *dstz);
#define CHANGE  0
#define ADD     1
int ism_change_object_data(ism_id id, unsigned char data, int value, unsigned char mode);
int ism_change_object_map(ism_id id, BITMAP *map);
int ism_change_object_shadow(ism_id id, BITMAP *shadow);
int ism_set_shadowing_percentage(char shadowper); 
int ism_set_gloom_percentage(char gloomper);
void ism_enable_dirty_rect(char enable);
int ism_move_object(ism_id id, int x, int y, int z, unsigned char mode);
int ism_object_exists(ism_id id);
ism_id ism_get_coll_id(void);
int ism_collided_against(ism_id id);
int ism_collisions_number(void);
int ism_set_transp_func(void(*transp_f)(BITMAP *, BITMAP *, int, int, int));
char *ism_error_desc(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

