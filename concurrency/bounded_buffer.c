#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>
#define MAX 100

int buffer[MAX];
int fill = 0;
int use = 0;
int count = 0;
int psum = 0;
int csum = 0;
int loops;

pthread_cond_t empty, filled;
pthread_mutex_t mutex;

void put(int value){
    buffer[fill] = value;
    fill = (fill+1)%MAX;
    count++;
    psum += (value-fill);
}
int get(){
    int tmp = buffer[use];
    use = (use + 1)%MAX;
    count--;
    csum += (tmp-use);
    return tmp;
}
void *producer(void *arg){
    int i;
    for(i=0;i<loops;i++){
        pthread_mutex_lock(&mutex);
        while(count == MAX)
            pthread_cond_wait(&empty, &mutex);
        put(i);
        pthread_cond_signal(&filled);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
void *consumer(void *arg){
    int i;
    for(i=0;i<loops;i++){
        pthread_mutex_lock(&mutex);
        while(count == 0)
            pthread_cond_wait(&filled, &mutex);
        int tmp = get();
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
        printf("%d\n",tmp);
    }
    return NULL;
}

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr,"usage:threads <value>\n");
        exit(1);
    }
    
    int rc1 = pthread_mutex_init(&mutex, NULL);
    int rc2 = pthread_cond_init(&empty, NULL);
    int rc3 = pthread_cond_init(&filled, NULL);
    assert(rc1 == 0 && rc2 == 0 && rc3 == 0);

    loops = atoi(argv[1]);
    pthread_t p1, p2, p3, p4;

    pthread_create(&p1, NULL, producer, NULL);
    pthread_create(&p2, NULL, consumer, NULL);
    pthread_create(&p3, NULL, producer, NULL);
    pthread_create(&p4, NULL, consumer, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);

    if(psum==csum)
        printf("TEST OK\n");
    else
        printf("TEST WRONG\n");

    return 0;
}
