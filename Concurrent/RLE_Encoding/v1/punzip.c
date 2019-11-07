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


int main(int argc, char* argv[]){
    
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    if(argc==1){
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    // Allocate Space for argc threads and their parameters, input and output files
    pthread_t *threads = (pthread_t *)malloc((argc-1) * sizeof(pthread_t));
    files *arg = (files *)malloc((argc-1) * sizeof(files));
    FILE **fp = (FILE **)malloc((argc-1) * sizeof(FILE *));
    FILE **fout = (FILE **)malloc((argc-1) * sizeof(FILE *));
    int l;
    char* sub;
    for(int i=1; i<argc; i++){
        if(checkfileformat(argv[i])){
            l = strlen(argv[i]);
            fp[i-1] = fopen(argv[i], "r");
            
            sub = malloc(l-3);
            substring(argv[i],0,l-4,sub);
            strcat(sub, "fff");
            fout[i-1] = fopen(sub, "w");
            free(sub);

            arg[i-1].fin = fp[i-1];
            arg[i-1].fout = fout[i-1];
            arg[i-1].c = i;

            // RLEunzip((void *)&arg[i-1]);
            pthread_create(&threads[i-1], NULL, RLEunzip, (void *)&arg[i-1]);

            // fclose(fp[i-1]);
            // fclose(fout[i-1]);
        }
        else{
            printf("%s not in correct format. Could not convert\n", argv[i]);
        }
    }

    for(int i=1; i<argc; i++){
        pthread_join(threads[i-1], (void**)NULL);
        // printf("completed %d\n", i);
        fclose(fp[i-1]);
        fclose(fout[i-1]);
    }

    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));

    free(threads);
    free(arg);
        
    return 0;
}

void *RLEunzip(void *param){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    files *iofiles = (files *)param;
    printf("UnZipping: %d\n", iofiles->c);
    char curr;
    int count;
    char *buf;
    while(fread(&count, sizeof(int), 1, iofiles->fin)>0){
        if(fread(&curr, sizeof(char), 1, iofiles->fin)<=0){
            exit(1);
        }
        buf = (char *)malloc(count * sizeof(char));
        for(int j=1; j<=count; j++){
            buf[j-1] = curr;
        }
        fwrite(buf, sizeof(char), count, iofiles->fout);
    }
    
    gettimeofday(&end, NULL);
    printf("UnZipping Completed: %d, Time Taken: %ld microsec\n", iofiles->c, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));

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
