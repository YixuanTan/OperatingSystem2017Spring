//
//  main.c
//  hw2
//
//  Created by Yixuan Tan on 2/9/17.
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

/*
 loop through command-line arguments (i.e., input files)
 {
 create pipe for next child process
 pid = fork();
 
 if ( pid == 0 )
 {
 child( ... );
 exit( EXIT_SUCCESS );
 }
 // no need for an else here, since we want parent to continue with the loop
}

loop again:
{
    read() from child pipe (or detect error on read() via return value of zero)
    // note that read() is a blocking call -- be sure to check its return value
    process data read from the child pipe
    wait()
}

*/
#ifndef DEBUG_MODE
//#define DEBUG_MODE
#endif

struct Pipes {
    int numChild;
    int* pArray; // with size 2*numChild;
};

void resizePipes(struct Pipes *pipeInfo, int numChildNew){
    if(numChildNew > pipeInfo->numChild) {
        pipeInfo->numChild = numChildNew;
        pipeInfo->pArray = (int*) realloc(pipeInfo->pArray, 2 * pipeInfo->numChild * sizeof(int));
    }
}

void countAlphanumeric(int pipesSecondFork[][2], char filename[]) {
    // use p[0] read ; p[1] write;
    FILE * fp;
#ifdef DEBUG_MODE
    printf("countAlphanumeric PID %d filename : %s\n", getpid(), filename);
#endif

    fp = fopen(filename, "r");
#ifdef DEBUG_MODE
    printf("opened !! \n");
    fflush(NULL);
#endif

    if (fp == NULL){
        perror("Error");
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

    if(write(pipesSecondFork[0][1], &alphanumericCounter, sizeof(alphanumericCounter)) < 0) perror("ERROR");
    printf( "PID %d: Sent alphanumeric count of %d to parent (then exiting)\n", getpid(), alphanumericCounter);
    fflush(NULL);
}

void countWhitespace(int pipesSecondFork[][2], char filename[]){
    // use p[1][0] read;  p[1][1] write;
    FILE * fp;
#ifdef DEBUG_MODE
    printf("countWhitespace PID %d filename : %s\n", getpid(), filename);
#endif

    fp = fopen(filename, "r");
    if (fp == NULL){
        perror("Error");
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
    if(write(pipesSecondFork[1][1], &spaceCounter, sizeof(spaceCounter)) < 0) perror("ERROR");
    printf( "PID %d: Sent whitespace count of %d to parent (then exiting)\n", getpid(), spaceCounter);
    fflush(NULL);
}

void countOther(int pipesSecondFork[][2], char filename[]){
    // use p[2][0] read; p[2][1] write;
    FILE * fp;
#ifdef DEBUG_MODE
    printf("countOther PID %d filename : %s\n", getpid(), filename);
#endif

    fp = fopen(filename, "r");
    if (fp == NULL){
        perror("Error");
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
    if(write(pipesSecondFork[2][1], &otherCounter, sizeof(otherCounter)) < 0) perror("ERROR");
    printf( "PID %d: Sent other count of %d to parent (then exiting)\n", getpid(), otherCounter);
    fflush(NULL);
}


int main(int argc, const char * argv[]) {
    // insert code here...
    if(argc < 2) {
        fprintf(stderr, "ERROR: no input file\n");
        return EXIT_SUCCESS; // no file to read
    }
    printf("PID %d: Program started (top-level process)\n", getpid());
    fflush(NULL);

    struct Pipes pipes = {1, (int*)calloc(2, sizeof(int))};
    int indexInpArray = 0;
    int fileCount = 0;
    char filename[1024]; // buffer for file name
    
    for(; fileCount < argc - 1; fileCount++) {
        resizePipes(&pipes, fileCount + 1); // fileCount + 1 is the number of child  after this iteration.
        memset(filename, '\0', 1024);
        strcpy(filename, argv[fileCount + 1]); // next child will share this filename with parent, while parent will continous update it.
        
        int p[2];
        int rc = pipe( p );
        if ( rc == -1 )
        {
            perror( "ERROR" );
        }
        pipes.pArray[indexInpArray] = p[0];
        pipes.pArray[indexInpArray + 1] = p[1];

        
        int pidFristFork = fork();

        if(pidFristFork == 0) {
            // medium level child, need to create 3 grand children
            // if the file does not exist?
            int package[3];
            memset(package, 0, sizeof(package));
            
            if (access(filename,F_OK) == -1){
                perror("Error");
            } else {
                printf("PID %d: Created child process for %s\n", getpid(), filename);
                fflush(NULL);
                int pipesSecondFork[3][2]; // two(one for read, the other one for write) for each of the 3 children.
                int grandChildNumbering = 0;
                printf("PID %d: Processing %s (created three child processes)\n", getpid(), filename);
                fflush(NULL);
                for(; grandChildNumbering < 3; grandChildNumbering++) {
                    
                    int rc = pipe( pipesSecondFork[grandChildNumbering] );
                    if ( rc == -1 )
                    {
                        perror( "ERROR" );
                    }
                    
                    int pidSecondFork = fork();
                    
                    if(pidSecondFork == 0) { // grandChild
                        
                        switch (grandChildNumbering) {
                            case 0: countAlphanumeric(pipesSecondFork, filename); break;// count alphanumeric
                            case 1: countWhitespace(pipesSecondFork, filename); break;// count whitespace
                            case 2: countOther(pipesSecondFork, filename); break; // count other
                            default: fprintf(stderr, "ERROR: create grand child is in error\n"); fflush(NULL); break;                        }
                        exit(EXIT_SUCCESS); // terminate grand child process
                    }
                    
                }
                
                int status = 0;
                int alphanumericCounter = 0;
                int spaceCounter = 0;
                int otherCounter = 0;
                for(grandChildNumbering = 0; grandChildNumbering < 3; grandChildNumbering++) {
                    // receive counter
                    switch (grandChildNumbering) {
                        case 0: if(read(pipesSecondFork[0][0], &alphanumericCounter, sizeof(alphanumericCounter)) < 0) perror("ERROR"); break;// count alphanumeric
                        case 1: if(read(pipesSecondFork[1][0], &spaceCounter, sizeof(spaceCounter)) < 0) perror("ERROR"); break;// count whitespace
                        case 2: if(read(pipesSecondFork[2][0], &otherCounter, sizeof(otherCounter)) < 0) perror("ERROR"); break; // count other
                        default: fprintf(stderr, "ERROR: create grand child is in error\n");fflush(NULL); break;
                    }
                    
                    wait(&status);
                    if (WIFSIGNALED( status )) {
                        perror("ERROR");
                    }
                }
                printf("PID %d: File %s contains %d alnum, %d space, and %d other characters\n", getpid(), filename, alphanumericCounter, spaceCounter, otherCounter);
                fflush(NULL);
                package[0] = alphanumericCounter;
                package[1] = spaceCounter;
                package[2] = otherCounter;
            }
            

            // send to parent
            if(write(pipes.pArray[indexInpArray + 1], package, sizeof(package)) < 0) { // indexInpArray is the read channel, indexInpArray + 1 is the write channel.
                perror("ERROR");
            }
            printf("PID %d: Sent %s counts to parent (then exiting)\n", getpid(), filename);
            fflush(NULL);
            exit(EXIT_SUCCESS); // terminate medium child process
        }
        
        indexInpArray += 2; // one for read, the other one for write
        
    }
    
    int alphanumericCounter = 0;
    int spaceCounter = 0;
    int otherCounter = 0;
    for(fileCount = 0; fileCount < argc - 1; fileCount++) {
        int indexInpArray = 2 * fileCount;
        int package[3];
        if(read(pipes.pArray[indexInpArray], package, sizeof(package)) < 0) {
            perror("ERROR");
        }
        alphanumericCounter += package[0];
        spaceCounter += package[1];
        otherCounter += package[2];
        
        int status = 0;
        wait(&status);
        if (WIFSIGNALED( status )) {
            perror("ERROR");
        }
    }
    printf("PID %d: All files contain %d alnum, %d space, and %d other characters\n", getpid(), alphanumericCounter, spaceCounter, otherCounter);
    fflush(NULL);
    printf("PID %d: Program ended (top-level process)", getpid());
    fflush(NULL);
    
    free(pipes.pArray);
    return EXIT_SUCCESS;
}
