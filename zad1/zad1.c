#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define WRITER_TURNS 10
#define READER_TURNS 10
#define READERS_COUNT 5

int readersCount = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_writer;
sem_t sem_reader;

long GetRandomTime(int maximumMiliseconds) {
    return (maximumMiliseconds * 1000.0 * (rand() / (RAND_MAX+1.0)));
}

// Writer thread function
int Writer(void* data) {
    sem_init(&sem_writer, 0, 1);

    int i;
    
    for (i = 0; i < WRITER_TURNS; i++) {
        sem_wait(&sem_writer);
        int result = pthread_mutex_lock(
                &mutex);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
	    // Write
	    printf("(W) Writer started writing...\n");
	    fflush(stdout);
	    usleep(GetRandomTime(800));
	    printf("(W) Writer FINISHED\n");
            
	    // Release ownership of the mutex object.
            result = pthread_mutex_unlock(&mutex);
            if (result != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
	    // Think, think, think, think
        sem_post(&sem_writer);
	    usleep(GetRandomTime(1000));
        }
    }

    return 0;
}

// Reader thread function
int Reader(void* data) {
    sem_init(&sem_reader, 0, 5);
    int i;
    int threadId = *(int*)data;
    int readCount = 0;
    
    for (i = 0; i < READER_TURNS; i++) {
        printf("(R) Reader %d is trying\n", threadId);
        sem_wait(&sem_reader);                      //wait
        readersCount++;
        printf("readersCount = %d \n", readersCount);
        if (readersCount == 1){
            sem_wait(&sem_writer);                  //writer is waiting          
            printf("(W) Writer is waiting\n");
        }
        sem_post(&sem_reader);                      //release

        int result = pthread_mutex_lock(
                &mutex);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
	    // Read
	    printf("(R) Reader %d started reading...\n", threadId);
        
	    fflush(stdout);
            // Read, read, read
	    usleep(GetRandomTime(200));
	    printf("(R) Reader %d FINISHED\n", threadId);
        //readersCount--;
	    readCount++;            
	    // Release ownership of the mutex object.
            result = pthread_mutex_unlock(&mutex);
            if (result != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
        sem_wait(&sem_reader);                      //wait
        readersCount--;
	    if(readersCount == 0)
            sem_post(&sem_writer);
        sem_post(&sem_reader);                      //release 
            usleep(GetRandomTime(800));
        }
    }

    printf("(R) Reader %d has read %d pages in total.\n", threadId, readCount);

    // Clean up the resources
    free(data);

    return 0;
}

int main(int argc, char* argv[])
{
    //sem_init(&sem_writer, 0, 1);
    //sem_init(&sem_reader, 0, 5);
    srand(100005);
    
    pthread_t writerThread;
    pthread_t readerThreads[READERS_COUNT];

    int i,rc;

    // Create the Writer thread
    rc = pthread_create(
            &writerThread,  // thread identifier
            NULL,           // thread attributes
            (void*) Writer, // thread function
            (void*) NULL);  // thread function argument

    if (rc != 0) 
    {
    	fprintf(stderr,"Couldn't create the writer thread");
        exit (-1);
    }

    // Create the Reader threads
    for (i = 0; i < READERS_COUNT; i++) {
	// Reader initialization - takes random amount of time
	usleep(GetRandomTime(1000));
	int* threadId=malloc(sizeof(int));
	*threadId=i;
	rc = pthread_create(
                &readerThreads[i], // thread identifier
                NULL,              // thread attributes
                (void*) Reader,    // thread function
                (void*) threadId); // thread function argument - in this example, it should be cleaned up by the Reader threa

        if (rc != 0)
        {
    	    fprintf(stderr,"Couldn't create the reader threads");
            exit (-1);
        }
    }

    // At this point, the readers and writers should perform their operations

    // Wait for the Readers
    for (i = 0; i < READERS_COUNT; i++) 
        pthread_join(readerThreads[i],NULL);

    // Wait for the Writer
    pthread_join(writerThread,NULL);

    return (0);
}
