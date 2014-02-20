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



#define BACKLOG 3
//#define PORT "80"

int get_req_from_client(int new_fd, char *buf, int buf_size);
int parse_req(char *buf, int max, char *newbuf, int& newind, char* port, char* host);
int getnextword(char* buf, int max, int& ind, char *word);

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
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; 
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int req_stat;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	int byte_count;
	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

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

	freeaddrinfo(servinfo); // all done with this structure

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
		//cout << "before accept" << endl;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		//cout << "after accept" << endl;
		if (new_fd == -1) {
			perror("accept");
			cout << "Here sir" << endl;
			//continue;
		}
		char buf[512];
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		//printf("%d\n", sizeof buf);
		byte_count=get_req_from_client(new_fd, buf, sizeof buf);
		
		//

		//

		//rec_reply_from_server()

		//send_reply_to_client()

		

		
		printf("recv()'d %d bytes of data in buf\n", byte_count);
		int k=0;
		while(k<byte_count)
		{
			printf("%c", buf[k]);
			k++;
		}
		printf("\n");
		char *newbuf;
		newbuf=(char*)malloc(50000);
		char *host;
		host=(char*)malloc(200);
		int newind=0;

		char *port;
		port=(char*)malloc(10);
		strcpy(port, "80");
		int pr = parse_req(buf, byte_count, newbuf, newind, port, host);
		k=0;
		printf("NEW BUFFER!!\n");
		while(k<newind)
		{
			printf("%c", newbuf[k]);
			k++;
		}
		printf("\n");
		cout << newind << endl;

		int sockfd2;
		struct addrinfo hints2, *servinfo2, *p2;

		memset(&hints2, 0, sizeof hints2);
		hints2.ai_family = AF_UNSPEC;
		hints2.ai_socktype = SOCK_STREAM;
		int rv2;
		//char* finalport;
		//finalport=(char*)malloc(10);
		//cout << port;
		if ((rv2 = getaddrinfo(host, port, &hints2, &servinfo2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2 ));
		return 1;
		}

		cout << rv2 << endl;

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
		printf("server: connecting to %s\n", s);

		freeaddrinfo(servinfo2); // all done with this structure



		//int srs = send_req_to_server()
		//send_req_to_server()

		close(new_fd);
		/*printf("server: got connection from %s\n", s);
		if (send(new_fd, "Hello, world!", 13, 0) == -1)
				perror("send");
		*/	
			//char* temp;
			

			/*scanf("%s", temp);
			if (send(new_fd, temp, strlen(temp), 0) == -1)
				perror("send");
			

			close(new_fd);
			*///exit(0);


		/*if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
		


			if (send(new_fd, "Hello, world!", 13, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this*/
	}


	return 0;

}

int get_req_from_client(int new_fd, char *buf, int buf_size)
{
	bool end=false;
	bool flag=false;
	int cur_count=0;
	//char buf[512];
	while(!end)
	{


	int byte_count = recv(new_fd, buf+cur_count, buf_size, 0);
	printf("%d\n", byte_count);
	//printf("recv()'d %d bytes of data in buf\n", byte_count);
		int k=0;
		while(k<byte_count)
		{
			if(buf[k+cur_count]=='\r' && buf[k+1+cur_count]=='\n')
			{
				//printf("here\n");
				if(flag)
				{
					end=true;
					//cout << "end" << endl;
				}
				//cout << "flag" << endl;
				flag=true;

				break;

			}
			else
			{
				flag=false;
			}
			//printf("%c", buf[k]);
			k++;

		}
		cur_count+=byte_count;
		printf("\n");
	}	
	return cur_count;


}


int parse_req(char *buf, int max, char *newbuf, int& newind, char* port, char *host)
{
	if(strncmp("GET", buf, 3)!=0)
	{
		//Throw up error !
	}
	int ind=3;
	//int fwd=0;

	if(!isspace(buf[ind]))
	{
		//bad request
	}
	ind++;
	char *word;
	word = (char*)malloc(1000);

	int word_size=getnextword(buf, max, ind, word);
	int k=0;
	while(k<word_size)
	{
		printf("%c", word[k]);
		k++;
	}

	printf("\n");
	cout << "Checkpoint1" << endl;
	bool is_url=true;
	
	char *path;
	int hostlen;
	
	cout << "Checkpoint5" << endl;
	if(strncmp(word, "http", 4)==0)
	{
		cout << "Checkpoint8" << endl;
		cout << strlen(word) << endl;
		is_url=true;
		cout << strncpy(host, word+7, strlen(word)-7) << endl;
		hostlen=word_size-7;
		cout << "Checkpoint3" << endl;

	}
	else if (word[0]=='/')
	{
		cout << "Checkpoint10" << endl;
		is_url=false;
		strcpy(path, word);
	}
	else
	{
		cout << "Checkpoint6" << endl;
	}
	cout << "Checkpoint2" << endl;
	//ind+=word_size;
	if(!isspace(buf[ind]))
	{
		//bad request
	}
	word_size=getnextword(buf, max, ind, word);
	//ind+=word_size;
	k=0;
	while(k<word_size)
	{
		printf("%c", word[k]);
		k++;
	}

	printf("\n");
	if(strcmp("HTTP/1.0", word)==0 || strcmp("HTTP/1.1", word)==0 || strcmp("HTTP/0.9", word)==0)
	{
		//ok
	}
	else
	{
		//bad request
	}


	if(!is_url)
	{
		word_size=getnextword(buf, max, ind, word);
		//ind+=word_size;
		if(strcmp("Host:", word)!=0)
		{
			//bad request
		}
		else
		{
			word_size=getnextword(buf, max, ind, word);
			strcpy(host, word);
			hostlen=word_size;
		}
	}

	
	//int newind=0;
	

	strcat(newbuf, "GET / HTTP/1.0\r\nHost: ");
	newind=24;
	strcat(newbuf, host);
	newind+=hostlen;
	strcat(newbuf, "\r\n");
	newind+=2;
	ind++;
	while(ind<max)
	{
		cout << ind << endl;
		if(buf[ind]=='\r' && buf[ind+1]=='\n')
		{
			break;
		}
		word_size=getnextword(buf, max, ind, word);
		if(word[word_size-1]!=':')
		{
			//bad request
			
			
			
		
		}
		cout << "Word" << word << endl;
		strncat(newbuf, word, word_size);
		newind+=word_size;
		strcat(newbuf, " ");
		newind++;
		ind++;
		cout << word << endl;
		cout << ind << endl;
		//ind=0;
		while(!(buf[ind]=='\r' && buf[ind+1]=='\n'))
		{	//cout << buf[ind];
			strncat(newbuf, buf+ind, 1);
			newind++;
			ind++;

		}
		strcat(newbuf, "\r\n");
		ind+=2;
	}
	strcat(newbuf, "\r\n");


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
	// cout << "ingetnextword" << endl;
	// int k=0	;
	// while(k<(end-start))
	// {
	// 	printf("%c", word[k]);
	// 	k++;
	// }
	cout << endl;
	return end-start;
}