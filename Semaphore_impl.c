#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 10
#define INCREMENT   10000

int counter_m = 0, counter_s = 0;
pthread_mutex_t mutex;
sem_t bin_sem;
sem_t count_sem;

void *mutex_task(void *arg) {
    for (int i = 0; i < INCREMENT; i++) {
        pthread_mutex_lock(&mutex);
        counter_m++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *sem_task(void *arg) {
    for (int i = 0; i < INCREMENT; i++) {
        sem_wait(&bin_sem);
        counter_s++;
        sem_post(&bin_sem);
    }
    return NULL;
}

void *resource_user(void *arg) {
    int id = (int)(long)arg;
    sem_wait(&count_sem);
    printf("  Thread %d ENTERED\n", id);
    sleep(1);
    printf("  Thread %d EXITING\n", id);
    sem_post(&count_sem);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    clock_t start, end;

    pthread_mutex_init(&mutex, NULL);
    start = clock();
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, mutex_task, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    end = clock();
    printf("Mutex:     counter = %d | time = %ld ms\n",
           counter_m, (end - start) * 1000 / CLOCKS_PER_SEC);
    pthread_mutex_destroy(&mutex);

    sem_init(&bin_sem, 0, 1);
    start = clock();
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, sem_task, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    end = clock();
    printf("Semaphore: counter = %d | time = %ld ms\n",
           counter_s, (end - start) * 1000 / CLOCKS_PER_SEC);
    sem_destroy(&bin_sem);

    printf("\n[Counting semaphore — only 3 threads at a time]\n");
    sem_init(&count_sem, 0, 3);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, resource_user, (void*)(long)i);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    sem_destroy(&count_sem);

    return 0;
}
