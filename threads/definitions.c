#include "definitions.h"

int checkSem(int semid) {
    return semctl(semid, 0, GETVAL, 0);
}

void podnies_bin(int semid, int semnum, int val) {
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

void opusc_bin(int semid, int semnum, int val) {
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

void podnies(int semid, int val) {
    struct sembuf buf;
    buf.sem_num	= 0;
    buf.sem_op	= val;
    buf.sem_flg	= 0;
    if (semop(semid, &buf, 1) == -1) {
	fprintf(stderr, "semid: %i", semid);
	perror("Podnoszenie semafora");
	exit(1);
    }
}

void opusc(int semid, int val) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op	= - val;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
	fprintf(stderr, "semid: %i", semid);
	perror("Opuszczenie semafora");
	exit(1);
    }
}

int initSem(int amnt, int key, int val) {
    int semid = semget(key, amnt, IPC_CREAT|0600);
    if (semid == -1) {
	semid = semget(key, amnt, 0600);
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

void initQueueArr(int* id, int* wait, int size) {
    for (int i = 0; i < size; i++) {
	id[i]	= i;
	wait[i] = 0;
	//printf("i: %i, id: %i, wait%i\n", i, id[i], wait[i]);
    }
}
