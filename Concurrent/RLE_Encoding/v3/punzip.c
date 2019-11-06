#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

// structure for io files

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
// Method Declarations
void *RLEunzip(void *);
char *substring(char *, int, int, char *);
int checkfileformat(char *);


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
            if(checkfileformat(argv[count])){
                pthread_create(&threads[i], NULL, RLEunzip, (void *)argv[count]);
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
    // printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    // printf("%ld", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));

    printf("%d\n", getValue());
    
    return 0;
}

void *RLEunzip(void *param){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    char *filename = (char *)param;
    int l = strlen(filename);
    FILE *fin = fopen(filename, "r");
    char *sub = malloc(l-3);
    substring(filename,0,l-4,sub);
    strcat(sub, "fff");
    FILE *fout = fopen(sub, "w");
    free(sub);
    char curr;
    int count;
    char *buf;
    while(fread(&count, sizeof(int), 1, fin)>0){
        fread(&curr, sizeof(char), 1, fin);
        buf = (char *)malloc(count * sizeof(char));
        for(int j=1; j<=count; j++){
            buf[j-1] = curr;
        }
        fwrite(buf, sizeof(char), count, fout);
    }
    
    gettimeofday(&end, NULL);

    fclose(fin);
    fclose(fout);

    return NULL;
}

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
