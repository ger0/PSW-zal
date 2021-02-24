#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "definitions.h"

static bool isRunning = true;

// watki
static pthread_t p_gnome[GNOME];
static pthread_t p_reind[REIND];
static pthread_t p_santa;

// semafory
static int sem_reind;
static int sem_gnome;
static int sem_santa;

static int sem_bin;

// funkcje
void *reind() {
    while (isRunning) {
	opusc(sem_reind, 1);

	opusc(sem_bin, 1);
	if (checkSem(sem_reind) == 0) {
	    podnies(sem_santa, 1);
	} else {
	    podnies(sem_bin, 1);
	}
    }
}
void *gnome() {
    while (isRunning) {
	opusc(sem_gnome, 1);

	opusc(sem_bin, 1);
	if (checkSem(sem_gnome) == 0) {
	    podnies(sem_santa, 1);
	} else {
	    podnies(sem_bin, 1);
	}
    }
}
void *santa() {
    while (isRunning) {
	printf("Mikolaj poszedl spac\n\n");
	opusc(sem_santa, 1);
	printf("Obudzono mikolaja\n");

	if (checkSem(sem_reind) == 0) {
	    printf("Zaprzegniecie reniferow\n");
	    podnies(sem_reind, 9);
	}
	if (checkSem(sem_gnome) == 0) {
	    printf("Obrady ze skrzatami\n");
	    podnies(sem_gnome, 3);
	}
	podnies(sem_bin, 1);
    }
}

int main () {  
    // inicjalizacja semaforow
    sem_reind = initSem(1, S_REIND, REIND);
    sem_gnome = initSem(1, S_GNOME, 3);
    sem_santa = initSem(1, S_SANTA, 0);

    sem_bin = initSem(1, S_RUN, 1);

    // tworzenie watkow
    for (unsigned i = 0; i < REIND; i++) {
	pthread_create(&p_reind[i], NULL, reind, NULL); 
    }
    for (unsigned i = 0; i < GNOME; i++) {
	pthread_create(&p_gnome[i], NULL, gnome, NULL); 
    }
    pthread_create(&p_santa, NULL, santa, NULL); 

    // konczenie pracy
    pthread_join(p_santa, NULL);
    for (unsigned i = 0; i < REIND; i++) {
	pthread_join(p_reind[i], NULL);
    }
    for (unsigned i = 0; i < GNOME; i++) {
	pthread_join(p_gnome[i], NULL);
    }
    semctl(sem_reind, IPC_RMID, 0);
    semctl(sem_gnome, IPC_RMID, 0);
    semctl(sem_santa, IPC_RMID, 0);
    return 0;
}
