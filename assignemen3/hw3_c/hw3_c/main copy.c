//
//  main.cpp
//  hw3
//
//  Created by Yixuan Tan on 3/14/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <pthread.h>

#define DEBUG 0

struct Info{
    pthread_t tid[3];
    int grandchildnum;
    int count[3];
    char filename[1024];
    pthread_mutex_t* low_mutex;
    
};

struct Files {
    pthread_t tid;
    struct Info info;
    int size;
    int *count;
    pthread_mutex_t* medium_mutex;
};

void resizeFiles(struct Files *files, int numOfFiles){
    if(numOfFiles > files->size) {
        int numOfMediumLocker = files->size;
        files->size = numOfFiles;
        int i = 0;
        pthread_mutex_lock(files->medium_mutex);
        for(; i < numOfMediumLocker; i++) {
            if(files->info.low_mutex != NULL) pthread_mutex_lock(files->info.low_mutex);
        }
        files = (struct Files*) realloc(files, 2 * files->size * sizeof(struct Files));
        pthread_mutex_unlock(files->medium_mutex);
        for(; i < numOfMediumLocker; i++) {
            pthread_mutex_unlock(files->info.low_mutex);
        }
    }
}

void* countAlphanumeric(void *arg) {
    struct Info * info = (struct Info *) arg;
    char* filename = info->filename;
    FILE * fp;
    fp = fopen(filename, "r");
    
    if (fp == NULL){
        fprintf(stderr, "ERROR: file %s does not exist\n", filename);
    }
    
    // directly from homework 1
    char c = 0;
    int alphanumericCounter = 0;
#ifdef DEBUG_MODE
    printf("ready to go \n");
    fflush(NULL);
#endif
    
    while((c = fgetc(fp)) != EOF){ // read one char at a time
#ifdef DEBUG_MODE
        //        printf("%c\n", c);
#endif
        if(isalnum(c)) {
            alphanumericCounter++;
        }
    }
    fclose(fp);
#ifdef DEBUG_MODE
    printf("file closed !! \n");
#endif
    
    pthread_mutex_lock( info->low_mutex);
    info->count[0] += alphanumericCounter;
    pthread_mutex_unlock( info->low_mutex );
    printf( "THREAD %d: Added alphanumeric count of %d to totals (then exiting)\n", (int)info->tid[0], alphanumericCounter);
    fflush(NULL);
    /* dynamically allocate space to hold a return value */
    pthread_t * x = (pthread_t *)malloc( sizeof( pthread_t ) );
    
    /* return value is simply the thread ID */
    *x = pthread_self();
    pthread_exit( (void *)x );   /* terminate the thread, returning x to
                          a waiting pthread_join() call */
    return NULL;
}

void* countWhitespace(void *arg) {
    struct Info * info = (struct Info *) arg;
    char* filename = info->filename;
    FILE * fp;
    fp = fopen(filename, "r");
    
    if (fp == NULL){
        fprintf(stderr, "ERROR: file %s does not exist\n", filename);
    }

    // directly from homework 1
    char c = 0;
    int spaceCounter = 0;
    while((c = fgetc(fp)) != EOF){ // read one char at a time
        if(isspace(c)) {
            spaceCounter++;
        }
    }
    fclose(fp);
    
    pthread_mutex_lock( info->low_mutex );
    info->count[1] += spaceCounter;
    pthread_mutex_unlock( info->low_mutex );
    printf( "THREAD %d: Added whitespace count of %d to totals (then exiting)\n", (int)info->tid[1], spaceCounter);
    fflush(NULL);
    /* dynamically allocate space to hold a return value */
    pthread_t * x = (pthread_t *)malloc( sizeof( pthread_t ) );
    
    /* return value is simply the thread ID */
    *x = pthread_self();
    pthread_exit( (void *)x );   /* terminate the thread, returning x to
                                  a waiting pthread_join() call */
    return NULL;
}

void* countOther(void *arg) {
    struct Info * info = (struct Info *) arg;
    char* filename = info->filename;
    FILE * fp;
    fp = fopen(filename, "r");
    
    if (fp == NULL){
        fprintf(stderr, "ERROR: file %s does not exist\n", filename);
    }

    // directly from homework 1
    char c = 0;
    int otherCounter = 0;
    while((c = fgetc(fp)) != EOF){ // read one char at a time
        if(!isspace(c) && !isalnum(c)) {
            otherCounter++;
        }
    }
    fclose(fp);

    pthread_mutex_lock( info->low_mutex );
    info->count[2] += otherCounter;
    pthread_mutex_unlock( info->low_mutex );
    printf( "THREAD %d: Added other count of %d to totals (then exiting)\n", (int)info->tid[2], otherCounter);
    fflush(NULL);
    /* dynamically allocate space to hold a return value */
    pthread_t * x = (pthread_t *)malloc( sizeof( pthread_t ) );
    
    /* return value is simply the thread ID */
    *x = pthread_self();
    pthread_exit( (void *)x );   /* terminate the thread, returning x to
                                  a waiting pthread_join() call */
    return NULL;
}

void* (*lowlevelwork[])(void*) = {countAlphanumeric, countWhitespace, countOther};

void *mediumlevelwork(void* arg) {
    struct Files * fileinfo = (struct Files *) arg;
    printf("filename %s\n", fileinfo->info.filename);
    fflush(NULL);
    if (access(fileinfo->info.filename,F_OK) == -1){
        fprintf(stderr, "ERROR: file %s does not exist\n", fileinfo->info.filename);
    } else {
        printf( "THREAD %d: Processing %s (created three child threads)\n", (int)fileinfo->tid, fileinfo->info.filename);
        int rc[3];
         // count of alnum, space and others
        
        // initialize
        int i = 0;
        for(; i < 3; i++) {
            fileinfo->info.count[0] = 0;
        }
        pthread_mutex_t low_mutex = PTHREAD_MUTEX_INITIALIZER;
        fileinfo->info.low_mutex = &low_mutex;
        
        // create an array of function pointer
        // http://stackoverflow.com/questions/252748/how-can-i-use-an-array-of-function-pointers
        
        int grandchild = 0;
        for(; grandchild < 3; grandchild++){
            rc[grandchild] = pthread_create(&fileinfo->info.tid[grandchild], NULL, lowlevelwork[grandchild], (void*)&fileinfo->info);
            if ( rc[grandchild] != 0 ) {
                fprintf( stderr, "MAIN: Could not create child thread (%d)\n", rc[grandchild] );
            }
        }
        //printf("THREAD %d: Created child thread for %s\n", (int)fileinfo->tid, fileinfo->info.filename);
        //fflush(NULL);
        
        grandchild = 0;
        for(; grandchild < 3; grandchild++){
            unsigned int * x;
            pthread_join(fileinfo->info.tid[grandchild], (void**)&x);
            if(DEBUG) printf( "MEDIUM: Joined a child thread that returned %u.\n", *x );
            fflush(NULL);
            free(x);
        }
        
        pthread_mutex_lock( fileinfo->medium_mutex );
        i = 0;
        for(; i < 3; i++) fileinfo->count[i] += fileinfo->info.count[i];
        pthread_mutex_unlock( fileinfo->medium_mutex );
        printf("THREAD %d: File %s contains %d alnum, %d space, and %d other characters\n", (int)fileinfo->tid, fileinfo->info.filename, fileinfo->info.count[0], fileinfo->info.count[1], fileinfo->info.count[2]);
        fflush(NULL);

    }
    /* dynamically allocate space to hold a return value */
    pthread_t * ret = (pthread_t *)malloc( sizeof( pthread_t ) );
    
    /* return value is simply the thread ID */
    *ret = pthread_self();
    pthread_exit( (void *)ret );   /* terminate the thread, returning x to
                                  a waiting pthread_join() call */
    return NULL;
}

struct TopLevelPack{
    struct Files* files;
    int countTotal[3];
    int argc;
    const char** argv;
    pthread_t tid;
};


void *toplevelwork(void *arg){
    struct TopLevelPack* topLevelPack = (struct TopLevelPack*) arg;
    struct Files** files = &topLevelPack->files;
    int *countTotal = topLevelPack->countTotal;
    countTotal[0] = 0; countTotal[1] = 0; countTotal[2] = 0;
    int argc = topLevelPack->argc;
    const char **argv = topLevelPack->argv;
    
    int fileCount = 0;
    char filename[1024]; // buffer for file name
    pthread_mutex_t medium_mutex = PTHREAD_MUTEX_INITIALIZER;
    *files = (struct Files*) malloc(sizeof(struct Files));
    for(; fileCount < argc - 1; fileCount++) {
        memset(filename, '\0', 1024);
        strcpy(filename, argv[fileCount + 1]); //
        printf("filename is %s\n", filename);
        fflush(NULL);
        if(fileCount + 1 > 1) resizeFiles(*files, fileCount + 1);
        
        // medium level child, need to create 3 grand children
        (*files)[fileCount].count = countTotal;
        (*files)[fileCount].medium_mutex = &medium_mutex;
        strcpy((*files)[fileCount].info.filename, filename);
        printf("filename is is is is %s", (*files)[fileCount].info.filename);

        pthread_create(&((*files)[fileCount].tid), NULL, mediumlevelwork, (void *)&files[fileCount]);
        printf("THREAD %d: Created child thread for %s\n", (int)topLevelPack->tid, filename);
        fflush(NULL);
    }
    
    for(fileCount = 0; fileCount < argc - 1; fileCount++) {
        unsigned int * x;
        pthread_join((*files)[fileCount].tid, (void**)&x);
        if(DEBUG) printf( "TOP: Joined a child thread that returned %u.\n", *x );
        fflush(NULL);
        free(x);
    }
    return NULL;
}


int main(int argc, const char * argv[]) {
    // insert code here...
    if(argc < 2) {
        fprintf(stderr, "ERROR: no input file\n");
        return EXIT_SUCCESS; // no file to read
    }
    printf("MAIN THREAD: Program started (top-level thread)\n");
    fflush(NULL);
    
    struct TopLevelPack topLevelPackp;
    topLevelPackp.argc = argc;
    topLevelPackp.argv = argv;
    
    
    pthread_create(&topLevelPackp.tid, NULL, toplevelwork, (void*)&topLevelPackp);
    unsigned int * x;
    pthread_join(topLevelPackp.tid, (void**)&x);
    if(DEBUG) printf( "MAIN: Joined a child thread that returned %u.\n", *x );
    fflush(NULL);
    free(x);

    printf("MAIN THREAD: All files contain %d alnum, %d space, and %d other characters\n", topLevelPackp.countTotal[0], topLevelPackp.countTotal[1], topLevelPackp.countTotal[2]);
    fflush(NULL);
    printf("MAIN THREAD: Program ended (top-level thread)");
    fflush(NULL);
    
    free(topLevelPackp.files);
    return EXIT_SUCCESS;
}
