#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Semaphore implemented with a FIFO waiting queue
typedef struct
{
    int value;
    pthread_mutex_t lock;   //Mutex Lock 
    pthread_cond_t wait;    //Condition Variable
} semaphore;

/*
    This function creates a sempahore dynamically with given initial value 
    returns a pointer to the semaphore
*/
semaphore *init(int intital_value)
{       
        semaphore* temp=(semaphore *)malloc(sizeof(semaphore));
        if(temp == NULL) 
        {
                return NULL;
        }
        temp->value = intital_value;
        pthread_mutex_init(&(temp->lock),NULL);
        pthread_cond_init(&(temp->wait),NULL);
        return temp;
}

/*
    Wait method of a semaphore  decrements its value 
    and puts the thread in the waiting queue of semaphore 
    if it is not the first thread. 
*/
void semaphore_wait(semaphore * sem)
{
        pthread_mutex_lock(&(sem->lock));
        sem->value--;
        if(sem->value<0)
            pthread_cond_wait(&(sem->wait),&(sem->lock));
        pthread_mutex_unlock(&(sem->lock));
}


/*
    Signal method is used to increment the semaphore value and
    wakeup a blocked thread from the waiting queue in FIFO manner
*/
void semaphore_signal(semaphore *sem)
{

        pthread_mutex_lock(&(sem->lock));
        sem->value++;
        if(sem->value <= 0)
            pthread_cond_signal(&(sem->wait));
        pthread_mutex_unlock(&(sem->lock));
}

//Writers code; All the writers attempt to write this.

semaphore resourceAccess,orderQueue,readerMutex;
int shared_variable=0,readcount=0;
void *writer(void* writer_number)
{   
    int num=*(int*)writer_number;
    semaphore_wait(&orderQueue);
    semaphore_wait(&resourceAccess);
    semaphore_signal(&orderQueue);

    shared_variable+=2;
    printf("Writer %d modified value of shared variable to %d\n",num,shared_variable);
    
    semaphore_signal(&resourceAccess);

}
/*
    Readers Code:- All the readers are synchronized in a starve free manner 
*/
void *reader(void *reader_number)
{   
    int num=*(int*)reader_number;
    semaphore_wait(&orderQueue);
    semaphore_wait(&readerMutex);
    readcount++;
    if(readcount==1)
        semaphore_wait(&resourceAccess);
    semaphore_signal(&orderQueue);
    semaphore_signal(&readerMutex);

    printf("Reader %d: read the shared variable as %d\n",num,shared_variable);


    semaphore_wait(&readerMutex);
    readcount--;
    if(readcount==0)
        semaphore_signal(&resourceAccess);
    semaphore_signal(&readerMutex);
}


/*
    Main function where we perform reads and writes which result in a synchronized output.
*/
int main()
{   

    pthread_t read[6],write[5];
    resourceAccess=* init(1);
    readerMutex=*init(1);
    orderQueue=*init(1);

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; 


    pthread_create(&read[0], NULL, (void *)reader, (void *)&a[0]);
    pthread_create(&read[1], NULL, (void *)reader, (void *)&a[1]);
    pthread_create(&write[0], NULL, (void *)writer, (void *)&a[0]);
    pthread_create(&write[1], NULL, (void *)writer, (void *)&a[1]);
    pthread_create(&read[2], NULL, (void *)reader, (void *)&a[2]);
    pthread_create(&write[2], NULL, (void *)writer, (void *)&a[2]);
    pthread_create(&read[3], NULL, (void *)reader, (void *)&a[3]);
    pthread_create(&read[4], NULL, (void *)reader, (void *)&a[4]);
    pthread_create(&write[3], NULL, (void *)writer, (void *)&a[3]);
    pthread_create(&read[5], NULL, (void *)reader, (void *)&a[5]);
    pthread_create(&write[4], NULL, (void *)writer, (void *)&a[4]);
  

    for(int i = 0; i < 6; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }

    return 0;
    
}