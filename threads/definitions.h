#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <stdatomic.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>


// liczby jednostek
#define REIND	9
#define GNOME	10

// klucze semaforow
#define S_REIND	988
#define S_GNOME	987
#define S_SANTA	986
#define S_RUN	985

int checkSem(int semid);
void podnies_bin(int semid, int semnum, int val);
void opusc_bin(int semid, int semnum, int val);

void podnies(int semid, int val);
void opusc(int semid, int val);

int initSem(int amnt, int key, int val);
void initQueueArr(int* id, int* wait, int size);
