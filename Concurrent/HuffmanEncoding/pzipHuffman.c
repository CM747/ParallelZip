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
void *RLEzip(void *);
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

    // Allocate Space for argc threads and their parameters, input and output files
    pthread_t *threads = (pthread_t *)malloc((argc-1) * sizeof(pthread_t));
    files *arg = (files *)malloc((argc-1) * sizeof(files));
    FILE **fp = (FILE **)malloc((argc-1) * sizeof(FILE *));
    FILE **fout = (FILE **)malloc((argc-1) * sizeof(FILE *));
	FILE **encode = (FILE **)malloc((argc-1) * sizeof(FILE *));
    int l;
    char* sub;
    for(int i=1; i<argc; i++){
        // Check file format and then proceed
        if(checkfileformat(argv[i])){
            l = strlen(argv[i]);
            fp[i-1] = fopen(argv[i], "r");
            
            sub = malloc(l-2);
            substring(argv[i],0,l-3,sub);
            strcat(sub, "pzip");
            fout[i-1] = fopen(sub, "wb");
            free(sub);
			sub = malloc(l-2);
            substring(argv[i],0,l-3,sub);
            strcat(sub, "encd");
            encode[i-1] = fopen(sub, "w");
            free(sub);

            arg[i-1].fin = fp[i-1];
            arg[i-1].fout = fout[i-1];
			arg[i-1].encode = encode[i-1];
            arg[i-1].c = i;

            // RLEzip(fp, fout);
            pthread_create(&threads[i-1], NULL, mainHuffman, (void *)&arg[i-1]);

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
		fclose(encode[i-1]);
    }

    gettimeofday(&end, NULL);
    printf("Main finished, Total Time Taken to zip %d files: %ld microsec\n", argc-1, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));
    
    free(threads);
    free(arg);

    
    return 0;
}


void *mainHuffman(void* param) 
{
	struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    files *iofiles = (files *)param;
    printf("Zipping: %d\n", iofiles->c);

	struct globalvars gvars;
	gvars.gi = 0;

	char m;
	int b[128],i,t=0,k,h;
	for(i=0;i<128;i++){
		b[i]=0;
	}
	
	m=getc(iofiles->fin);
	while(m!=EOF){
		b[(int) m]+=1;
		if(b[(int) m]==1){
			t++;
		}
		m=getc(iofiles->fin);
	}
	gvars.gsize=t;
	gvars.gchar=(char *)malloc(t*sizeof(char));
	gvars.gb=(char **)malloc(t*sizeof(char *));
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
	int size = sizeof(arr) / sizeof(arr[0]); 
	HuffmanCodes(arr, freq, size, &gvars);
	rewind(iofiles->fin);

	
	// for(k=0;k<gvars.gsize;k++){
	// 	printf("%c :",gvars.gchar[k]);
	// 	printf("%s\n",gvars.gb[k]);
	// }
	
	m=getc(iofiles->fin);
	while(m!=EOF){
		for(k=0;k<gvars.gsize;k++){
			if(gvars.gchar[k]==m){
				h=0;
				while(gvars.gb[k][h] !='\0'){
					writeBit(((int) gvars.gb[k][h] -48), iofiles->fout, &gvars);
					// printf("%c",gvars.gb[k][h]);
					h++;
				}
				break;
			}
		}
		m=getc(iofiles->fin);
	}
	if(gvars.cnt!=0){
		fwrite(&gvars.byte,sizeof(char),1, iofiles->fout);
	}
	
	fwrite(&gvars.gsize,sizeof(int),1,iofiles->encode);
	fwrite(&gvars.length,sizeof(int),1,iofiles->encode);
	fwrite(&arr,sizeof(char),gvars.gsize,iofiles->encode);
	fwrite(&freq,sizeof(int),gvars.gsize,iofiles->encode);

	gettimeofday(&end, NULL);
    printf("Zipping Completed: %d, Time Taken: %ld microsec\n", iofiles->c, (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec));

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
		// printf("%d", arr[i]);
		if(arr[i]==0){
			gvars->gb[gvars->gi][i]='0';
		}
		else{
			gvars->gb[gvars->gi][i]='1';	
		}
	}
	gvars->gb[gvars->gi][i]='\0';
	// printf("\n"); 
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

	// Step 1: Create a min heap of capacity equal to size. Initially, there are modes equal to size. 
	struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size); 

	// Iterate while size of heap doesn't become 1 
	while (!isSizeOne(minHeap)) { 

		// Step 2: Extract the two minimum freq items from min heap 
		left = extractMin(minHeap); 
		right = extractMin(minHeap); 

		// Step 3: Create a new internal node with frequency equal to the sum of the two nodes frequencies. Make the two extracted node as 
		// left and right children of this new node. Add this node to the min heap '$' is a special value for internal nodes, not used 
		top = newNode('$', left->freq + right->freq); 
		top->left = left; 
		top->right = right; 

		insertMinHeap(minHeap, top); 
	} 

	// Step 4: The remaining node is the root node and the tree is complete. 
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
		// printf("%c: ",root->data);
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