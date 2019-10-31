#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

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
        pthread_join(threads[i-1], (void**)NULL);
    }

    gettimeofday(&end, NULL);
    // printf("%ld\n", (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    
    free(threads);

    printf("%d\n", getValue());
    
    return 0;
}


void *RLEzip(void *param){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    char *filename = (char *)param;
    int l = strlen(filename);
    FILE *fin = fopen(filename, "r");
    char *sub = malloc(l-2);
    substring(filename,0,l-3,sub);
    strcat(sub, "pzip");
    FILE *fout = fopen(sub, "w");
    free(sub);
    
    int count = 0;
    char curr;
    char c;
    if((curr = fgetc(fin))!=EOF){
        count+=1;
        while((c = fgetc(fin))!=EOF){
            if(curr==c){
                count+=1;
            }else{
                fwrite(&count, sizeof(int), 1, fout);
                fwrite(&curr, sizeof(char), 1, fout);
                curr = c;
                count = 1;
            }
        }
    }
    fwrite(&count, sizeof(int), 1, fout);
    fwrite(&curr, sizeof(char), 1, fout);

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
    if(l>4){
        sub = malloc(4);
        substring(s,l-3,3,sub);
        if(strcmp(sub,"txt")==0){
            return 1;
        }
    }
    return 0;
}