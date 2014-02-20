/* Assignment 2 : HTTP Proxy

Simrat Singh Chhabra
Roll No : 11010165

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>


using namespace std;



#define BACKLOG 10 // how many pending connections queue will hold

/*get_req from client gets the request from the connected client and stores it in buf*/
int get_req_from_client(int new_fd, char *buf, int buf_size); 

/*parses the request from the client and checks for incorrect syntax or methode which are not implemented*/
int parse_req(char *buf, int max, char *newbuf, int& newind, char* port, char* host, int new_fd);

/*gets the next word ignoring whitespace (used while parsing the request)*/
int getnextword(char* buf, int max, int& ind, char *word);

/*sends a Bad Request error message to the client*/
void send_bad_req_error(int new_fd);

/*sends a Not Implemented error message to the client*/
void send_not_implem_error(int new_fd);


void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int main(int argc, char *argv[])
{
	if (argc!=2)
	{
		fprintf(stderr, "usage: ./proxy portno\n");
		exit(1);
	}
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int req_stat;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use own IP
	int byte_count;
	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
		/* loop through all the results and bind to the first one possible*/
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); 

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	cout << "Proxy server running..." << endl;
	while(1) {  

		sin_size = sizeof their_addr;
		
		/*Accept an incoming connection on a listening socket (new_fd)*/
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		
		if (new_fd == -1) {
			perror("accept");
			
			continue;
		}
		char buf[5000]; /*buf will store the raw request received from client*/
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		
		byte_count=get_req_from_client(new_fd, buf, sizeof buf); //This function will fetch the request
		if(byte_count==-1)
		{
			fprintf(stderr, "Unable to get data. Disconnected.\n");
			continue;
		}
	
		char *newbuf;
		if((newbuf=(char*)malloc(50000))==NULL)
		{
			fprintf(stderr, "Unable to allocate newbuf. Disconnected.\n");
			continue;
		}
		char *host;
		if((host=(char*)malloc(200))==NULL)
		{
			fprintf(stderr, "Unable to allocate host. Disconnected.\n");
			continue;

		}
		int newind=0;

		char *port;
		if((port=(char*)malloc(10))==NULL)
		{
			fprintf(stderr, "Unable to allocate port. Disconnected.\n");
			continue;
		}
		strcpy(port, "80");
	

		int pr = parse_req(buf, byte_count, newbuf, newind, port, host, new_fd);
		if(pr==-1)
		{
			fprintf(stderr, "Error in request. Disconnected.\n");
			continue;
		}
		

		int sockfd2;
		struct addrinfo hints2, *servinfo2, *p2;

		memset(&hints2, 0, sizeof hints2);
		hints2.ai_family = AF_UNSPEC;
		hints2.ai_socktype = SOCK_STREAM;
		int rv2;
		
		//Getting the info of the server (stored in host)
		if ((rv2 = getaddrinfo(host, port, &hints2, &servinfo2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2 ));
		return 1;
		}

		

		// loop through all the results and connect to the first we can
		for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
			if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
					p2->ai_protocol)) == -1) {
				perror("client: socket");
				continue;
			}

			if (connect(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
				close(sockfd);
				perror("client: connect");
				continue;
			}

			break;
		}

		if (p2 == NULL) {
		fprintf(stderr, "failed to connect to server\n");
		return 2;
		}

		inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr),
			s, sizeof s);
		

		freeaddrinfo(servinfo2); 

		int bytes_sent=0;
		/*sending the parsed request to the server*/
		bytes_sent=send(sockfd2, newbuf, newind, 0);
		if(bytes_sent==-1)
		{
			perror("sending req to server failed");
			continue;
		}

		while(bytes_sent<newind)
		{
			bytes_sent+=send(sockfd2, newbuf+bytes_sent, newind-bytes_sent, 0);
			if(bytes_sent==-1)
			{
				perror("sending req to server failed");
				continue;
			}
		}

	
		int numbytes;
		char *recvbuf;
		if((recvbuf=(char*)malloc(500000))==NULL)
		{
			fprintf(stderr, "Unable to allocate recvbuf. Disconnected.\n");
			continue;

		}
		int cur=0;
		/* receiving reply from the server and storing it into recvbuf*/
		while((numbytes = recv(sockfd2, recvbuf+cur, 499999-cur, 0)) > 0) {
			
			cur+=numbytes;   

		}
		if(numbytes==-1)
		{
			perror("receiving data from server failed");
			continue;
		}
		
		
		
		
		
		int datasize=cur;
		cur=0;
		/* sending the data received from the server as it is to the client*/
		while((bytes_sent=send(new_fd, recvbuf+cur, datasize-cur, 0))>0)
		{
			
			cur+=bytes_sent;
		}
		
		if(bytes_sent==-1)
		{
				perror("sending data back to client failed");
				
		}
		close(new_fd);


		
	}


	return 0;

}

int get_req_from_client(int new_fd, char *buf, int buf_size)
{
	bool end=false;
	bool flag=false;
	int cur_count=0;
	
	
	while(!end)
	{

	

	/*receiving request from client*/
	int byte_count = recv(new_fd, buf+cur_count, buf_size, 0);
	if(byte_count<=0)
	{
		perror("Error getting request from client");
		return -1;
	}
	
		int k=0;
		while(k<byte_count)
		{
			if(buf[k+cur_count]=='\r' && buf[k+1+cur_count]=='\n')
			{
				
				if(flag)
				{
					end=true;
					/*this breaks out when it sees 2 consecutive <CRLF>*/
					
				}
			
				flag=true;

				break;

			}
			else
			{
				flag=false;
			}
		
			k++;

		}
		cur_count+=byte_count;
		
	}	
	return cur_count;


}


void send_bad_req_error(int new_fd)
{
	char error_mes[50];
	sprintf(error_mes, "Error 400 : Bad Request\n");
	if(send(new_fd, error_mes, strlen(error_mes), 0)==-1)
	{
		perror("sending req failed");

	}
	close(new_fd);

	return;
}

void send_not_implem_error(int new_fd)
{

	char error_mes[50];
	sprintf(error_mes, "Error 501 : Not Implemented\n");
	if(send(new_fd, error_mes, strlen(error_mes), 0)==-1)
	{
		perror("sending req failed");

	}

	close(new_fd);

	return;


}


int parse_req(char *buf, int max, char *newbuf, int& newind, char* port, char *host, int new_fd)
{
	char *word;
	word = (char*)malloc(5000);
	int ind=0;
	int word_size=getnextword(buf, max, ind, word);
	if(word_size==0)
	{
		send_bad_req_error(new_fd);
		return -1;
	}
	if(word_size!=3  || strncmp("GET", word, 3)!=0)
	{
		if(strncmp("POST", word, 4)!=0 || strncmp("HEAD", word, 4)!=0 )
		{
			send_not_implem_error(new_fd);
		}
		else
		{
			send_bad_req_error(new_fd);

		}
		return -1;
	}

	/* path stores the absolute or relative path given by the client*/
	char *path;
	if((path=(char*)malloc(1000))==NULL)
	{

			fprintf(stderr, "Unable to allocate path. Disconnected.\n");
			return -1;
	}

	word_size=getnextword(buf, max, ind, word);
	if(word_size==0)
	{
		send_bad_req_error(new_fd);
		return -1;

	}
	int k=0;
	
	bool is_url=true;
	bool is_port=false;
	
	int hostlen;
	int d=0;
	
	if(strncmp(word, "http://", 7)==0)
	{
		
		is_url=true;
		d=7;
		hostlen=0;
		while(word[d]!='/' && word[d]!=':' && d<word_size)
		{
			strncat(host, word+d, 1);
			d++;
			hostlen++;
		}
		if(word[d]==':')
		{
			d++;
			is_port=true;
			strcpy(port, "\0");
			while(word[d]!='/' && d<word_size)
			{
				strncat(port, word+d, 1);
				d++;
			}
		}
		if(word[d]=='/')
		{
			while(d<word_size)
			{
				strncat(path, word+d, 1);
				d++;
			}

		}
		else
		{
			strcat(path, "/");
		}		
		

	}
	else if (word[0]=='/')
	{
		
		is_url=false;
		strncpy(path, word, word_size);
	}
	else
	{
		send_bad_req_error(new_fd);
		return -1;
		
	}
	
	word_size=getnextword(buf, max, ind, word);


	if(strncmp("HTTP/1.0", word, word_size)==0 || strncmp("HTTP/1.1", word, word_size)==0 || strncmp("HTTP/0.9", word, word_size)==0)
	{
		
	}
	else
	{
		send_bad_req_error(new_fd);
		return -1;
		
	}


	if(!is_url)
	{
		word_size=getnextword(buf, max, ind, word);
		
		if(strncmp("Host:", word, word_size)!=0)
		{
			send_bad_req_error(new_fd);
			return -1;
		}
		else
		{
			d=0;
			hostlen=0;
			word_size=getnextword(buf, max, ind, word); 
			while(word[d]!=':' && d<word_size)
			{
				strncat(host, word+d, 1);
				d++;
				hostlen++;
			}
			if(word[d]==':')
			{
				d++;
				is_port=true;
				strcpy(port, "\0");
			}
			while(d<word_size)
			{
				strncat(port, word+d, 1);
				d++;
			}
			
		}
	}

	
	/*new buf stores the modified parsed request*/

	strcat(newbuf, "GET ");
	strcat(newbuf, path);
	strcat(newbuf, " ");

	strcat(newbuf, "HTTP/1.0\r\nHost: ");
	strcat(newbuf, host);
	if(is_port)
	{
		strcat(newbuf, ":");
		strcat(newbuf, port);
		
		
	}
	strcat(newbuf, "\r\n");
	
	ind+=2;
	while(ind<max)
	{
	
		if(buf[ind]=='\r' || buf[ind]=='\n')
		{
			strcat(newbuf, "\r\n");
			break;
		}
		word_size=getnextword(buf, max, ind, word);
		if(word[word_size-1]!=':')
		{
			send_bad_req_error(new_fd);
			return -1;
			
			
			
		
		}
		
		strncat(newbuf, word, word_size);
		
		strcat(newbuf, " ");
		
		ind++;
		
		while(!(buf[ind]=='\r' || buf[ind+1]=='\n'))
		{	
			strncat(newbuf, buf+ind, 1);
		
			ind++;

		}
		strcat(newbuf, "\r\n");
		ind+=2;
	}
	strcat(newbuf, "\r\n");
	
	newind=strlen(newbuf);
	return 0;



}

int getnextword(char* buf, int max, int& ind, char *word)
{
	while(isspace(buf[ind]))
	{
		ind++;
	}
	int start=ind;
	while(!isspace(buf[ind]))
	{
		ind++;
	}
	int end=ind;

	strncpy(word, buf+start, end-start);
	
	cout << endl;
	return end-start;
}