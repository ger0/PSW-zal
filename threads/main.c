#include "definitions.h"

static atomic_bool isRunning;

// binarne semafory dla kazdego watku 
static int bin_reind;
static int bin_gnome;

// tablice z id procesow
static int reinID[REIND];
static int gnomID[GNOME];

// tablice z id procesow w kolejce
static int rein_wait[REIND];
static int gnom_wait[GNOME];

// watki
static pthread_t p_gnome[GNOME];
static pthread_t p_reind[REIND];
static pthread_t p_santa;

// semafory
static int sem_reind;
static int sem_gnome;
static int sem_santa;

static int sem_bin;

// konczenie pracy SIGINT
void funcINT() {
    isRunning = false;
    for (int i = 0; i < 9; i++) {
	printf("i: %i - %i\n", rein_wait[i]);
    }
}

// funkcje
void *reind(void *addr) {
    int *id = (int*)addr;
    signal(SIGINT, NULL);
    while (isRunning) {
	opusc_bin(bin_reind, *id, 1);
	opusc(sem_reind, 1);

	rein_wait[*id] = 1;

	opusc(sem_bin, 1);
	if (checkSem(sem_reind) == 0) {
	    podnies(sem_santa, 1);
	} else {
	    podnies(sem_bin, 1);
	}
    }
}
void *gnome(void *addr) {
    int *id = (int*)addr;
    signal(SIGINT, NULL);
    while (isRunning) {
	opusc_bin(bin_gnome, *id, 1);
	opusc(sem_gnome, 1);

	gnom_wait[*id] = 1;

	opusc(sem_bin, 1);
	if (checkSem(sem_gnome) == 0) {
	    podnies(sem_santa, 1);
	} else {
	    podnies(sem_bin, 1);
	}
    }
}
void *santa() {
    signal(SIGINT, NULL);
    while (isRunning) {
	printf("Mikolaj poszedl spac\n\n");
	opusc(sem_santa, 1);
	printf("Obudzono mikolaja\n");

	if (checkSem(sem_reind) == 0) {
	    printf("Zaprzegniecie reniferow\n");
	    for (int i = 0; i < REIND; i++) {
		rein_wait[i] = 0;
		podnies_bin(bin_reind, i, 1);
	    }
	    printf("Odprowadzenie reniferow\n");
	    podnies(sem_reind, REIND);
	}
	if (checkSem(sem_gnome) == 0) {
	    printf("Obrady ze skrzatami\n");
	    for (unsigned i = 0; i < GNOME; i++) {
		if (gnom_wait[i] == 1) {
		    gnom_wait[i] = 0;
		    podnies_bin(bin_gnome, i, 1);
		}
	    }
	    podnies(sem_gnome, 3);
	}
	podnies(sem_bin, 1);
    }
}

int main () {  
    isRunning = true;
    signal(SIGINT, funcINT);
    // inicjalizacja semaforow
    sem_reind = initSem(1, S_REIND, REIND);
    sem_gnome = initSem(1, S_GNOME, 3);
    sem_santa = initSem(1, S_SANTA, 0);

    bin_reind = initSem(9, S_REIND + 200, 1);
    bin_gnome = initSem(10, S_GNOME + 200, 1);

    sem_bin = initSem(1, S_RUN, 1);
    
    // wait	----------------------------
    initQueueArr(reinID, rein_wait, (int)REIND);
    initQueueArr(gnomID, gnom_wait, (int)GNOME);

    // tworzenie watkow
    for (int i = 0; i < REIND; i++) {
	pthread_create(&p_reind[i], NULL, reind, &reinID[i]); 
    }
    for (int i = 0; i < GNOME; i++) {
	pthread_create(&p_gnome[i], NULL, gnome, &gnomID[i]); 
    }
    pthread_create(&p_santa, NULL, santa, NULL); 

    // konczenie pracy
    pthread_join(p_santa, NULL);
    for (unsigned i = 0; i < REIND; i++) {
	pthread_join(p_reind[i], NULL);
    }
    for (unsigned i = 0; i < REIND; i++) {
	pthread_join(p_reind[i], NULL);
    }
    for (unsigned i = 0; i < GNOME; i++) {
	pthread_join(p_gnome[i], NULL);
    }
    semctl(sem_reind, IPC_RMID, 0);
    semctl(sem_gnome, IPC_RMID, 0);
    semctl(sem_santa, IPC_RMID, 0);

    semctl(sem_bin, IPC_RMID, 0);

    semctl(bin_reind, IPC_RMID, 0);
    semctl(bin_gnome, IPC_RMID, 0);
    return 0;
}
