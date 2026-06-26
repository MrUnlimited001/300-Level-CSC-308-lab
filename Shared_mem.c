#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define SHM_KEY  0x1234
#define SEM_KEY  0x5678
#define SHM_SIZE 1024

void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget failed"); exit(1); }

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) { perror("shmat failed"); exit(1); }

    pid_t pid = fork();

    if (pid == 0) {
        char *child_shm = (char *)shmat(shmid, NULL, 0);
        sleep(1);
        sem_lock(semid);
        printf("[Child]  Read: \"%s\"\n", child_shm);
        sem_unlock(semid);
        shmdt(child_shm);
    } else {
        sem_lock(semid);
        snprintf(shm_ptr, SHM_SIZE, "IPC message from parent | PID: %d", getpid());
        printf("[Parent] Wrote: \"%s\"\n", shm_ptr);
        sem_unlock(semid);
        wait(NULL);
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        printf("[Parent] Cleaned up.\n");
    }
    return 0;
}
