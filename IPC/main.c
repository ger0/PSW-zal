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

int min(int a, int b) {
    if (a > b) {
	return b;
    } else {
	return a;
    }
}

static unsigned state[COOKS];
static unsigned tID[COOKS];

static bool isRunning = true;

void *func(void *p_id) {
    // identyfikator kucharza
    unsigned 	*ID = p_id;
    unsigned	*seed = &state[*ID];
    struct	msgBuf elem;
    int 	msgid;

    int 	sem_forks, sem_avail, sem_taken;
    int		avail_weight, taken_weight;
    int		mutex;

    mutex	= semget(MUTEX, 1, 0600);
    sem_forks	= semget(FORKS, COOKS, 0600);

    sem_avail	= semget(AVAIL_SPACE, 1, 0600);
    sem_taken	= semget(TAKEN_SPACE, 1, 0600);

    avail_weight = semget(AVAIL_WEIGHT, 1, 0600);
    taken_weight = semget(TAKEN_WEIGHT, 1, 0600);

    msgid = msgget(145227, 0600);
    if (msgid == -1) {
	perror("Przylaczenie kolejki");
	exit(1);
    }
    while (isRunning) {
	// gotowanie przy odp pojemnosci i obciazeniu stolu
	/*
	if (checkSem(sem_avail) > 0 && checkSem(avail_weight) > 0 && 
	      ((checkSem(sem_taken) < 1) && (checkSem(taken_weight) < 1))) {
	      */
	if (checkSem(avail_weight) > 0 && checkSem(taken_weight) < W - 1) {
	    // przygotowywanie potrawy
	    elem.mtype 	= rand_r(seed) % 5 + 1;
	    memcpy(elem.mvalue, dania[rand_r(seed) % AMNT], SIZE);

	    //opusc(sem_avail, 0, 1);
	    opusc(avail_weight, 0, (int)(elem.mtype));
	    //takeForks(ID, sem_forks);
	    
	    opusc(mutex, 0, 1);
	    printf("DO KOLEJKI\n"); 
	    if (msgsnd(msgid, &elem, sizeof(elem.mvalue), 0) == -1) {
		fprintf(stderr, "wrongP: %-15s o wadze: %d \n", 
			elem.mvalue, elem.mtype);
		perror("Przygotowanie dania");
		exit(1);
	    }
	    printf("AvailW: %i, TakenW: %i\n", checkSem(avail_weight), checkSem(taken_weight));
	    printf("Avail: %i, Taken: %i\n", checkSem(sem_avail), checkSem(sem_taken));
	    printf("(+) PRZYGOTOWANE: %-15s o wadze: %d \n", 
		    elem.mvalue, elem.mtype);
	    podnies(mutex, 0, 1);

	    //podnies(sem_taken, 0, 1);
	    podnies(taken_weight, 0, (int)(elem.mtype));
	}
	// konsumpcja
	else if (checkSem(taken_weight) > 0){
	    //opusc(sem_taken, 0, 1);
	    //takeForks(ID, sem_forks);
	    opusc(mutex, 0, 1);
	    // konsumowanie potrawy
	    if (msgrcv(msgid, &elem, sizeof(elem.mvalue), 0, 0) == -1) {
		perror("Spozywanie dania");
		exit(1);
	    }
	    printf("(-) SKONSUMOWANE: %-15s o wadze: %d \n", 
		    elem.mvalue, elem.mtype);
	    podnies(mutex, 0, 1);
	    opusc(taken_weight, 0, (int)(elem.mtype));

	    //podnies(sem_avail, 0, 1);
	    podnies(avail_weight, 0, (int)(elem.mtype));
	}
	// ZWALNIANIE WIDELCOW
	//freeForks(ID, sem_forks);
	//sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t p[COOKS];

    // zajestosc stolu
    int sem_avail;
    int	sem_taken; 		
    // obciazenie stolu
    int avail_weight;
    int taken_weight;

    // inicjalizacja semaforow
    initSemaphore(COOKS, FORKS, 1); 

    sem_avail = initSemaphore(1, AVAIL_SPACE, (int)K);  
    sem_taken = initSemaphore(1, TAKEN_SPACE, 0);  

    avail_weight = initSemaphore(1, AVAIL_WEIGHT, (int)W);  
    taken_weight = initSemaphore(1, TAKEN_WEIGHT, 0);  

    initSemaphore(1, MUTEX, 1);

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
