#define PUSTY 1
#define PELNY 2

#define COOKS	5	// liczba kucharzy
#define K	10	// max pojemnosc
#define W	20	// max obciazenie

// KLUCZE 
#define FORKS		100
#define AVAIL_SPACE	200
#define TAKEN_SPACE	300

#define QUEUE		999

#define AVAIL_WEIGHT	400
#define TAKEN_WEIGHT	500

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

// queue
struct occBuf {
    long 	mtype;
    int		mvalue;
};

// testowanie
void test(int var, int bar) {
    if (var > K) {
	printf("POJEMNOSC! %i\n", var);
    }
    if (bar > W) {
	printf("OBCIAZ! %i\n", bar);
    }
}
