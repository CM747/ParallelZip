#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#define MAX_THREADS 15

typedef struct Node{
    char *filename;
    struct Node *next;
}node;



// Method Declarations
void *RLEzip(void *);
char *substring(char *, int, int, char *);
int checkfileformat(char *);

pthread_mutex_t lock;
node *files;


node *push(node *head, char *value){
    node *newnode = (node*)malloc(sizeof(node));
    newnode->filename = value;
    newnode->next = NULL;

    if(head==NULL){
        head = newnode;
    }else{
        newnode->next = head;
        head = newnode;
    }
    return head;
}

char *pop(node **head){
    pthread_mutex_lock(&lock);

    if(*head==NULL){
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    char *s = (*head)->filename;
    (*head) = (*head)->next;

    pthread_mutex_unlock(&lock);

    return s;
}


int main(int argc, char *argv[]){

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    // check if files are provided
    if(argc==1){
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }


    int N;
    if(argc>MAX_THREADS){
        N = MAX_THREADS;
    }else{
        N = argc-1;
    }

    for(int i=1; i<argc; i++){
        files = push(files, argv[i]);// adding all files to a list
    }

    if(pthread_mutex_init(&lock, NULL) != 0){
        exit(1);
    }

    pthread_t *threads = (pthread_t *)malloc(N * sizeof(pthread_t));
    for(int i=0; i<N; i++){
        pthread_create(&threads[i], NULL, RLEzip, (void *)pop(&files));
    }

    for(int i=0; i<N; i++){
        pthread_join(threads[i], (void**)NULL);
    }
    free(threads);


    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    

    return 0;
}


void *RLEzip(void *param){
    char *filename = (char *)param;
    int l;
    FILE *fin;
    FILE *fout;
    char *sub;

    while(filename){
        if(checkfileformat(filename)){
            l = strlen(filename);
            fin = fopen(filename, "r");  //we are reading the files parallely in each thread
            sub = malloc(l-2);
            substring(filename,0,l-3,sub);//slicing the filename --> removing txt extension from filename
            strcat(sub, "pzip");          // concating the pzip extension to filename
            fout = fopen(sub, "w");
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

            fclose(fin);
            fclose(fout);
        }else{
            printf("Incorrect file format: %s\n", filename);
        }
        filename = pop(&files);     //after zipping of one file it will pop the file and will start zipping next file waiting.
    }

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