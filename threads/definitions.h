// liczby jednostek
#define REIND	9
#define GNOME	10

// klucze semaforow
#define S_REIND	988
#define S_GNOME	987
#define S_SANTA	986
#define S_RUN	985

int checkSem(int semid) {
    return semctl(semid, 0, GETVAL, 0);
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
