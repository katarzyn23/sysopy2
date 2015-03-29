#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define WRITER_TURNS 10
#define READER_TURNS 10
#define READERS_COUNT 3
#define WRITER_COUNT 3

int readersCount = 0;
int writerTab[WRITER_COUNT];
int wroteCount = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
sem_t sem_writer;
sem_t sem_reader;

long GetRandomTime(int maximumMiliseconds) {
    return (maximumMiliseconds * 1000.0 * (rand() / (RAND_MAX+1.0)));
}

void ClearWriterTab(){
    int i=0;
    for(i; i<WRITER_COUNT; i++){
        writerTab[i]=0;
    }
}

int CheckWriterTab(){
    int i=0;
    int tmp=1;
    for(i; i<WRITER_COUNT; i++){
        if (writerTab[i] == 0)
            tmp=0;
    }
    return tmp;
}

void WriteWriterTab(){
    int i=0;
    printf("WriterTab: ");
    for(i; i<WRITER_COUNT; i++){
        printf(" %d", writerTab[i]);
    }
    printf("\n");
}
// Writer thread function
int Writer(void* data) {
   // sem_init(&sem_writer, 0, 1);

    int i;
    int threadIds = *(int*)data;
    
    
    for (i = 0; i < WRITER_TURNS; i++) {
        sem_wait(&sem_writer);
        printf("(W) Writer %d is trying to write...\n", threadIds);
        int result = pthread_mutex_lock(
                &mutex);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
	    // Write
	    printf("(W) Writer %d started writing...\n", threadIds);
        wroteCount++;
	    fflush(stdout);
	    usleep(GetRandomTime(800));
	    printf("(W) Writer %d finished writing\n", threadIds);
        writerTab[threadIds] = 1;
        WriteWriterTab();
        printf("----------------------------------%d\n", CheckWriterTab() );
	    // Release ownership of the mutex object.
        result = pthread_mutex_unlock(&mutex);
        if (result != 0) {
            fprintf(stderr, "Error occured during unlocking the mutex.\n");
            exit (-1);
        }
	    // Think, think, think, think
        /*
        if (CheckWriterTab() == 1){
            pthread_mutex_lock(&mutex);
            printf("CHUJ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            ClearWriterTab();
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
        }*/
        
        sem_post(&sem_writer);


        
	    usleep(GetRandomTime(1000));
        }
    }

    return 0;
}

// Reader thread function
int Reader(void* data) {
   // sem_init(&sem_reader, 0, 5);
    int i;
    int threadId = *(int*)data;
    int readCount = 0;
    
    for (i = 0; i < READER_TURNS; i++) {
        
        sem_wait(&sem_reader);                      //wait
        printf("(R) Reader %d is trying\n", threadId);
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
	    printf("(R) Reader %d finished \n", threadId);
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

// Writer thread function
int Critic(void* data) {
    int i = 0;
    int n = 1;
    while(1) {
        if(wroteCount == WRITER_TURNS*WRITER_COUNT){

            break;
         }
        while (CheckWriterTab() == 1) {

           // pthread_cond_wait(&cond, &mutex);
        
        pthread_mutex_lock(&mutex);
        ClearWriterTab();
       // pthread_cond_broadcast(&cond);
      //  sem_wait(&sem_reader);
      //  sem_wait(&sem_writer);

        printf(" (C) ------------------------------------CRITIC TIME!!!\n");
        fflush(stdout);

       // sem_post(&sem_writer);
       // sem_post(&sem_reader);
        
        pthread_mutex_unlock(&mutex);
        printf("                                       %d\n", wroteCount );
        
    }
    }
    free(data);
    return 0;
}


int main(int argc, char* argv[])
{
    sem_init(&sem_writer, 0, 1);
    sem_init(&sem_reader, 0, 1);
  //  srand(100005);
    srand(time(NULL));
    ClearWriterTab();
    pthread_t criticThread;
    pthread_t writerThreads[WRITER_COUNT];
    pthread_t readerThreads[READERS_COUNT];
    

    int i,rc;

    rc = pthread_create(
            &criticThread,  // thread identifier
            NULL,           // thread attributes
            (void*) Critic, // thread function
            (void*) NULL);  // thread function argument

    if (rc != 0)
    {
        fprintf(stderr,"Couldn't create the critic thread");
        exit (-1);
    }

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

  

    // At this point, the readers and writers (and critic) should perform their operations
    pthread_join(criticThread, NULL);
    // Wait for the Readers
    for (i = 0; i < READERS_COUNT; i++) 
        pthread_join(readerThreads[i],NULL);

    // Wait for the Writer

    for (i = 0; i < WRITER_COUNT; i++)
        pthread_join(writerThreads[i],NULL);

   // pthread_join(criticThread, NULL);


   
    return (0);
}
