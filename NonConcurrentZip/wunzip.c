#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    
    if(argc==1){
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }
    FILE* fp;
    char curr;
    int count;
    for(int i=1; i<argc; i++){
        fp = fopen(argv[1], "r");
        while(fread(&count, sizeof(int), 1, fp)>0){
            fread(&curr, sizeof(char), 1, fp);
            for(int j=1; j<=count; j++){
                printf("%c", curr);
            }
        }
        fclose(fp);
    }
    return 0;
}