Assignment 2 : HTTP Proxy
Simrat Singh Chhabra, 11010165

running the proxy:
./proxy <port>
where <port> is any port no.


To test the proxy, enter the following in a separate terminal/computer:
telnet <proxyaddress> <port>
(where <proxyaddress> is the ip address of the maching on which the proxy is running and <port> is its port.)
GET http://iitg.ernet.in HTTP/1.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Encoding: gzip,deflate,sdch

(output of the above input is in output1.txt)

GET / HTTP/1.0
Host: intranet.iitg.ernet.in:80

(output of the above input is in output2.txt)

GET / HTTP/1.0

Output : Error 400 : Bad Request

POST /path/script.cgi HTTP/1.0

Output : Error 501 : Not Implemented



Notes on the program:
The functions have been commented in proxy.cpp

get_req_from_client() gets the raw request from the client. It takes care that the request has to be recieved until two consecutive <CRLF> are seen.

parse_req() parses the raw request and checks for its syntactic correction. It also parses the host, the path and the port(default is 80) and constructs the new request to be forwarded to the server.


Sending the correctly formatted request to the server is done in the main function itself. The program waits for a reply and once it gets it, forwards it, as it is, to the waiting client. The connection with the client is then terminated and if the client wants to send another request it has to connect again.

Threading has not been implemented, so any connection requests which come while a client is already connected are put in a queue and served when the proxy becomes free.

