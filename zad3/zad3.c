#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define WRITER_TURNS 10
#define READER_TURNS 10
#define READERS_COUNT 5
#define WRITER_COUNT 5
#define BUF_SIZE 3

//int buf = 0;
//int readersCount = 0;
pthread_mutex_t mutex_bufor[BUF_SIZE]= PTHREAD_MUTEX_INITIALIZER;
/*pthread_cond_t cond_full   = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty   = PTHREAD_COND_INITIALIZER;
//sem_t sem_writer;
//sem_t sem_reader;
*/
long GetRandomTime(int maximumMiliseconds) {
    return (maximumMiliseconds * 1000.0 * (rand() / (RAND_MAX+1.0)));
}

// Writer thread function
int Writer(void* data) {
    //sem_init(&sem_writer, 0, 5);
    int i;
    int threadIds = *(int*)data;
    int wroteCount = 0;
    for (i = 0; i < WRITER_TURNS; i++) {
        int buf_num = rand() % BUF_SIZE;                           
        printf("(W) Writer %d is trying to write using bufor %d\n", threadIds, buf_num);
        int result = pthread_mutex_lock(
                &mutex_bufor[buf_num]);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
        // Write
            printf("(W) Writer %d started writing using bufor %d\n", threadIds, buf_num);
            fflush(stdout);
            usleep(GetRandomTime(800));
            printf("(W) Writer %d finished writing using bufor %d\n", threadIds, buf_num);
            wroteCount++;
            result = pthread_mutex_unlock(&mutex_bufor[buf_num]);
            if (result != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
        usleep(GetRandomTime(1000));
        }
    }
    printf("(W) Writer %d has written %d pages in total.\n", threadIds, wroteCount);
    free(data);
    return 0;
}

// Reader thread function
int Reader(void* data) {
  //  sem_init(&sem_reader, 0, 5);
    int i;
    int threadId = *(int*)data;
    int readCount = 0;
    
    for (i = 0; i < READER_TURNS; i++) {
        int buf_num = rand()%BUF_SIZE;
        printf("(R) Reader %d is trying to read using bufor %d\n", threadId, buf_num);
        int result = pthread_mutex_lock(
            &mutex_bufor[buf_num]);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
            printf("(R) Reader %d started reading using bufor %d\n", threadId, buf_num);
            fflush(stdout);
            // Read, read, read
            usleep(GetRandomTime(200));
            printf("(R) Reader %d finished using bufor %d\n", threadId, buf_num);
            readCount++;            
            result = pthread_mutex_unlock(&mutex_bufor[buf_num]);
            if (result != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
     /*     sem_wait(&sem_reader);                      //wait
            readersCount--;
            if(readersCount == 0)
                 sem_post(&sem_writer);
            sem_post(&sem_reader);                      //release 
            usleep(GetRandomTime(800));*/
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
    
    pthread_t writerThreads[WRITER_COUNT];
    pthread_t readerThreads[READERS_COUNT];

    int i,rc;

    // Create the Writer thread
    for(i = 0; i < WRITER_COUNT; i++)   {
    usleep(GetRandomTime(1000));
    int* threadId=malloc(sizeof(int));
    *threadId=i;
    rc = pthread_create(
            &writerThreads[i],  // thread identifier
            NULL,           // thread attributes
            (void*) Writer, // thread function
            (void*) threadId);  // thread function argument

        if (rc != 0) 
        {
            fprintf(stderr,"Couldn't create the writer thread");
            exit (-1);
        }
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

    for (i = 0; i < WRITER_COUNT; i++)
        pthread_join(writerThreads[i],NULL);

    return (0);
}
