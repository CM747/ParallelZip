#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <ctype.h>


// Method Declarations
void *RLEzip(void *);
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
            if(checkfileformat(argv[count])){                                    // file in correct format
                pthread_create(&threads[i], NULL, RLEzip, (void *)argv[count]);  // create the thread 
            }else{
                printf("%s not in correct format. Could not convert\n", argv[count]);
            }
            count++;
        }

        int turn = 0;
        while(count<argc){
            pthread_join(threads[turn], (void**)NULL);
            if(checkfileformat(argv[count])){
                pthread_create(&threads[turn], NULL, RLEzip, (void *)argv[count]);
            }else{
                printf("%s not in correct format. Could not convert\n", argv[count]);
            }
            count++;
            turn = (turn+1)%N;
        }
        // wait for threads to join
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
                pthread_create(&threads[i-1], NULL, RLEzip, (void *)argv[i]);
            }
            else{
                printf("%s not in correct format. Could not convert\n", argv[i]);
            }
        }

        for(int i=1; i<argc; i++){
            pthread_join(threads[i-1], (void**)NULL); // wait for threads to join
        }
        free(threads);
    }

    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    // prints the time required for the main to finish
    
    return 0;
}

// thread for zipping file
void *RLEzip(void *param){
    // reading the files and change the format
    char *filename = (char *)param;
    int l = strlen(filename);
    FILE *fin = fopen(filename, "r");
    char *sub = malloc(l-2);
    substring(filename,0,l-3,sub);
    strcat(sub, "pzip");
    FILE *fout = fopen(sub, "w");
    free(sub);
    int count = 0;
    int curr;
    int c;
    char c1;
    // read the version, number of rows, columns and maximum value of gray
    char version[3];
    int col, row, max_gray;
    if(fgets(version, sizeof(version), fin)==NULL){
        exit(1);
    }
    if(fscanf(fin, "%d ", &col)==EOF){
        exit(1);
    }
    if(fscanf(fin, "%d", &row)==EOF){
        exit(1);
    }
    if(fscanf(fin, "%d", &max_gray)==EOF){
        exit(1);
    }
    // write the number of columns and rows in the file
    fwrite(&col, sizeof(int), 1, fout);
    fwrite(&row, sizeof(int), 1, fout);

    // do RLE zip on the image
    for (int i=0; i<row; i++) {
        if(fscanf(fin, "%d ", &curr)==EOF){
            exit(1);
        }
        count = 1;
        for (int j=1; j<col; j++) {
            if(fscanf(fin, "%d ", &c)==EOF){
                exit(1);
            }
            if(c==curr){                                // value equal to the previous
                count += 1;
            }else{                                      // value equal to the previous
                c1 = (char)curr;
                fwrite(&count, sizeof(int), 1, fout);
                fwrite(&c1, sizeof(char), 1, fout);
                curr = c;
                count = 1;
            }
        }
        c1 = (char)curr;
        fwrite(&count, sizeof(int), 1, fout);
        fwrite(&curr, sizeof(char), 1, fout);
    }


    fclose(fin);
    fclose(fout);

    return NULL;
}

// function to create a substring
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

// function to create file format
int checkfileformat(char *s){
    int l = strlen(s);
    char *sub;
    if(l>4){
        sub = malloc(4);
        substring(s,l-3,3,sub);
        if(strcmp(sub,"pgm")==0){
            return 1;
        }
    }
    return 0;
}