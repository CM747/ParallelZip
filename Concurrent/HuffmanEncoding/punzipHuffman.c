#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>


#define MAX_TREE_HT 100 

// structure for io files
typedef struct IOfiles{
    FILE *fin;
    FILE *fout;
	FILE *encode;
    int c;
}files;

struct MinHeapNode { 
	char data; 
	unsigned freq; 
	struct MinHeapNode *left, *right; 
}; 

struct MinHeap { 
	unsigned size; 
	unsigned capacity; 
	struct MinHeapNode** array; 
}; 

struct globalvars{
	int gsize;
	char *gchar;
	char **gb;
	int gi;
	char byte;
	int cnt;
	int length;
	char temp;
	int r;
	char buffer[100];
	int btop;
};


// Method Declarations
void *mainHuffmanDecode(void* );
struct MinHeapNode* newNode(char , unsigned );
struct MinHeap* createMinHeap(unsigned );
void swapMinHeapNode(struct MinHeapNode** , struct MinHeapNode** );
void minHeapify(struct MinHeap* , int);
int isSizeOne(struct MinHeap* );
struct MinHeapNode* extractMin(struct MinHeap* );
void insertMinHeap(struct MinHeap* , struct MinHeapNode* );
void buildMinHeap(struct MinHeap*);
void printArr(int * , int ,struct globalvars *);
int isLeaf(struct MinHeapNode* );
struct MinHeap* createAndBuildMinHeap(char *, int *, int );
struct MinHeapNode* buildHuffmanTree(char *, int *, int );
void printCodes(struct MinHeapNode* , int *, int, struct globalvars *);
void HuffmanCodes(char *, int *, int , struct globalvars *);
int readBit(FILE *, struct globalvars *);
int match();
char *substring(char *, int , int, char *);
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
            if(checkfileformat(argv[count])){
                pthread_create(&threads[i], NULL, mainHuffmanDecode, (void *)argv[count]);
            }else{
                printf("%s not in correct format. Could not convert\n", argv[count]);
            }
            count++;
        }

        int turn = 0;
        while(count<argc){
            pthread_join(threads[turn], (void**)NULL);
            if(checkfileformat(argv[count])){
                pthread_create(&threads[turn], NULL, mainHuffmanDecode, (void *)argv[count]);
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
                pthread_create(&threads[i-1], NULL, mainHuffmanDecode, (void *)argv[i]);
            }
            else{
                printf("%s not in correct format. Could not convert\n", argv[i]);
            }
        }

        for(int i=1; i<argc; i++){
            pthread_join(threads[i-1], (void**)NULL); //wait for threads to join
        }
        free(threads);
    }

    gettimeofday(&end, NULL); // print the time for the main to commplete
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));

    return 0;
}

// create the thread for decoding

void *mainHuffmanDecode(void* param) { 
    
	char *filename = (char *)param;
    int l = strlen(filename);
	FILE *fin = fopen(filename, "r");
	char *sub = malloc(l-3);
	substring(filename,0,l-4,sub); //change the filename to appropriate format
	strcat(sub, "fff");
	FILE *fout = fopen(sub, "w");
	free(sub);
	sub = malloc(l-3);
	substring(filename,0,l-4,sub);
	strcat(sub, "encd");
	FILE *encode = fopen(sub, "r");
	free(sub);

	struct globalvars gvars;
	gvars.gi = 0;
	gvars.cnt = 0;
	gvars.r = 0;
	gvars.btop = 0;

	int i,j;
	
	fread(&gvars.gsize,sizeof(int),1,encode); // get the value required to build huffman tree
	fread(&gvars.length,sizeof(int),1,encode);
	char arr[gvars.gsize];
	int freq[gvars.gsize];
	fread(&arr,sizeof(char),gvars.gsize,encode);
	fread(&freq,sizeof(int),gvars.gsize,encode);
	gvars.gchar=(char *)malloc(gvars.gsize*sizeof(char));
	gvars.gb=(char **)malloc(gvars.gsize*sizeof(char *));
	for(i=0;i<gvars.gsize;i++){
		gvars.gb[i]=(char *)malloc(100*sizeof(char));
	}

	HuffmanCodes(arr, freq, gvars.gsize, &gvars); //build the huffman code

	while(gvars.r<=gvars.length){
		gvars.buffer[gvars.btop]=(char)	(readBit(fin, &gvars)+48); //read the file bit by bit
		(gvars.btop)++;
		j=match(&gvars);
		if(j!=-1){
			fwrite(&gvars.gchar[j],sizeof(char),1,fout);
			gvars.btop=0;
		}
	}
	gvars.cnt=0;
	

	fclose(fin);
    fclose(fout);

	return NULL;
}

// function to a new node 
struct MinHeapNode* newNode(char data, unsigned freq) { 
	struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode)); 
	temp->left = temp->right = NULL; 
	temp->data = data; 
	temp->freq = freq; 
	return temp; 
} 

// function create a minheap
struct MinHeap* createMinHeap(unsigned capacity) { 
	struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap)); 
	minHeap->size = 0; 
	minHeap->capacity = capacity; 
	minHeap->array = (struct MinHeapNode**)malloc(minHeap-> capacity * sizeof(struct MinHeapNode*)); 
	return minHeap; 
} 

// function to swap two nodes
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) { 
	struct MinHeapNode* t = *a; 
	*a = *b; 
	*b = t; 
} 

// function to heapify
void minHeapify(struct MinHeap* minHeap, int idx) { 
	int smallest = idx; 
	int left = 2 * idx + 1; 
	int right = 2 * idx + 2; 
	if (left < minHeap->size && minHeap->array[left]-> freq < minHeap->array[smallest]->freq) 
		smallest = left; 
	if (right < minHeap->size && minHeap->array[right]-> freq < minHeap->array[smallest]->freq) 
		smallest = right; 
	if (smallest != idx) { 
		swapMinHeapNode(&minHeap->array[smallest],&minHeap->array[idx]); 
		minHeapify(minHeap, smallest); 
	} 
} 

// function to check the size
int isSizeOne(struct MinHeap* minHeap) { 
	return (minHeap->size == 1); 
} 

// function to extract the minimum value
struct MinHeapNode* extractMin(struct MinHeap* minHeap) { 

	struct MinHeapNode* temp = minHeap->array[0]; 
	minHeap->array[0] = minHeap->array[minHeap->size - 1]; 
	--minHeap->size; 
	minHeapify(minHeap, 0); 
	return temp; 
} 

// function to insert a new node to minheap
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode){ 
	++minHeap->size; 
	int i = minHeap->size - 1; 
	while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) { 
	minHeap->array[i] = minHeap->array[(i - 1) / 2]; 
		i = (i - 1) / 2; 
	} 
	minHeap->array[i] = minHeapNode; 
} 

// function to build a minheap
void buildMinHeap(struct MinHeap* minHeap) { 
	int n = minHeap->size - 1; 
	int i; 
	for (i = (n - 1) / 2; i >= 0; --i) 
		minHeapify(minHeap, i); 
} 

// function to print array of size n
void printArr(int arr[], int n, struct globalvars *gvars) { 
	int i;
	for (i = 0; i < n; ++i) {
		if(arr[i]==0){
			gvars->gb[gvars->gi][i]='0';
		}
		else{
			gvars->gb[gvars->gi][i]='1';	
		}
	}
	gvars->gb[gvars->gi][i]='\0';

} 
 
// function to check if the node is the leaf 
int isLeaf(struct MinHeapNode* root) { 
	return !(root->left) && !(root->right); 
} 

// Creates a min heap of capacity equal to size and inserts all character of data[] in min heap. Initially size of min heap is equal to capacity 
struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) { 
	struct MinHeap* minHeap = createMinHeap(size); 
	for (int i = 0; i < size; ++i) 
		minHeap->array[i] = newNode(data[i], freq[i]); 
	minHeap->size = size; 
	buildMinHeap(minHeap); 
	return minHeap; 
} 

// The main function that builds Huffman tree 
struct MinHeapNode* buildHuffmanTree(char data[], int freq[], int size) { 
	struct MinHeapNode *left, *right, *top; 
	struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size); 
	while (!isSizeOne(minHeap)) { 

		//Extract the two minimum 
		left = extractMin(minHeap); 
		right = extractMin(minHeap); 
		// Create a new internal node with frequency equal to the sum of the two nodes frequencies. 
		// Make the two extracted node as 
		// left and right children of this new node. 
		top = newNode('$', left->freq + right->freq); 
		top->left = left; 
		top->right = right; 
		insertMinHeap(minHeap, top); 
	}  
	return extractMin(minHeap); 
} 

// Prints huffman codes from the root of Huffman Tree. It uses arr[] to store codes 
void printCodes(struct MinHeapNode* root, int arr[], int top, struct globalvars *gvars) {  
	if (root->left) {
		arr[top] = 0; 
		printCodes(root->left, arr, top + 1, gvars); 
	}
	if (root->right) {
		arr[top] = 1; 
		printCodes(root->right, arr, top + 1, gvars); 
	} 
	if (isLeaf(root)) {
		// printf("%c: ",root->data);
		gvars->gchar[gvars->gi]= root->data;
		printArr(arr,top, gvars);
		gvars->gi=gvars->gi+1;
	} 
}

// The main function that builds a Huffman Tree and print codes by traversing the built Huffman Tree 
void HuffmanCodes(char data[], int freq[], int size, struct globalvars *gvars) {  
	struct MinHeapNode* root = buildHuffmanTree(data, freq, size); 
	int arr[MAX_TREE_HT], top = 0; 
	printCodes(root, arr, top, gvars); 
} 

// function to read a bit from the file
int readBit(FILE *f, struct globalvars *gvars){	
	char t;
	int k;
	t=1;
	gvars->cnt=gvars->cnt%8;
	if(gvars->cnt==0){
		fread(&gvars->temp,sizeof(char),1,f);
	}
	gvars->cnt++;
	gvars->r++;
	k=((gvars->temp & (t<<(8-(gvars->cnt))))>>(8-(gvars->cnt)));
	return(k);
}

int match(struct globalvars *gvars){
	int t=0;
	int i,j;
	for(i=0;i<gvars->gsize;i++){
		for(j=0;j<100;j++){
			if(gvars->buffer[j]!=gvars->gb[i][j]){
				break;
			}
			else{
				if(j==gvars->btop-1 && gvars->gb[i][j+1]=='\0'){
					t=1;
					return(i);
				}
			}
		}
	}
	return(t-1);
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