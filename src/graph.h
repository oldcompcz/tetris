/*
*	graph.h
*	Jaroslav Beran - jaroslav.beran@gmail.com
*	28.6.2014
*	redistributable under the terms of the GNU/GPLv3
*/


#ifndef _GRAPH_
#define _GRAPH_

typedef struct {
        HPEN hPen;
        HBRUSH hBrush;
} COLOR_T;

#define G_COLORS_SIZE 8

extern void g_draw_mesh(HDC hdc, TETRIS_T *tetris, RECT client);
extern void g_put_mesh_pixel(HDC hdc, TETRIS_T *tetris, int x, int y, int color);
extern void g_empty_mesh_pixel(HDC hdc, TETRIS_T *tetris, int x, int y);
extern void g_set_colors(COLOR_T *colors);


#endif

