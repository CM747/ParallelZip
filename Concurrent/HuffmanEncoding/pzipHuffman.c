#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_TREE_HT 100 



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

// A Huffman tree node 
struct MinHeapNode { 
	// One of the input characters 
	char data; 
	// Frequency of the character 
	unsigned freq; 
	// Left and right child of this node 
	struct MinHeapNode *left, *right; 
}; 

// A Min Heap: Collection of min-heap (or Huffman tree) nodes 
struct MinHeap { 
	// Current size of min heap 
	unsigned size; 
	// capacity of min heap 
	unsigned capacity; 
	// Array of minheap node pointers 
	struct MinHeapNode** array; 
};

struct globalvars
{
	int gsize;
	char *gchar;
	char **gb;
	int gi;
	char byte;
	int cnt;
	int length;
};

// Method Declarations
void *mainHuffman(void* );
char *substring(char *, int, int, char *);
int checkfileformat(char *);
struct MinHeapNode* newNode(char , unsigned );
struct MinHeap* createMinHeap(unsigned );
void swapMinHeapNode(struct MinHeapNode**, struct MinHeapNode**);
void minHeapify(struct MinHeap* , int );
int isSizeOne(struct MinHeap* );
struct MinHeapNode* extractMin(struct MinHeap*);
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* );
void buildMinHeap(struct MinHeap* );
void printArr(int *, int , struct globalvars *);
int isLeaf(struct MinHeapNode* );
struct MinHeap* createAndBuildMinHeap(char *, int *, int );
struct MinHeapNode* buildHuffmanTree(char *, int *, int ); 
void printCodes(struct MinHeapNode* , int *, int, struct globalvars *);
void HuffmanCodes(char *, int *, int , struct globalvars *);
void writeBit(int , FILE *, struct globalvars *);


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
                pthread_create(&threads[i], NULL, mainHuffman, (void *)argv[count]);
            }else{
                printf("%s not in correct format. Could not convert\n", argv[count]);
            }
            count++;
        }

        int turn = 0;
        while(count<argc){
            pthread_join(threads[turn], (void**)NULL);
            if(checkfileformat(argv[count])){
                pthread_create(&threads[turn], NULL, mainHuffman, (void *)argv[count]);
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
                pthread_create(&threads[i-1], NULL, mainHuffman, (void *)argv[i]); //file in correct format
            }
            else{
                printf("%s not in correct format. Could not convert\n", argv[i]); //file in wrong format
            }
        }

        for(int i=1; i<argc; i++){
            pthread_join(threads[i-1], (void**)NULL); 
        }
        free(threads); //free memory allocated for threads
    }

    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    // printing the total time required for the main to execute
    
    return 0;
}


void *mainHuffman(void* param) //create the thread for encoding the file 
{

    char *filename = (char *)param;
    int l = strlen(filename);
    FILE *fin = fopen(filename, "r");
    char *sub = malloc(l-2);  //naming the file to appropiate format
    substring(filename,0,l-3,sub);
    strcat(sub, "pzip"); 
    FILE *fout = fopen(sub, "w");
    free(sub);
    sub = malloc(l-2);
    substring(filename,0,l-3,sub);
    strcat(sub, "encd");
    FILE *encode = fopen(sub, "w");
    free(sub);


	struct globalvars gvars;
	gvars.gi = 0;

	char m;
	int b[128],i,t=0,k,h; //making a hash for each of the ascii character
	for(i=0;i<128;i++){
		b[i]=0;
	}
	

	// runnning first time for finding the number of characters and count of each ascii character 
	m=getc(fin);
	while(m!=EOF){
		b[(int) m]+=1;
		if(b[(int) m]==1){
			t++;
		}
		m=getc(fin);
	}
	gvars.gsize=t; //number of distinct characters in the file
	gvars.gchar=(char *)malloc(t*sizeof(char));  // array for the all the character in the file
	gvars.gb=(char **)malloc(t*sizeof(char *));  // 2-d array with encoding of each characterin the row
	for(i=0;i<t;i++){
		gvars.gb[i]=(char *)malloc(100*sizeof(char));
	}
	char arr[t];
	int freq[t];
	t=0;
	for(i=0;i<128;i++){
		if(b[i]>0){
			freq[t]=b[i];
			arr[t]=(char) i;
			t=t+1;
		}
	}
	int size = sizeof(arr) / sizeof(arr[0]); // number of elements in the arr
	HuffmanCodes(arr, freq, size, &gvars); // build the huffman coding
	rewind(fin);

	//read the file from starting to store the encoding

	m=getc(fin);
	while(m!=EOF){
		for(k=0;k<gvars.gsize;k++){
			if(gvars.gchar[k]==m){
				h=0;
				while(gvars.gb[k][h] !='\0'){
					writeBit(((int) gvars.gb[k][h] -48), fout, &gvars); //write bit in in the file
					h++;
				}
				break;
			}
		}
		m=getc(fin);
	}
	if(gvars.cnt!=0){
		fwrite(&gvars.byte,sizeof(char),1, fout);
	}
	
	fwrite(&gvars.gsize,sizeof(int),1,encode); // write the number of elements, length of file encoded by huffman encoding
	fwrite(&gvars.length,sizeof(int),1,encode);
	fwrite(&arr,sizeof(char),gvars.gsize,encode);
	fwrite(&freq,sizeof(int),gvars.gsize,encode);

	fclose(fin);
    fclose(fout);

	return NULL;
}


// A utility function allocate a new min heap node with gvars.given character and frequency of the character 
struct MinHeapNode* newNode(char data, unsigned freq) 
{ 
	struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode)); 
	temp->left = temp->right = NULL; 
	temp->data = data; 
	temp->freq = freq; 
	return temp;
} 

// A utility function to create a min heap of gvars.given capacity 
struct MinHeap* createMinHeap(unsigned capacity) 
{ 
	struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap)); 
	// current size is 0 
	minHeap->size = 0; 
	minHeap->capacity = capacity; 
	minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity * sizeof(struct MinHeapNode*)); 
	return minHeap; 
}

// A utility function to swap two min heap nodes 
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b)
{ 
	struct MinHeapNode* t = *a; 
	*a = *b; 
	*b = t; 
} 

// The standard minHeapify function. 
void minHeapify(struct MinHeap* minHeap, int idx) 
{ 
	int smallest = idx; 
	int left = 2 * idx + 1; 
	int right = 2 * idx + 2;

	if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq) 
		smallest = left; 
	if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq) 
		smallest = right; 
	if (smallest != idx) { 
		swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]); 
		minHeapify(minHeap, smallest);
	}
}

// A utility function to check if size of heap is 1 or not 
int isSizeOne(struct MinHeap* minHeap) 
{
	return (minHeap->size == 1); 
} 

// A standard function to extract minimum value node from heap 
struct MinHeapNode* extractMin(struct MinHeap* minHeap) 
{ 
	struct MinHeapNode* temp = minHeap->array[0]; 
	minHeap->array[0] = minHeap->array[minHeap->size - 1]; 

	--minHeap->size; 
	minHeapify(minHeap, 0); 

	return temp; 
}

// A utility function to insert a new node to Min Heap 
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode)
{ 
	++minHeap->size; 
	int i = minHeap->size - 1; 

	while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) { 
		minHeap->array[i] = minHeap->array[(i - 1) / 2]; 
		i = (i - 1) / 2; 
	} 

	minHeap->array[i] = minHeapNode; 
} 

// A standard function to build min heap 
void buildMinHeap(struct MinHeap* minHeap)
{ 
	int n = minHeap->size - 1; 
	int i; 

	for (i = (n - 1) / 2; i >= 0; --i) 
		minHeapify(minHeap, i); 
} 

// A utility function to print an array of size n 
void printArr(int arr[], int n, struct globalvars *gvars)
{ 
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

// Utility function to check if this node is leaf 
int isLeaf(struct MinHeapNode* root) 
{ 
	return !(root->left) && !(root->right); 
} 

// Creates a min heap of capacity equal to size and inserts all character of data[] in min heap. Initially size of min heap is equal to capacity 
struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) 
{ 
	struct MinHeap* minHeap = createMinHeap(size); 
	for (int i = 0; i < size; ++i) 
		minHeap->array[i] = newNode(data[i], freq[i]); 
	minHeap->size = size; 
	buildMinHeap(minHeap); 
	return minHeap; 
} 

// The main function that builds Huffman tree 
struct MinHeapNode* buildHuffmanTree(char data[], int freq[], int size) 
{ 
	struct MinHeapNode *left, *right, *top; 

	// Create a min heap of capacity equal to size. Initially, there are modes equal to size. 
	struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size); 

	// Iterate while size of heap doesn't become 1 
	while (!isSizeOne(minHeap)) { 

		// Extract the two minimum freq items from min heap 
		left = extractMin(minHeap); 
		right = extractMin(minHeap); 

		// Create a new internal node with frequency equal to the sum of the two nodes frequencies. Make the two extracted node as 
		// left and right children of this new node. Add this node to the min heap '$' is a special value for internal nodes, not used 
		top = newNode('$', left->freq + right->freq); 
		top->left = left; 
		top->right = right; 

		insertMinHeap(minHeap, top); 
	} 

	// The remaining node is the root node and the tree is complete. 
	return extractMin(minHeap); 
} 

// Prints huffman codes from the root of Huffman Tree. It uses arr[] to store codes 
void printCodes(struct MinHeapNode* root, int arr[], int top, struct globalvars *gvars) 
{ 
	// Assign 0 to left edge and recur 
	if (root->left) {
		arr[top] = 0; 
		printCodes(root->left, arr, top + 1, gvars); 
	}
	// Assign 1 to right edge and recur 
	if (root->right) {
		arr[top] = 1; 
		printCodes(root->right, arr, top + 1, gvars); 
	} 
	// If this is a leaf node, then it contains one of the input characters, print the character and its code from arr[] 
	if (isLeaf(root)) {
		gvars->gchar[gvars->gi]= root->data;
		printArr(arr,top,gvars);
		gvars->gi=gvars->gi+1;
	} 
}

// The main function that builds a Huffman Tree and print codes by traversing the built Huffman Tree 
void HuffmanCodes(char data[], int freq[], int size, struct globalvars *gvars) 
{ 
	// Construct Huffman Tree 
	struct MinHeapNode* root = buildHuffmanTree(data, freq, size); 

	// Print Huffman codes using the Huffman tree built above 
	int arr[MAX_TREE_HT], top = 0;

	printCodes(root, arr, top, gvars);
} 

void writeBit(int b,FILE *f, struct globalvars *gvars)
{
	//Maintain static buffer, if it is full, write into file 
	char temp;
	if(b==1)
	{	temp=1;
		temp=temp<<(7-gvars->cnt);		//right shift bits
		gvars->byte=gvars->byte | temp;
	}
	(gvars->cnt)++;
	(gvars->length)++;	
	if(gvars->cnt==8)	//buffer full
	{
		fwrite(&gvars->byte,sizeof(char),1,f);
		gvars->cnt=0; gvars->byte=0;	//reset buffer
		return;// buffer written to file
	}
	return;
}


// creates a substring of size length
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

//check file format

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