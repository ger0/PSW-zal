#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "definitions.h"

#define COOKS	5	// liczba kucharzy
#define K	10	// max pojemnosc
#define W	40	// max obciazenie

// KLUCZE 
#define FORKS		100
#define AVAIL_SPACE	200
#define TAKEN_SPACE	300

unsigned state[COOKS];
unsigned tID[COOKS];

void initSemaphore(int amnt, int key, int val) {
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

void *func(void *p_id) {
    // identyfikator kucharza
    unsigned 	*ID = p_id;
    unsigned	*seed = &state[*ID];
    struct	msgBuf elem;
    int		isRunning = 1;

    int 	sem_forks;
    int 	sem_avail;
    int 	sem_taken;
    int		msgid;

    sem_forks	= semget(FORKS, COOKS, 0600);
    sem_avail	= semget(AVAIL_SPACE, 1, 0600);
    sem_taken	= semget(TAKEN_SPACE, 1, 0600);

    msgid = msgget(145227, 0600);
    if (msgid == -1) {
	perror("Przylaczenie kolejki");
	exit(1);
    }
    while (isRunning) {
	// gotowanie
	if ((rand_r(seed) % 10) < 5 && checkSem(sem_avail) > 0) {
	    opusc(sem_avail, 0, 1);

	    elem.mtype 	= rand_r(seed) % 8 + 1;
	    memcpy(elem.mvalue, dania[rand_r(seed) % AMNT], SIZE);

	    takeForks(ID, sem_forks);
	    
	    if (msgsnd(msgid, &elem, sizeof(elem.mvalue), 0) == -1) {
		perror("Przygotowanie dania");
		exit(1);
	    }
	    fprintf(stdout, "(+) PRZYGOTOWANE: %-15s o wadze: %d \n", 
		    elem.mvalue, elem.mtype);
	    podnies(sem_taken, 0, 1);
	}
	// konsumpcja +++ tylko jezeli jest przynajmniej jeden element w kolejce
	else if (checkSem(sem_taken) > 0) {
	    takeForks(ID, sem_forks);
	    if (msgrcv(msgid, &elem, sizeof(elem.mvalue), 0, 0) == -1) {
		perror("Spozywanie dania");
		exit(1);
	    } else {
		fprintf(stdout, "(-) SKONSUMOWANE: %-15s o wadze: %d \n", 
			elem.mvalue, elem.mtype);
		podnies(sem_avail, 0, 1);
		opusc(sem_taken, 0, 1);
	    }
	}
	// ZWALNIANIE WIDELCOW
	freeForks(ID, sem_forks);
	sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t p[COOKS];
    int semid;		// widelce
    int semempt;	// 

    // inicjalizacja widelcow
    initSemaphore(COOKS, FORKS, 1); 

    // ile dan jest juz na stole
    initSemaphore(1, AVAIL_SPACE, (int)K);  
    initSemaphore(1, TAKEN_SPACE, 0);  

    // tworzenie kolejki komunikatow
    int msgid = msgget(145227, IPC_CREAT|0600);
    if (msgid == -1) {
	perror("Utworzenie kolejki komunikatow");
	exit(1);
    }
    // tworzenie watkow - kucharzy
    for (unsigned i = 0; i < COOKS; i++) {
	tID[i] = i;
	state[i] = rand();
	if (pthread_create(&p[i], NULL, func, &tID[i])) {
	    perror("Tworzenie watku");
	    exit(1);
	}
    }
    for (unsigned i = 0; i < COOKS; i++) {
	if (pthread_join(p[i], NULL)) {
	    perror("Laczenie watkow");
	    exit(1);
	}
    }
    return 0;
}