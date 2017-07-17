/*
*       wait_tc.c
*
*       This file is part of Tetris game.
*       https://bitbucket.org/berk76/tetris
*
*       Tetris is free software; you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation; either version 3 of the License, or
*       (at your option) any later version. <http://www.gnu.org/licenses/>
*
*       Written by Jaroslav Beran <jaroslav.beran@gmail.com>, on 6.7.2017
*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "wait_tc.h"


static JOB_T *job_q = NULL;


void w_wait(long tck) {
        long ret;
        JOB_T *j, *pq;

        tck += clock();
        while (1) {
                pq = job_q;
                j = NULL;
                while (pq != NULL) {
                        if ((j == NULL) || (j->endwait > pq->endwait)) {
                                j = pq;
                        } 
                        pq = pq->next;
                }
                if ((j != NULL) && (clock() >= j->endwait) && (tck > j->endwait)) {
                        ret = j->run(RUN);
                        if (ret != 0) {
                                j->endwait = ret + clock();
                        } else {
                                j->endwait = j->period + clock();
                        }
                } else 
                if (clock() >= tck) {
                        return;
                }
        }
}


JOB_T * w_register_job(long tck, long (*run)(enum W_ACTION)) {
        JOB_T *j;
        
        j = (JOB_T *) malloc(sizeof(JOB_T));
        assert(j != NULL);
        
        j->run = run;
        j->period = tck;
        j->endwait = tck + clock();
        j->prev = NULL;
        j->next = job_q;
        job_q = j;
        if (j->next != NULL) {
                (j->next)->prev = j;        
        } 
        
        return j;
}


void w_unregister_job(JOB_T *j) {
        assert(j != NULL);
        
        if (j->prev != NULL) {
                (j->prev)->next = j->next;
                if (j->next != NULL) {
                        (j->next)->prev = j->prev;
                }
        } else {
                job_q = j->next;
                if (j->next != NULL) {
                        (j->next)->prev = NULL;
                } 
        }
        
        free((void *) j);
}

