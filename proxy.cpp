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



#define BACKLOG 10
/*get_req from client gets the request from the connected client and stores it in buf*/
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

	//cout << "Proxy server running..." << endl;
	while(1) {  

		sin_size = sizeof their_addr;
		//cout << "before accept" << endl;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		//cout << "after accept" << endl;
		if (new_fd == -1) {
			perror("accept");
			
			continue;
		}
		char buf[512];
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		//printf("server: got connection from %s\n", s);
		//printf("%d\n", sizeof buf);
		byte_count=get_req_from_client(new_fd, buf, sizeof buf);
		if(byte_count==-1)
		{
			fprintf(stderr, "Unable to get data. Disconnecting...\n");
			continue;
		}
		//

		//

		//rec_reply_from_server()

		//send_reply_to_client()

		

		
		//printf("recv()'d %d bytes of data in buf\n", byte_count);
		/*int k=0;
		while(k<byte_count)
		{
			printf("%c", buf[k]);
			k++;
		}*/
		//printf("\n");
		char *newbuf;
		if((newbuf=(char*)malloc(50000))==NULL)
		{
			fprintf(stderr, "Unable to allocate newbuf. Disconnecting...\n");
			continue;
		}
		char *host;
		if((host=(char*)malloc(200))==NULL)
		{
			fprintf(stderr, "Unable to allocate host. Disconnecting...\n");
			continue;

		}
		int newind=0;

		char *port;
		if((port=(char*)malloc(10))==NULL)
		{
			fprintf(stderr, "Unable to allocate port. Disconnecting...\n");
			continue;
		}
		strcpy(port, "80");
	
		int pr = parse_req(buf, byte_count, newbuf, newind, port, host);
		//k=0;
		//printf("NEW BUFFER!!\n");
		/*while(k<newind)
		{
			printf("%c", newbuf[k]);
			k++;
		}*/
		//printf("\n");
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
		//printf("server: connecting to %s\n", s);

		freeaddrinfo(servinfo2); // all done with this structure


		int bytes_sent=0;
		//int suc_sent=0;
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

		//cout << "Send :" << bytes_sent << endl;
		//int srs = send_req_to_server()
		//send_req_to_server()
		int numbytes;
		char *recvbuf;
		if((recvbuf=(char*)malloc(500000))==NULL)
		{
			fprintf(stderr, "Unable to allocate recvbuf. Disconnecting...\n");
			continue;

		}
		int cur=0;
		while((numbytes = recv(sockfd2, recvbuf+cur, 499999-cur, 0)) > 0) {
			
			cur+=numbytes;   

		}
		if(numbytes==-1)
		{
			perror("receiving data from server failed");
			continue;
		}
		
		
		
		
		//cout << cur << endl;
		int datasize=cur;
		cur=0;
		while((bytes_sent=send(new_fd, recvbuf+cur, datasize-cur, 0))>0)
		{
			
			cur+=bytes_sent;
		}
		//cout << cur << endl;
		if(bytes_sent==-1)
		{
				perror("sending data back to client failed");
				//break;
		}
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
	if(byte_count==-1)
	{
		perror("Error getting request from client");
	}
	//printf("%d\n", byte_count);
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
		//printf("\n");
	}	
	return cur_count;


}


int parse_req(char *buf, int max, char *newbuf, int& newind, char* port, char *host)
{
	char *word;
	word = (char*)malloc(5000);
	int ind=0;
	int word_size=getnextword(buf, max, ind, word);
	if(word_size!=3  || strncmp("GET", word, 3)!=0)
	{
		//Throw up error !
	}
	//int fwd=0;

	
	char *path;
	path=(char*)malloc(1000);

	word_size=getnextword(buf, max, ind, word);
	int k=0;
	/*while(k<word_size)
	{
		printf("%c", word[k]);
		k++;
	}

	printf("\n");*/
	//cout << "Checkpoint1" << endl;
	bool is_url=true;
	bool is_port=false;
	
	int hostlen;
	int d=0;
	//cout << "Checkpoint5" << endl;
	if(strncmp(word, "http://", 7)==0)
	{
		//cout << "Checkpoint8" << endl;
		//cout << strlen(word) << endl;
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
		}		//cout << strncpy(host, word+7, strlen(word)-7) << endl;
		//hostlen=word_size-7;
		//cout << "Checkpoint3" << endl;

	}
	else if (word[0]=='/')
	{
		//cout << "Checkpoint10" << endl;
		is_url=false;
		strncpy(path, word, word_size);
	}
	else
	{
		
	}
	//cout << "Checkpoint2" << endl;
	//ind+=word_size;
	
	word_size=getnextword(buf, max, ind, word);
	//ind+=word_size;
	//k=0;
	/*while(k<word_size)
	{
		printf("%c", word[k]);
		k++;
	}
*/
	//printf("\n");
	if(strncmp("HTTP/1.0", word, word_size)==0 || strncmp("HTTP/1.1", word, word_size)==0 || strncmp("HTTP/0.9", word, word_size)==0)
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
		if(strncmp("Host:", word, word_size)!=0)
		{
			//bad request
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
			//=word_size;
			//cout << host << endl;
		}
	}

	
	//int newind=0;
	

	strcat(newbuf, "GET ");
	strcat(newbuf, path);
	strcat(newbuf, " ");

	strcat(newbuf, "HTTP/1.0\r\nHost: ");
	strcat(newbuf, host);
	if(is_port)
	{
		strcat(newbuf, ":");
		strcat(newbuf, port);
		/*ind+=strlen(port);
		ind++;*/
		
	}
	strcat(newbuf, "\r\n");
	//strcat(newbuf, "test");
	//strcat(newbuf, "test2");
	//newind=strlen(newbuf);
	//strcat(newbuf, "strcat1");
	/*cout << newind << endl;
	k=0;
	while(k<newind)
	{
		printf("%c", newbuf[k]);
		k++;
	}
	printf("\n");
	char x, c, v;
	cin >> x >> c >> v;
	strcat(newbuf, host);
	newind+=hostlen;
	
	newind+=2;
	ind++;*/
	ind+=2;
	while(ind<max)
	{
		cout << "in here" << endl;
		//cout << ind << endl;
		if(buf[ind]=='\r' || buf[ind]=='\n')
		{
			strcat(newbuf, "\r\n");
			break;
		}
		word_size=getnextword(buf, max, ind, word);
		if(word[word_size-1]!=':')
		{
			//bad request
			
			
			
		
		}
		cout << "Word" << word << endl;
		strncat(newbuf, word, word_size);
		//newind+=word_size;
		strcat(newbuf, " ");
		//newind++;
		ind++;
		cout << word << endl;
		cout << ind << endl;
		//ind=0;
		while(!(buf[ind]=='\r' || buf[ind+1]=='\n'))
		{	//cout << buf[ind];
			strncat(newbuf, buf+ind, 1);
			//newind++;
			ind++;

		}
		strcat(newbuf, "\r\n");
		ind+=2;
	}
	strcat(newbuf, "\r\n");
	//strcat(newbuf, "strcat2");
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