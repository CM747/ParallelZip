#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

    if(argc==1){
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    FILE* fp;
    int count = 0;
    char curr;
    char c;
    for(int i=1; i<argc; i++){
        fp = fopen(argv[i], "r");
        if((curr = fgetc(fp))!=EOF){
            count+=1;
            while((c = fgetc(fp))!=EOF){
                if(curr==c){
                    count+=1;
                }else{
                    // if(count>sizeof(int)){
                    //     fwrite(&count, sizeof(int), 1, stdout);
                    //     fwrite(&curr, sizeof(char), 1, stdout);
                    // }
                    // else{
                    //     for(int j=1; j<=count; j++){
                    //         fwrite(&curr, sizeof(char), 1, stdout);
                    //     }
                    // }
                    fwrite(&count, sizeof(int), 1, stdout);
                    fwrite(&curr, sizeof(char), 1, stdout);
                    curr = c;
                    count = 1;
                }
            }
        }
        fclose(fp);
    }

    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&curr, sizeof(char), 1, stdout);

    return 0;
}
