#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

// structure for io files
typedef struct IOfiles{
    FILE *fin;
    FILE *fout;
    int c;
}files;

// Method Declarations
void *RLEunzip(void *);
char *substring(char *, int, int, char *);
int checkfileformat(char *);

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmPeak:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

int main(int argc, char* argv[]){
    
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    // check if files are provided
    if(argc==1){
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }


    int N = 15;
    if(argc>N){
        pthread_t *threads = (pthread_t *)malloc(N * sizeof(pthread_t));
        int count = 1;
        for(int i=0; i<N; i++){
            if(checkfileformat(argv[count])){                                            // file in the correct format
                pthread_create(&threads[i], NULL, RLEunzip, (void *)argv[count]);        // create the threads
            }else{
                printf("%s not in correct format. Could not convert\n", argv[count]);
            }
            count++;
        }

        int turn = 0;
        while(count<argc){
            pthread_join(threads[turn], (void**)NULL);
            if(checkfileformat(argv[count])){
                pthread_create(&threads[turn], NULL, RLEunzip, (void *)argv[count]);
            }else{
                printf("%s not in correct format. Could not convert\n", argv[count]);
            }
            count++;
            turn = (turn+1)%N;
        }
        // wait for all thread to join
        for(int i=0; i<N; i++){
            pthread_join(threads[i], (void**)NULL);
        }
        free(threads);

    }else{
        // Allocate Space for argc threads and their parameters, input and output files
        pthread_t *threads = (pthread_t *)malloc((argc-1) * sizeof(pthread_t));
        for(int i=1; i<argc; i++){
            // Check file format and then proceed
            if(checkfileformat(argv[i])){
                pthread_create(&threads[i-1], NULL, RLEunzip, (void *)argv[i]);
            }
            else{
                printf("%s not in correct format. Could not convert\n", argv[i]);
            }
        }

        for(int i=1; i<argc; i++){
            pthread_join(threads[i-1], (void**)NULL);
        }
        free(threads);
    }

    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    // print the total time required for the to execute
    
    return 0;
}

// thread for unzipping
void *RLEunzip(void *param){
    
    char *filename = (char *)param;
    int l = strlen(filename);              // making file and naming it
    FILE *fin = fopen(filename, "r");
    char *sub = malloc(l-3);
    substring(filename,0,l-4,sub);
    strcat(sub, "pgm");
    FILE *fout = fopen(sub, "w");
    free(sub);

    char curr;
    int count, row, col;
    
    if(fread(&col, sizeof(int), 1, fin)==0){
        exit(1);
    }
    if(fread(&row, sizeof(int), 1, fin)==0){
        exit(1);
    }
    fprintf(fout, "P2\n%d %d\n255\n", col, row);

    // reading row wise
    int c, c1;
    for(int i=0; i<row; i++){
        c = 0;
        while(c<col){
            if(fread(&count, sizeof(int), 1, fin)==0){
                exit(1);
            }
            if(fread(&curr, sizeof(char), 1, fin)==0){
                exit(1);
            }
            c += count;
            c1 = (int)curr;                  // changing character to ascii value
            if(c1<0){                                       
                c1 = 256+c1;
            }
            for(int j=0; j<count; j++){
                fprintf(fout, "%d ", c1);
            }
        }
        fprintf(fout, "\n");
    }
    
    fclose(fin);
    fclose(fout);

    return NULL;
}

// function to make sunstring 
char *substring(char *string, int position, int length, char *pointer){
    int c;
    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }
    for (c = 0 ; c < length ; c++)
    {
        *(pointer+c) = *(string+position+c);
    }
    *(pointer+c) = '\0';
    return pointer;
}

// function to check fileformat
int checkfileformat(char *s){
    int l = strlen(s);
    char *sub;
    if(l>5){
        sub = malloc(5);
        substring(s,l-4,4,sub);
        if(strcmp(sub,"pzip")==0){
            return 1;
        }
    }
    return 0;
}