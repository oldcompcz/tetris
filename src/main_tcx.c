/*
*       main_tcx.c
*
*       This file is part of Tetris game.
*       https://bitbucket.org/berk76/tetris
*
*       Tetris is free software; you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation; either version 3 of the License, or
*       (at your option) any later version. <http://www.gnu.org/licenses/>
*
*       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 12.6.2017
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "sound_tc.h"
#include "tetris.h"
#include "tui_tc.h"
#include "wait_tc.h"
#include "res_tc.h"
#include "main.h"


#define TUI_COL LIGHTGRAY
#define TUI_BKCOL BLACK

static int MESH_BK_COLOR = 7;
static TETRIS_T tetris;
static int color_vec[] = {LIGHTBLUE,
                          LIGHTGREEN,
                          LIGHTCYAN,
                          LIGHTRED,
                          LIGHTMAGENTA,
                          YELLOW,
                          WHITE,
                          BLACK,
                          BLUE};
static WINDOW_T *mainw = NULL;        
static JOB_T *j2, *j3, *j4;
static SND_SONG song;
static int play_sound;
static int _delay;
static int score_x;
static int score_y;


static int draw_mainscreen(TETRIS_T *t);
static void draw_addtris();
static void draw_tetris();
static void draw_xtris();
static int update_score(int reset);
static int process_user_input();
/* Jobs */
static long draw_floating_text(enum W_ACTION a);
static long animate_scr_main(enum W_ACTION a);
static long animate_scr_add(enum W_ACTION a);
static long animate_scr_tet(enum W_ACTION a);
static long animate_scr_xte(enum W_ACTION a);


int main() {
        int c, ret;
        int d;
        

        mainw = tui_create_win(1, 1, TUI_SCR_X_SIZE, TUI_SCR_Y_SIZE, TUI_COL, TUI_BKCOL, ' ');
        srand(time(NULL) % 37);
        j2 = w_register_job(4, &draw_floating_text);
        j3 = NULL;
        j4 = NULL;
        play_sound = 1;

        do {
                ret = draw_mainscreen(&tetris);
                
                /* Quit game */
                if (ret == 0) {
                        w_unregister_job(j2);
                        if (j3 != NULL)
                                w_unregister_job(j3);
                        if (j4 != NULL)
                                w_unregister_job(j4);
                        snd_speaker(0);
                        tui_delete_win(mainw);
                        return 0;
                }
                         
                switch (tetris.game) {
                        case TETRIS:
                                draw_tetris();
                                break;
                        case XTRIS:
                                draw_xtris();
                                break;
                        case ADDTRIS:
                                draw_addtris();
                                break; 
                }
                        
                tui_message("\n\x01\x0fPress any key to start ...\x01\x0b\n", LIGHTCYAN, TUI_BKCOL);
                d = _delay;
                if (play_sound == 1)
                        j3 = w_register_job(6, &snd_play_sound);
        
                do {
                        int i;                             

                        for (i = 0; i < d; i++) {
                                c = process_user_input();
                                if (c == -1)
                                        break;
                                w_wait(1);
                        }
                        
                        ret = t_go(&tetris);
                        
                        if (update_score(0) == 1) {
                                d = _delay - (tetris.score / 20);
                                if (d < 3)
                                        d = 3;
                        }  
                } while ((ret != -1) && (c != -1));

                t_delete_game(&tetris);
                if (j3 != NULL) {
                        w_unregister_job(j3);
                        j3 = NULL;
                        snd_speaker(0);
                }
                
                if (j4 != NULL) {
                        w_unregister_job(j4);
                        j4 = NULL;
                }
                
                if (c != -1)
                        tui_message("\n\x01\x0fGAME OVER\x01\x0b\n", LIGHTCYAN, TUI_BKCOL);
                
        } while (1);
}


int draw_mainscreen(TETRIS_T *t) {
        int i, c, seg, wide, ret;
        char buff[512];
        GAME_T game;
        int menu_only;
        
        ret = -1;
        menu_only = 0;
        do {
                if (menu_only == 0) {
                        tui_cls_win(mainw, FALSE);
                        tui_draw_box(15, 1, TUI_COL, TUI_BKCOL, gfx_ptakovina, FALSE);
                        
                        gotoxy(23,14);
                        tui_set_attr(0, BROWN, TUI_BKCOL);
                        for (i = 0; i < 34; i++)
                                putch('=');
                        tui_draw_box(5, 12, TUI_COL, TUI_BKCOL, gfx_bird_08, FALSE);
                        tui_draw_box(13, 9, TUI_COL, TUI_BKCOL, gfx_bird_10, FALSE);
                        tui_draw_box(57, 9, TUI_COL, TUI_BKCOL, gfx_bird_11, FALSE);
                        tui_draw_box(67, 12, TUI_COL, TUI_BKCOL, gfx_bird_09, FALSE);
                        
                        tui_draw_box(17, 22, LIGHTGREEN, TUI_BKCOL, "- - - = = = (c) 2017 Jaroslav Beran = = = - - -", FALSE);
                        menu_only = 1;
                }
                
                if ((play_sound == 1) && (j3 == NULL)) {
                        song.duration = D2;
                        song.rest = R2;
                        song.song = s2;
                        snd_setsong(&song);
                        j3 = w_register_job(6, &snd_play_sound);
                }
                
                if ((play_sound == 0) && (j3 != NULL)) {
                        w_unregister_job(j3);
                        j3 = NULL;
                        snd_speaker(0);
                }
                
                sprintf(buff, "\n\x01\x0f 1) Addtris\x01\x0b \n\n" \
                               "\x01\x0f 2) Tetris\x01\x0b \n\n" \
                               "\x01\x0f 3) X-Tris\x01\x0b \n\n" \
                               "\x01\x0f S) Sound: %s\x01\x0b \n\n" \
                               "\x01\x0f Q) Quit\x01\x0b \n", \
                               (play_sound == 0) ? "off" : "on");
                               
                j4 = w_register_job(18, &animate_scr_main);
                
                c = tui_option(buff, "123SsQq", LIGHTCYAN, TUI_BKCOL);
                
                w_unregister_job(j4);
                j4 = NULL;
                        
                switch (c) {
                        case '1':
                                game = ADDTRIS;
                                seg = 1;
                                wide = 10;
                                _delay = 15;
                                ret = 1;
                                break;
                        case '2':
                                game = TETRIS;
                                seg = 4;
                                wide = 10;
                                _delay = 10;
                                ret = 1;
                                break;
                        case '3':
                                game = XTRIS;
                                c = tui_option("\n\x01\x0f Brick size? (1..9)\x01\x0b \n", "123456789", LIGHTCYAN, TUI_BKCOL);
                                seg = c - '0';
                                c = tui_option("\n\x01\x0f (S)tandard grid or (D)ouble wide?\x01\x0b \n", "sSdD", LIGHTCYAN, TUI_BKCOL);
                                if (c == 's' || c == 'S') {
                                        wide = 10;
                                } else {
                                        wide = 20;
                                }
                                if (seg > 4) {
                                        _delay = 15;
                                } else {
                                        _delay = 10;
                                }
                                ret = 1;
                                break;
                        case 'S':
                        case 's':
                                if (play_sound == 0) {
                                        play_sound = 1;
                                } else {
                                        play_sound = 0;
                                }
                                break;
                        case 'Q':
                        case 'q':
                                ret = 0;
                                break;
                }
        } while (ret == -1);
        
        if (j3 != NULL) {
                w_unregister_job(j3);
                j3 = NULL;
                snd_speaker(0);
        }
        
        t_create_game(t, game, wide, 20, seg);
        
        /* 0=quit game; 1=game created */
        return ret;
}


void draw_addtris() {
        int x, y;

        tui_cls_win(mainw, FALSE);
	tetris.element_size = 1;
        tetris.origin_y = 3;

        tetris.origin_x = 47;
        tui_draw_box(1, 1, TUI_COL, TUI_BKCOL, gfx_addtris, FALSE);
        
        tui_set_attr(0, LIGHTMAGENTA, TUI_BKCOL);

        for (y = 0; y < tetris.element_size * tetris.grid_size_y; y++) {
                gotoxy(tetris.origin_x - 1, tetris.origin_y + y);
                putch('|');
                for (x = 0; x < tetris.element_size * 2 * tetris.grid_size_x; x++) {
                        putch(' ');
                }
                putch('|');
        }

        gotoxy(tetris.origin_x - 1, tetris.origin_y + tetris.element_size * tetris.grid_size_y);
        putch('+');
        for (x = 0; x < tetris.element_size * 2 * tetris.grid_size_x; x++) {
                putch('-');
        }
        putch('+');
        
        tui_set_attr(0, TUI_COL, TUI_BKCOL);
        tui_draw_box(69, 19, TUI_COL, TUI_BKCOL, gfx_mush_01, FALSE);
        tui_draw_box(6, 8, TUI_COL, TUI_BKCOL, gfx_witch_01, FALSE);
        
        #define ADD_OWL_X 34
        #define ADD_OWL_Y 6
        tui_draw_box(ADD_OWL_X, ADD_OWL_Y, TUI_COL, TUI_BKCOL, gfx_owl_03, FALSE);
        tui_draw_box(70, 3, TUI_COL, TUI_BKCOL, gfx_owl_02, FALSE);

        score_x = 69;
        score_y = 11;        
        update_score(1);

        tui_set_attr(0, WHITE, TUI_BKCOL);        
        gotoxy(69, 13);
        cprintf("Pause .. P");
        gotoxy(69, 14);
        cprintf("Quit  .. Q");
        tui_set_attr(0, TUI_COL, TUI_BKCOL);
        
        if ((play_sound == 1) && (j3 == NULL)) {
                song.duration = D4;
                song.rest = R4;
                song.song = s4;
                snd_setsong(&song);
        }
        
        j4 = w_register_job(18, &animate_scr_add);
}


void draw_tetris() {
        int i, x, y;

        tui_cls_win(mainw, FALSE);
	tetris.element_size = 1;
        tetris.origin_y = 3;

        tetris.origin_x = 39;
        tui_draw_box(1, 1, TUI_COL, TUI_BKCOL, gfx_tetris, FALSE);
        
        tui_set_attr(0, LIGHTMAGENTA, TUI_BKCOL);

        for (y = 0; y < tetris.element_size * tetris.grid_size_y; y++) {
                gotoxy(tetris.origin_x - 1, tetris.origin_y + y);
                putch('|');
                for (x = 0; x < tetris.element_size * 2 * tetris.grid_size_x; x++) {
                        putch(' ');
                }
                putch('|');
        }

        gotoxy(tetris.origin_x - 1, tetris.origin_y + tetris.element_size * tetris.grid_size_y);
        putch('+');
        for (x = 0; x < tetris.element_size * 2 * tetris.grid_size_x; x++) {
                putch('-');
        }
        putch('+');
        
        gotoxy(5,9);
        tui_set_attr(0, BROWN, TUI_BKCOL);
        for (i = 0; i < 15; i++)
                putch('=');
        tui_draw_box(20, 7, TUI_COL, TUI_BKCOL, gfx_bird_09, FALSE);
        tui_draw_box(10, 6, TUI_COL, TUI_BKCOL, gfx_bird_07, FALSE);
                
        tui_draw_box(2, 12, TUI_COL, TUI_BKCOL, \
                "For controls use arrows\n\n" \
                "Pause  ---------- P\n" \
                "Quit   ---------- Q\n\n", \
                FALSE);
        
        gotoxy(2, 23);
        tui_set_attr(0, LIGHTGREEN, TUI_BKCOL);
        for (x = 2; x < 24; x++)
                putch(',');
        tui_draw_box(2, 18, TUI_COL, TUI_BKCOL, gfx_rabbit_01, FALSE);
        tui_draw_box(26, 18, TUI_COL, TUI_BKCOL, gfx_rabbit_02, FALSE);        
        tui_draw_box(12, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_03, FALSE);
        tui_draw_box(22, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_04, FALSE);
        
        tui_draw_box(tetris.origin_x + 23, 1, TUI_COL, TUI_BKCOL, gfx_bird_03, FALSE);
        tui_draw_box(71, 8, TUI_COL, TUI_BKCOL, gfx_bird_06, FALSE);
        tui_draw_box(tetris.origin_x + 23, 18, TUI_COL, TUI_BKCOL, gfx_rabbit_05, FALSE);


        score_x = 65;
        score_y = 12;
        update_score(1);
        
        if ((play_sound == 1) && (j3 == NULL)) {
                song.duration = D6;
                song.rest = R6;
                song.song = s6;
                snd_setsong(&song);
        }
        
        j4 = w_register_job(18, &animate_scr_tet);
}


void draw_xtris() {
        int x, y;

        tui_cls_win(mainw, FALSE);
	tetris.element_size = 1;
        tetris.origin_y = 3;

        tetris.origin_x = 40;
        tui_draw_box(1, 1, TUI_COL, TUI_BKCOL, gfx_xtris, FALSE);
        
        tui_set_attr(0, LIGHTMAGENTA, TUI_BKCOL);

        for (y = 0; y < tetris.element_size * tetris.grid_size_y; y++) {
                gotoxy(tetris.origin_x - 1, tetris.origin_y + y);
                putch('|');
                for (x = 0; x < tetris.element_size * 2 * tetris.grid_size_x; x++) {
                        putch(' ');
                }
                putch('|');
        }

        gotoxy(tetris.origin_x - 1, tetris.origin_y + tetris.element_size * tetris.grid_size_y);
        putch('+');
        for (x = 0; x < tetris.element_size * 2 * tetris.grid_size_x; x++) {
                putch('-');
        }
        putch('+');
        
        tui_set_attr(0, TUI_COL, TUI_BKCOL);
        
        tui_draw_box(14, 16, TUI_COL, TUI_BKCOL, gfx_plant_01, FALSE);
        tui_draw_box(2, 10, TUI_COL, TUI_BKCOL, gfx_butter_01, FALSE);

        score_x = 23;
        score_y = 9;
        update_score(1);
        
        if ((play_sound == 1) && (j3 == NULL)) {
                song.duration = D5;
                song.rest = R5;
                song.song = s5;
                snd_setsong(&song);
        }
        
        j4 = w_register_job(18, &animate_scr_xte);
}


int update_score(int reset) {
        static int last_score;
        char s[20];

        if (reset) {
                last_score = 0;
        }
        
        if ((last_score == tetris.score) && !reset)
                return 0;

        last_score = tetris.score;
        
        sprintf(s, "Score: %3d", tetris.score);
        tui_draw_box(score_x, score_y, WHITE, TUI_BKCOL, s, FALSE);
        return 1;
}


int process_user_input() {
        int c, result;

        result = 0;
        while (kbhit()) {

                int c;
                c = getch();

                switch (c) {
                        case 0 :
                                c = getch();
                                switch (c) {
                                        case 75:
                                                t_move_left(&tetris);
                                                break;
                                        case 72:
                                                t_rotate(&tetris, 1);
                                                break;
                                        case 77:
                                                t_move_right(&tetris);
                                                break;
                                        case 80:
                                                while(t_move_down(&tetris) != -1)
                                                        w_wait(0);
                                                break;
                                }
                                break;
                        case 'p':
                                tui_message("\n\x01\x0fPaused\x01\x0b\n", LIGHTCYAN, TUI_BKCOL);
                                break;
                        case 'q':
                                if (tui_confirm("\n\x01\x0f Do you want to quit game? (Y/N) \x01\x0b\n", LIGHTCYAN, TUI_BKCOL) == TRUE) {
                                        result = -1;
                                }
                }
        }
        
        return result;
}


void m_put_mesh_pixel(TETRIS_T *tetris, int x, int y, int color) {
        x = tetris->origin_x + x * tetris->element_size * 2;
        y = tetris->origin_y + y * tetris->element_size;
        gotoxy(x, y);
        if ((tetris->game == TETRIS) || (tetris->game == XTRIS)) {
                textcolor(color_vec[color]);
                cprintf("%c%c", 0xdb, 0xdb);
                textcolor(color_vec[MESH_BK_COLOR]);
        } else {
                textcolor(WHITE);
                cprintf("%d ", tetris->brick.value);
                textcolor(color_vec[MESH_BK_COLOR]);
        }
        tui_flush();
}


void m_empty_mesh_pixel(TETRIS_T *tetris, int x, int y) {
        x = tetris->origin_x + x * tetris->element_size * 2;
        y = tetris->origin_y + y * tetris->element_size;
        gotoxy(x,y);
        printf("  ");
        tui_flush();
}


long draw_floating_text(enum W_ACTION a) {
        #define FT_X 5
        #define FT_Y 25
        #define FT_LEN 70
        
        static char b[(FT_LEN - 1) * 2];
        static char *p = NULL;
        static i = 0;
        
        if (a == RUN) {
                gotoxy(FT_X,FT_Y);
                tui_set_attr(0, DARKGRAY, TUI_BKCOL);
                
                if (p == NULL) {
                        p = floating_text;
                }
                
                gettext(FT_X + 1 , FT_Y, FT_X + 1 + (FT_LEN - 2), FT_Y, b);
                puttext(FT_X, FT_Y, FT_X + (FT_LEN - 2), FT_Y, b);
                
                gotoxy(FT_X + FT_LEN - 1, FT_Y);
                
                if (*p == '\0') {
                        putch(' ');
                        i++;
                        if (i == FT_LEN) {
                                p = NULL;
                                i = 0;
                        } 
                } else {
                        putch(*p);
                        p++;
                }
                
                tui_flush();
        }
        return 0;
}


long animate_scr_main(enum W_ACTION a) {
        static int step = 0;

        if (a == RESET) {
                return 0;        
        }            
        
        if (a == RUN) {
                switch (step) {
                        case 0:
                                switch (rand() % 10) {
                                        case 1:
                                                step = 1;
                                                break;
                                        case 2:
                                                step = 3;
                                                break;
                                }
                                break;
                        case 1:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(14, 10);
                                putch('-');
                                gotoxy(16, 10);
                                putch('-');
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(15, 11);
                                putch('v');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 16;
                        case 2:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(14, 10);
                                putch('.');
                                gotoxy(16, 10);
                                putch('.');
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(15, 11);
                                putch('V');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step = 0;
                                return 18;
                        case 3:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(63, 10);
                                putch('-');
                                gotoxy(65, 10);
                                putch('-');
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(64, 11);
                                putch('v');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 16;
                        case 4:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(63, 10);
                                putch('.');
                                gotoxy(65, 10);
                                putch('.');
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(64, 11);
                                putch('V');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step = 0;
                                return 18;
                }
        }
        
        return 18;
}


long animate_scr_add(enum W_ACTION a) {
        static int step = 0;

        if (a == RESET) {
                return 0;        
        }            
        
        if (a == RUN) {
                switch (step) {
                        case 0:
                                switch (rand() % 20) {
                                        case 1:
                                                step = 1;
                                                break;
                                        case 2:
                                                step = 3;
                                                break;
                                        case 3:
                                                step = 5;
                                                break;
                                        default:
                                                step = 7;
                                                break;
                                }
                                break;
                        case 1:
                                tui_draw_box(ADD_OWL_X, ADD_OWL_Y, TUI_COL, TUI_BKCOL, gfx_owl_04, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 2:
                                tui_draw_box(ADD_OWL_X, ADD_OWL_Y, TUI_COL, TUI_BKCOL, gfx_owl_03, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 8;
                        case 3:
                                tui_draw_box(ADD_OWL_X, ADD_OWL_Y, TUI_COL, TUI_BKCOL, gfx_owl_04, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 4:
                                tui_draw_box(ADD_OWL_X, ADD_OWL_Y, TUI_COL, TUI_BKCOL, gfx_owl_03, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step = 0;
                                return 18;
                        case 5:
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                gotoxy(73, 4);
                                putch('.');
                                gotoxy(75, 4);
                                putch('.');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 20;
                        case 6:
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                gotoxy(73, 4);
                                putch('*');
                                gotoxy(75, 4);
                                putch('*');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step = 0;
                                return 18;
                                
                        case 7:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(14, 22);
                                putch(')');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 8:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(17, 22);
                                putch('(');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 9:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(15, 23);
                                putch('(');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 10:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(17, 23);
                                putch(')');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 11:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(14, 24);
                                putch('(');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 12:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(13, 24);
                                putch(')');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                                
                        case 13:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(14, 22);
                                putch('(');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 14:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(17, 22);
                                putch(')');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 15:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(15, 23);
                                putch(')');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 16:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(17, 23);
                                putch('(');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 17:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(14, 24);
                                putch(')');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 2;
                        case 18:
                                tui_set_attr(0, LIGHTRED, TUI_BKCOL);
                                gotoxy(13, 24);
                                putch('(');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step ++;
                                return 2;
                                
                        case 19:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(13, 14);
                                putch('.');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;
                        case 20:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(13, 14);
                                putch('*');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;
                        case 21:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(16, 12);
                                putch('.');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;
                        case 22:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(16, 12);
                                putch('*');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;        
                        case 23:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(10, 10);
                                putch('.');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;
                        case 24:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(10, 10);
                                putch('*');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;        
                        case 25:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(12, 9);
                                putch('.');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 4;
                        case 26:
                                tui_set_attr(0, WHITE, TUI_BKCOL);
                                gotoxy(12, 9);
                                putch('*');
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step=0;
                                return 4;
                }
        }
        
        return 18;
}


long animate_scr_tet(enum W_ACTION a) {
        static int step = 0;

        if (a == RESET) {
                return 0;        
        }            
        
        if (a == RUN) {
                switch (step) {
                        case 0:
                                switch (rand() % 10) {
                                        case 1:
                                                step = 1;
                                                break;
                                        case 2:
                                                step = 7;
                                                break;
                                }
                                break;
                        case 1:
                                tui_draw_box(13, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_031, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 2:
                                tui_draw_box(14, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_03, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 16;
                        case 3:
                                tui_draw_box(14, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_04, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 4:
                                tui_draw_box(13, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_041, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 5:
                                tui_draw_box(12, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_04, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 6:
                                tui_draw_box(12, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_03, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step = 0;
                                return 18;
                        case 7:
                                tui_draw_box(21, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_041, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 8:
                                tui_draw_box(20, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_04, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 16;
                        case 9:
                                tui_draw_box(20, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_031, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 10:
                                tui_draw_box(21, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_03, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 11:
                                tui_draw_box(22, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_031, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step++;
                                return 5;
                        case 12:
                                tui_draw_box(22, 19, TUI_COL, TUI_BKCOL, gfx_rabbit_04, FALSE);
                                tui_set_attr(0, TUI_COL, TUI_BKCOL);
                                tui_flush();
                                step = 0;
                                return 18;
                }
        }
        
        return 18;
}

long animate_scr_xte(enum W_ACTION a) {
        static int x = 2;
        static int y = 10;
        static int direction = 0;

        if (a == RESET) {
                return 0;        
        }            
        
        if (a == RUN) {
                switch (y) {
                        case 9:
                                x = 2;
                                y = 10;
                                direction = 0;
                                break;
                        case 10:
                                if (direction == 0) {
                                        x = 2;
                                        y = 11;
                                } else {
                                        x = 3;
                                        y = 9;
                                }
                                break;
                        case 11:
                                x = 2;
                                y = 10;
                                direction = 1;
                                break;
                }
                
                tui_draw_box(x, y, TUI_COL, TUI_BKCOL, gfx_butter_01, FALSE);        
        }
        return 11;
}

