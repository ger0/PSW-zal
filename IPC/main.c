#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "definitions.h"

static unsigned state[COOKS];
static unsigned tID[COOKS];
static bool isRunning = true;

// widelce
static int sem_forks;
// zajestosc stolu
static int sem_avail;
static int sem_taken; 		
// obciazenie stolu
static int avail_weight;
static int taken_weight;
// kolejka komunikatow
static int occup;

void funcKill() {
    isRunning = false;
    perror("WYMUSZONO ZAKONCZENIE PROCESU");
    if (msgctl(occup, IPC_RMID, 0) == -1) {
	perror("Kasacja kolejki");
	exit(1);
    }
    pthread_kill(pthread_self(), SIGKILL);
}

void *func(void *p_id) {
    // identyfikator kucharza
    unsigned 	*ID = p_id;
    unsigned	*seed = &state[*ID];
    struct	occBuf buf;
    while (isRunning) {
	// gotowanie przy odp pojemnosci i obciazeniu stolu
	if (checkSem(sem_avail) > 0 && checkSem(avail_weight) > 0 
		&& checkSem(sem_taken) < 1) {
	    opusc(sem_avail, 0, 1);
	    if (msgrcv(occup, &buf, sizeof(buf.mvalue), PUSTY, 0) == -1) {
		perror("Odebranie pustego komunikatu");
		exit(1);
	    }
	    buf.mtype	= PELNY;
	    buf.mvalue	= rand_r(seed) % 5 + 1;

	    opusc(avail_weight, 0, buf.mvalue);
	    takeForks(ID, sem_forks);

	    printf("(+) PRZYGOTOWANE danie o wadze: %d \n", buf.mvalue);
	    if (msgsnd(occup, &buf, sizeof(buf.mvalue), 0) == -1) {
		perror("Wyslanie pelnego komunikatu");
		exit(1);
	    }

	    podnies(sem_taken, 0, 1);
	    podnies(taken_weight, 0, buf.mvalue);
	}
	// konsumpcja
	else {
	    opusc(sem_taken, 0, 1);
	    takeForks(ID, sem_forks);

	    if (msgrcv(occup, &buf, sizeof(buf.mvalue), PELNY, 0) == -1) {
		perror("Odebranie Pelnego komunikatu");
		exit(1);
	    }
	    printf("(-) SKONSUMOWANE danie o wadze: %d \n", buf.mvalue);
	    opusc(taken_weight, 0, buf.mvalue);

	    buf.mtype = PUSTY;
	    if (msgsnd(occup, &buf, sizeof(buf.mvalue), 0) == -1) {
		perror("Wyslanie pustego komunikatu");
		exit(1);
	    }
	    podnies(sem_avail, 0, 1);
	    podnies(avail_weight, 0, buf.mvalue);
	}
	freeForks(ID, sem_forks);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    signal(SIGINT, funcKill);

    pthread_t p[COOKS];

    // inicjalizacja semaforow
    sem_forks = initSemaphore(COOKS, FORKS, 1); 
    sem_avail = initSemaphore(1, AVAIL_SPACE, (int)K);  
    sem_taken = initSemaphore(1, TAKEN_SPACE, 0);  
    avail_weight = initSemaphore(1, AVAIL_WEIGHT, (int)W);  
    taken_weight = initSemaphore(1, TAKEN_WEIGHT, 0);  

    struct occBuf elem;
    elem.mtype = PUSTY;
    elem.mvalue = 0;
    // tworzenie kolejki komuniktow i wysylanie pustych elementow
    occup = msgget(QUEUE, IPC_CREAT|IPC_EXCL|0600);
    if (occup == -1) {
	occup = msgget(QUEUE, IPC_CREAT|0600);
	if (occup == -1) {
	    perror("Przylaczenie kolejki");
	    exit(1);
	}
    } else {
	for (int i = 0; i < K; i++) {
	    if (msgsnd(occup, &elem, sizeof(elem.mvalue), 0) == -1) {
		perror("Wyslanie pustego komunikatu");
		exit(1);
	    }
	}
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
	if (pthread_join(p[i], NULL));
    }
    return 0;
}
