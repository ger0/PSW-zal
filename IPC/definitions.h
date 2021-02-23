#define AMNT 14
#define SIZE 15


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
    buf.sem_op	= -val;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
	fprintf(stderr, "semid: %i, semnum: %i, ", semid, semnum);
	perror("Opuszczenie semafora");
	exit(1);
    }
}

// queue
struct msgBuf {
    long 	mtype;
    char	mvalue[SIZE];
};

// strings
const char dania[AMNT][SIZE] = {
    "spaghetti", "pizza", "ravioli",
    "nalesniki", "bulka", "rosol",
    "bigos", "lasagne", "schabowy",
    "zurek", "barszcz", "zupa cebulowa",
    "pyry z gzikiem", "pierogi ruskie"
};
