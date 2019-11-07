#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/sysinfo.h>


// Method Declarations
void *RLEzip(void *);
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


    int N = 15;       //at a time only 15 threads will create and next sets of file will create threads in fifo order, 
                    //in previous versions we were creating the threads equal to number of files
    if(argc>N){
        pthread_t *threads = (pthread_t *)malloc(N * sizeof(pthread_t));
        int count = 1;
        for(int i=0; i<N; i++){
            if(checkfileformat(argv[count])){
                pthread_create(&threads[i], NULL, RLEzip, (void *)argv[count]);
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
            pthread_join(threads[i-1], (void**)NULL);
        }
        free(threads);
    }

    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    
    return 0;
}


void *RLEzip(void *param){
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    char *filename = (char *)param;
    int l = strlen(filename);
    FILE *fin = fopen(filename, "r"); // we are reading the files parallely in each thread
    char *sub = malloc(l-2);
    substring(filename,0,l-3,sub); //slicing the filename --> removing txt extension from filename
    strcat(sub, "pzip");          // concating the pzip extension to filename
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
                fwrite(&count, sizeof(int), 1, fout);     //we will write 4 byte integer (Run Length)
                fwrite(&curr, sizeof(char), 1, fout);     //wrtie 1 byte character
                curr = c;                                 //thus a compressed file will consist of some number of 5 bytes
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