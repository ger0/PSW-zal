#pragma once

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

int checkSem(int semid);

void podnies(int semid, int semnum, int val);
void opusc(int semid, int semnum, int val);
int initSemaphore(int amnt, int key, int val);

void takeForks(unsigned *ID, int semid);
void freeForks(unsigned *ID, int semid);

struct occBuf {
    long 	mtype;
    int		mvalue;
};

void test(int var, int bar);
