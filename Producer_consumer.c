#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t mutex_sem;
sem_t empty_sem;
sem_t full_sem;

void *producer(void *arg) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100;
        sem_wait(&empty_sem);
        sem_wait(&mutex_sem);
        buffer[in] = item;
        printf("[Producer] Wrote %d at buffer[%d]\n", item, in);
        in = (in + 1) % BUFFER_SIZE;
        sem_post(&mutex_sem);
        sem_post(&full_sem);
        sleep(1);
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        sem_wait(&full_sem);
        sem_wait(&mutex_sem);
        int item = buffer[out];
        printf("[Consumer] Read  %d from buffer[%d]\n", item, out);
        out = (out + 1) % BUFFER_SIZE;
        sem_post(&mutex_sem);
        sem_post(&empty_sem);
        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;
    sem_init(&mutex_sem, 0, 1);
    sem_init(&empty_sem, 0, BUFFER_SIZE);
    sem_init(&full_sem,  0, 0);
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    sem_destroy(&mutex_sem);
    sem_destroy(&empty_sem);
    sem_destroy(&full_sem);
    return 0;
}
