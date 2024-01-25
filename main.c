#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>

int numsegnali;
int pa[2];
int sigusr1cnt = 0;
int sigusr2cnt = 0;

void body_figlio(int signo);
void body_padre ();

void p1_hanlder () {
    ++sigusr1cnt;
    printf("%d riceve SIGUSR1: %d\n", getpid(), sigusr1cnt);
}

void p2_hanlder () {
    ++sigusr2cnt;
    printf("%d riceve SIGUSR1: %d\n", getpid(), sigusr2cnt);
}

int main (int argc, char *argv[]) {

    int pid, ppid;
    struct sigaction act;


    sigset_t sigmask1, sigmask2, sigmaskpa;

    if (argc != 2) {
        printf("Numero di argomenti errato");
        exit(1);
    }

    numsegnali = atoi(argv[1]);

    // gestione del segnale per pa
    act.sa_handler = p1_hanlder;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR2); // blocca SIGUSR1 nel gestore
    act.sa_flags = SA_RESTART; // restart automatico di una primitiva di lettura interrotta dal segnale SIGUSR1
    sigaction(SIGUSR1, &act, NULL);
    act.sa_handler = p2_hanlder;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &act, NULL);

    // 3. creazione della pipe

    if (pipe(pa) < 0) {
        perror("Errore creazione pipe");
        exit(2);
    }

    // 4. creazione dei processi figli

    if ((pid = fork()) < 0) {
        perror("Errore nella fork");
        exit(3);
    }
    else if ((pid = fork()) == 0) {
        // Processo p1
        body_figlio(SIGUSR1);
        exit(0);
    }
    else {
        if ((pid = fork()) == 0) {
            // Processo p2
            body_figlio(SIGUSR2);
            exit(0);
        }
        else {
            // Processo padre
            body_padre();
        }
    }
}

void body_figlio (int signo) {
    int mesg[2];

    for (int i = 0; i < numsegnali; i++) {
        kill(getppid(), signo);
        printf("PROCESSO %d invia messaggio a %d\n", getpid(), getppid());
        sleep(1+ rand() % 5);
    }

    mesg[0] = getpid();
    mesg[1] = numsegnali;

    write(pa[1], mesg, sizeof mesg);
}

void body_padre() {
    int msg1[2], msg2[2];
    read(pa[0], msg1, sizeof msg1);
    read(pa[0], msg2, sizeof msg2);

    printf("Processo %d ha mandato %d segnali\n", msg1[0], msg1[1]);
    printf("Processo %d ha mandato %d segnali\n", msg2[0], msg2[1]);
    printf("Ricevuti %d SIGUSR1 e %d SIGUSR2\n", sigusr1cnt, sigusr2cnt);
}

