#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int counter = 0;

void* count()
{
    pthread_mutex_lock(&mutex1);
    counter++;
    printf("Counter value : %d\n", counter);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

int main()
{
    int rc1, rc2;
    pthread_t thread1, thread2;

    if ( (rc1 = pthread_create(&thread1, NULL, &count, NULL)) ){
        printf("thread creation failed : %d \n", rc1);
    }

    if ( (rc2 = pthread_create(&thread2, NULL, &count, NULL)) ){
        printf("thread creation failed : %d \n", rc1);
    }

    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);

    return 0;
}
