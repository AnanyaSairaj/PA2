#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */
#define DELIMITER (" ")
#define DELIMITER1 ("\n")
#define t_out 10 //timeout value
int open_listenfd(int port);//open the port for listening
void response(int connfd);//function for echo
void *thread(void *vargp);//thread function

void HTTP_OK(char *msg, char *version, ssize_t f_size, char *f_type, char *conn_status)
{
	snprintf(msg, 512, "%s 200 OK\r\n""Content-Type: %s\r\n""Connection: %s\r\n""Content-Length: %ld\r\n\r\n", version, f_type, conn_status, f_size);//Print the OK message for OK response
}
void HTTP_ERROR(char *msg, char *version, char *conn_status, int c_size)
{
	if (conn_status != NULL)
		snprintf(msg, 512, "%s 500 Internal Server Error\r\n""Content-Type: html\r\n""Connection: %s\r\n""Content-Length: %d\r\n\r\n", version, conn_status, c_size);//Error response if persistent connection is there in the request
	else
		snprintf(msg, 512, "%s 500 Internal Server Error\r\n""Content-Type: html\r\n""Connection: Close\r\n""Content-Length: %d\r\n\r\n", version, c_size);//Error response for non-persistent connections
}
void get_post_data(char *msg, char *post_data)
{
	if (strstr(msg,"POST")!=NULL)//check if the message post exists
	{
	//printf("THE VALUE OF THE BUFFER IS %s",msg);
	char *ptr = NULL;   //initialize a variable and assign to null
	if((ptr = strstr(msg,"\r\n\r\n"))!=NULL)//get the instance of carriage return
	{ptr=ptr+strlen("\r\n\r\n");//increment the value to value after carriage return aka the post data value
	strcpy(post_data, ptr);}
	printf("THE VALUE OF THE POST DATA IS %s\n",post_data);//copy the post data into a pointer and display with string
		}
	else
		printf("Empty input buffer\n");
}
void get_url_components(char *url, char *f_name, char *f_type)
{      printf("URL IS %s",url);
	char type[5];//initialise an array of 5 
	//int ptr =strlen(url);
	
	char *ptr="NULL";//pointer and variables assigned to 0
	memset(type, 0, sizeof(type));

	
	ptr=strrchr(url,'.');//check for the last occurence of the char and return to the ptr
	//printf("\nTHE VALUE OF THE POINTER IS %s",ptr);
	ptr=ptr+1;
	strcpy(type,ptr);//copy the ptr value into type
	
	ptr=strrchr(url,'/');//check for the lasst occurence of '/' and return in the pointer value
	//printf("\n THE VALUE OF THE POINTER IS %s",ptr);
	ptr=ptr+1;
	strcpy(f_name,ptr);//copy the update pointer value to the name
	
	if (strcmp(type, "js") == 0) {//compare the type with js 
			strcpy(f_type, "application/javascript");
	}
	else
	{
		switch (type[0])//switch case function to check the type
		{
			case 'h':
				strcpy(f_type, "text/");//if type is for this case cancatenate with the type specified
				strcat(f_type, type);
				break;

			case 't':
				strcpy(f_type, "text/plain");
				break;

			case 'p':
				strcpy(f_type, "image/");
				strcat(f_type, type);
				break;

			case 'g':
				strcpy(f_type, "image/");
				strcat(f_type, type);
				break;

			case 'j':
				strcpy(f_type, "image/");
				strcat(f_type, type);
				break;

			case 'c':
				strcpy(f_type, "text/");
				strcat(f_type, type);
				break;

			default:
				strcpy(f_type, "text/");//for default casse take it as text/html
				strcat(f_type, "html");
            break;	
	}
	}
	printf("THE FILE TYPE IS %s\n",f_type);
	
}

int main(int argc, char **argv) 
{
    int listenfd, *connfdp, port;
    unsigned int clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid; 

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);//assign the port

    listenfd = open_listenfd(port);//call the listen fuction
    while (1) {
	connfdp = malloc(sizeof(int));//allocate the memory space for the socket
	*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &(clientlen));//accept the incoming client requests
	pthread_create(&tid, NULL, thread, connfdp);//create a thread for each individual thread
    }
}

/* thread routine */
void * thread(void * vargp) 
{  
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());//attribute ass pthread detach so that resources are automatically released instead of having to join 
    free(vargp);
    response(connfd);//call the response function 
    close(connfd);
    return NULL;
}


void response(int connfd) 
{
	char r_buffer[MAXBUF];//Stores the CLient Response
	char s_buffer[512];  //Stores HTTP_Ok_Response
 	char e_buffer[512]; //Stores HTTP_Error_Response
	
	char p_buffer[512];
	
	char post_data[512];	//Stores Content of post data
	char method[10];	//Stores the Requested HTTP Method
	char url[30];		//Stores the Requested File Path
	char path[35];
	char version[10];	//Stores the HTTP Version
	char f_type[30];	//Stores the type of file requested
	char f_name[30];	//Stores the file name requested
	int c_size;
	char error_msg[5][512]; //an array of strings to store the error messages
	
	strcpy(error_msg[1],"<!DOCTYPE html><html><title>Invalid Request</title>""<pre><h1>400 BAD REQUEST: REASON INAVALID METHOD</h1></pre>""</html>\r\n");
	
	strcpy(error_msg[2],"<!DOCTYPE html><html><title>Invalid Request</title>""<pre><h1>400 BAD REQUEST: REASON INAVALID HTTP VERSION</h1></pre>""</html>\r\n");
	
	strcpy(error_msg[3],"<!DOCTYPE html><html><title>Invalid Request</title>""<pre><h1>400 BAD REQUEST: REASON INAVALID URL</h1></pre>""</html>\r\n");
	
	strcpy(error_msg[4],"<!DOCTYPE html><html><title>Invalid Request</title>""<pre><h1>400 BAD REQUEST: REASON INAVALID URL</h1></pre>""</html>\r\n");
	
	
	
	ssize_t f_size;
	
	struct timeval timeout;//for the  timeout for persistent functions	
	struct stat st;
	
	
	
	while ((recv(connfd, r_buffer, MAXBUF, 0)) > 0)//recive the client request into the http
{
	//clear all the buffers with memset		
	memset(s_buffer, 0, sizeof(s_buffer));
	memset(method, 0, sizeof(method));
	memset(url, 0, sizeof(url));
	memset(path, 0, sizeof(path));
	memset(version, 0, sizeof(version));
	memset(f_type, 0, sizeof(f_type));
	memset(f_name, 0, sizeof(f_name));
	
	char *saveptr=0;//saveptr for strtok_r 
	
	
	strcpy(p_buffer,r_buffer);//since strtok_r makes changes to the buffer, copy the request to another buffer to parse
	strcpy(method, strtok_r(p_buffer,DELIMITER,&saveptr));//strtok_r to parse till the specified delimiter
	strcpy(url ,strtok_r(NULL,DELIMITER,&saveptr));
	strncpy(version, strtok_r(NULL,DELIMITER,&saveptr),8);
	//sscanf(r_buffer, "%s %s %s", method, url, version);
	//printf("%s %s %s\n\n", method,url,version);

	char *conn_status = strstr(r_buffer, "Connection: keep-alive");//check for the connection status and set the timeout period
	if (conn_status)
{
	printf("%s\n", conn_status);
	timeout.tv_sec = t_out;//timeout value for keep alive
	setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(struct timeval));
				         }
	else {
	timeout.tv_sec = 0;//no value for any others
	setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(struct timeval));
				  }
 
	printf("Method: %s\nPath: %s\nVersion: %s\n", method, url, version);
if ((strcmp(method, "GET") != 0) && (strcmp(method, "POST") != 0) && (strcmp(method, "HEAD") != 0))//check for metho error
{
	printf("INVALID METHOD\n");
	c_size = strlen(error_msg[1]);

	if (conn_status)
	HTTP_ERROR(e_buffer, version, "keep-alive", c_size);
	else
	HTTP_ERROR(e_buffer, version, "Close", c_size);

	send(connfd, e_buffer, strlen(e_buffer), 0);
	send(connfd, error_msg[1], strlen(error_msg[1]), 0);
	if (conn_status) {
		continue;
					}
	else {
		printf("Closing Socket \n");
		close(connfd);
		exit(0);
					}
				}
	//check for version error
if ((strcmp(version, "HTTP/1.1") != 0) && (strcmp(version, "HTTP/1.0") != 0))
{
	printf("INVALID VERSION\n");
	c_size = strlen(error_msg[2]);

        if (conn_status)
	    HTTP_ERROR(e_buffer, version, "keep-alive", c_size);
	else
	    HTTP_ERROR(e_buffer, version, "Close", c_size);

	send(connfd, e_buffer, strlen(e_buffer), 0);
	send(connfd, error_msg[2], strlen(error_msg[2]), 0);
					
	if (conn_status) {
	continue;
					}
	else {
	printf("Closing Socket \n");
	close(connfd);
	exit(0);
					}
				}
       strcpy(path, "./www");//copy default directory into the path value

	if (strcmp(url,"/") == 0)
{
	strcat(path, "/index.html");//for default cancatenate with index.html
				}
	else
{
	strcat(path, url);//else cancatenate with the type 
				}

	//check for path access error
if (access(path, F_OK) != 0)
{
	printf("INVALID URL\n");
	c_size = strlen(error_msg[3]);

        if (conn_status)
	HTTP_ERROR(e_buffer, version, "keep-alive", c_size);
	else
	HTTP_ERROR(e_buffer, version, "Close", c_size);

	send(connfd, e_buffer, strlen(e_buffer), 0);
	send(connfd, error_msg[3], strlen(error_msg[3]), 0);
					
	if (conn_status) {
	continue;
			}
	else {
	printf("Closing Socket \n");
	close(connfd);
	exit(0);
					}
				}
// THIS IS THE GET METHOD 
if (strcmp(method, "GET") == 0)//inside the get method
{
	printf("Processing the GET request\n");
	memset(s_buffer, 0, sizeof(s_buffer));
	get_url_components(path, f_name, f_type);//get the url components
	stat(path, &st);
	f_size = st.st_size;//get file size using stat 
	printf("Path -> %s  file_name -> %s file_type -> %s	fs -> %ld\n",path, f_name, f_type, f_size); //print the header variables

	if (conn_status)
	HTTP_OK(s_buffer, version, f_size, f_type, "keep-alive");//response 200-OK with keep alive
	else
	HTTP_OK(s_buffer, version, f_size, f_type, "Close");//http ok response 200-ok  with close for zero timeout
	
					
	printf("\n%s\n", s_buffer);
	send(connfd, s_buffer, strlen(s_buffer), 0);//send the response to the socket

	int nsize;
	char *buffer = (char *) malloc(f_size*sizeof(char));//assign a buffer pointer with memory of file size times charecters
		FILE *fptr;//assign a file ptr to traverse the file
		fptr = fopen(path, "r");//open file in read mode
		nsize = fread(buffer, 1, f_size, fptr);//fread to read the file into the buffer

		send(connfd, buffer, nsize, 0);//send the buffer value to the socket
					
		fclose(fptr);//close the file
		free(buffer);//free the buffer

	if (conn_status == NULL) {//close the socket if conn status is close
		printf("Closing Socket \n");
		close(connfd);
		exit(0);
					}
				}								    
    
    
if (strcmp(method, "POST") == 0)
 {
	printf("Processing Post Request\n");
	memset(s_buffer, 0, sizeof(s_buffer));
	get_url_components(path, f_name, f_type);//get url components
	get_post_data(r_buffer, post_data);//get the post data by parsing from the request string
	//printf("%s\n", post_data);

	stat(path, &st);
	f_size = st.st_size;//file size with the stat functions
	printf("Path -> %s    file_name -> %s file_type -> %s   fs -> %ld\n",path, f_name, f_type, (f_size + strlen(post_data)));//append the file size value with the post data string lenght and print the values

       if (conn_status)
       	HTTP_OK(s_buffer, version, (f_size + strlen(post_data)), f_type, "keep-alive");//ok response for keep alive
       else
       {	HTTP_OK(s_buffer, version, (f_size + strlen(post_data)), f_type, "Close");//ok response for close connection status
}
	printf("\n%s\n", s_buffer);
	send(connfd, s_buffer, strlen(s_buffer), 0);//send the ok response in the buffer

	char send_msg[512];
	snprintf(send_msg, 512, "<html><body><pre><h1>%s</h1></pre></html>", post_data);//send the message in html format 
	printf("\n%s\n", send_msg);
	send(connfd, send_msg, strlen(send_msg), 0);//send the html value 

	int nsize;
	char *buffer = (char *) malloc(f_size*sizeof(char));//assign a buffer pointer with memory of file size times charecters
		     FILE *fptr;

                    fptr = fopen(path, "r");//assign a file ptr to traverse the file
                    nsize = fread(buffer, 1, f_size, fptr);//fread to read the file into the buffer
		     printf("%s\n", buffer);
		     

                    send(connfd, buffer, nsize, 0);//send the buffer value through the socket

                    fclose(fptr);//close the fptr
                    free(buffer);//free the buffer

	if (conn_status == NULL) {
		printf("Closing Socket \n");
		close(connfd);
		exit(0);//close the socket and exit if close
					}
				}
			}
	printf("Connection Close due to timeout for socket \n");
		}
	

/* 
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure 
 */
int open_listenfd(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    //Create a socket descriptor 
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    // Eliminates "Address already in use" error from bind.
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    // listenfd will be an endpoint for all requests to port on any IP address for this host 
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    //Make it a listening socket ready to accept connection requests 
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} // end open_listenfd 

