#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "definitions.h"

int checkSem(int semid) {
    return semctl(semid, 0, GETVAL, 0);
}

void podnies(int semid, int semnum, int val) {
    struct sembuf buf;
    buf.sem_num	= semnum;
    buf.sem_op	= val;
    buf.sem_flg	= 0;
    if (semop(semid, &buf, 1) == -1) {
	fprintf(stderr, "semid: %i, semnum: %i, ", semid, semnum);
	perror("Podnoszenie semafora");
	exit(1);
    }
}
void opusc(int semid, int semnum, int val) {
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op	= - val;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
	fprintf(stderr, "semid: %i, semnum: %i, ", semid, semnum);
	perror("Opuszczenie semafora");
	exit(1);
    }
}

int initSemaphore(int amnt, int key, int val) {
    int semid = semget(key, amnt, IPC_CREAT|0600);
    if (semid == -1) {
	semid = semget(key, COOKS, 0600);
	if (semid == -1) {
	    perror("Tworzenie tablicy semaforow");
	    exit(1);
	}
    }
    for (unsigned i = 0; i < amnt; i++) {
	if (semctl(semid, i, SETVAL, val) == -1) {
	    perror("Nadanie wartosci semaforowi");
	    exit(1);
	}
    }
    return semid;
}

void takeForks(unsigned *ID, int semid) {
    if (*ID != (unsigned)COOKS - 1) {
	opusc(semid, *ID, 1);
	opusc(semid, *ID + 1, 1);
    } else {
	opusc(semid, 0, 1);
	opusc(semid, *ID, 1);
    }
}
void freeForks(unsigned *ID, int semid) {
    if (*ID != (unsigned)COOKS - 1) {
	podnies(semid, *ID + 1, 1);
	podnies(semid, *ID, 1);
    } else {
	podnies(semid, *ID, 1);
	podnies(semid, 0, 1);
    }
}

// testowanie
void test(int var, int bar) {
    if (var > K) {
	printf("POJEMNOSC! %i\n", var);
    }
    if (bar > W) {
	printf("OBCIAZ! %i\n", bar);
    }
}
