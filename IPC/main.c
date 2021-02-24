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

#define PUSTY 1
#define PELNY 2

static unsigned state[COOKS];
static unsigned tID[COOKS];
static pthread_t p[COOKS];

static bool isRunning = true;

void funcKill() {
    perror("WYMUSZONO ZAKONCZENIE PROCESU");
    int occup = msgget(QUEUE, IPC_CREAT|IPC_EXCL|0600);
    if (occup == -1) {
	occup = msgget(QUEUE, IPC_CREAT|0600);
	if (occup == -1) {
	    perror("Przylaczenie kolejki");
	    exit(1);
	}
    }
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
    int 	occup;

    int 	sem_forks, sem_avail, sem_taken;
    int		avail_weight, taken_weight;

    sem_forks	= semget(FORKS, COOKS, 0600);

    sem_avail	= semget(AVAIL_SPACE, 1, 0600);
    sem_taken	= semget(TAKEN_SPACE, 1, 0600);

    avail_weight = semget(AVAIL_WEIGHT, 1, 0600);
    taken_weight = semget(TAKEN_WEIGHT, 1, 0600);

    occup = msgget(QUEUE, IPC_CREAT|IPC_EXCL|0600);
    if (occup == -1) {
	occup = msgget(QUEUE, IPC_CREAT|0600);
	if (occup == -1) {
	    perror("Przylaczenie kolejki");
	    exit(1);
	}
    }
    while (isRunning) {
	// gotowanie przy odp pojemnosci i obciazeniu stolu
	if (checkSem(sem_avail) > 0 && checkSem(avail_weight) > 0 &&
		(checkSem(sem_taken) < 1)) {
	    opusc(sem_avail, 0, 1);
	    if (msgrcv(occup, &buf, sizeof(buf.mvalue), PUSTY, 0) == -1) {
		perror("Odebranie pustego komunikatu");
		exit(1);
	    }
	    buf.mtype	= PELNY;
	    buf.mvalue	= rand_r(seed) % 5 + 1;

	    opusc(avail_weight, 0, buf.mvalue);
	    takeForks(ID, sem_forks);

	    printf("(+) PRZYGOTOWANE o wadze: %d \n", buf.mvalue);
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
	    opusc(taken_weight, 0, buf.mvalue);
	    printf("(-) SKONSUMOWANE o wadze: %d \n", buf.mvalue);

	    buf.mtype = PUSTY;
	    if (msgsnd(occup, &buf, sizeof(buf.mvalue), 0) == -1) {
		perror("Wyslanie pustego komunikatu");
		exit(1);
	    }
	    podnies(sem_avail, 0, 1);
	    podnies(avail_weight, 0, buf.mvalue);
	}
	// ZWALNIANIE WIDELCOW
	freeForks(ID, sem_forks);
	//sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    signal(SIGINT, funcKill);
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

    struct occBuf elem;
    elem.mtype = PUSTY;
    elem.mvalue = 0;
    int occup = msgget(QUEUE, IPC_CREAT|IPC_EXCL|0600);
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
