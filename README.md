# ParallelZip

The aim of this project is to build a parrallelized version of non concurrent zip, in order to reduce the time taken to zip multiple files.

First we build a parallel zip that uses Run Length Encoding(RLE) algorithm for text files. We optimized our code for time taken and memory usage and went from v1 to v3 (v3 being the best).

Then we also build another parallel zipping zip for text files that uses Huffman encoding as the zipping algorithm.
At last we designed a parallel zip for image files. It uses RLE algoritm to compresss grayscale images.

## RLE Zip: The 3 versions explained
### Version 1
	Number of threads = Number of Files
	Files are being opened serially, and then file pointers are send to the threads. So there is no IO inside threads.

### Version 2
	Number of threads = Number of Files
	Files are opened inside of threads. So IO inside threads.

Below Graph shows time(microsec) vs number of files (Green line: Version 1; Blue line: Version 2):
![](/Concurrent/RLE_Encoding/graphs/v1vsv2_time.png)

As the number of files increases, the time taken by version 1 increases more rapidly than the time taken by version 2, because of the serial opening of large amount of files.

Till now we are using as many threads as the number of files. But this is not good as it uses a huge amount of memory resources. And also the OS might KILL a process when it uses too much of memory resources.

So to have to limit the number of threads, to decrease the memory resources. And also to do the job in minimum time, we plotted time vs number of threads graph. At each iteration 500 files are zipped on X(x-axis value) number of threads, and allocation of threads is done in a FIFO fashion.
![](/Concurrent/RLE_Encoding/graphs/v3_optimalThreads.png)

From the graph we can see that as the number of threads is increased beyond somewhere 15, the time taken is almost constant. So to keep the memory usage as well as the time low, we kept 15 as the optimal number of threads.

### Version 3
	Number of threads = 15
	Files are opened inside of threads. So IO inside threads.
	Files is assigned to threads in a FIFO fashion.

FIFO means that after 15 files are assigned to the 15 threads, the 16th file can only be assignesd to the first thread.

Below graph shows the memory usage(in KB) vs number of files(version 2(green) and version 3(blue))
![](/Concurrent/RLE_Encoding/graphs/v2vsv3_mem.png)

From the graph we can see that after 15 threads memory usage of version 3 is almost constant, while that of version 2 increases.


### Version 4
	Number of threads = 15
	Files are opened inside of threads. So IO inside threads.
	Any thread which finished its work can take up another work from the tasks queue.

Below graph shows time reduction from v3(blue line) to v4(green line) as number of files
![](/Concurrent/RLE_Encoding/graphs/v3vsv4_time.png)


## Non-Concurrent Zip Vs RLE Zip v3

Time vs number of files (Simple Zip: Green; Parallel Zip: Blue)
![](/Concurrent/RLE_Encoding/graphs/nonConcurrent_vs_v3_time.png)

Memory vs number of Files (Simple Zip: Green; Parallel Zip: Blue)
![](/Concurrent/RLE_Encoding/graphs/nonConcurrent_vs_v3_mem.png)
