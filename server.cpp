#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int BUFSIZ = 1024;
const char* ADDR = "127.0.0.1";
const int PORT = 50051;

int clnt_sock;

int main(){
	// Create a socket
	int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Bind the socket with IP and port
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET;  // Use Ipv4 protocol
	serv_addr.sin_addr.s_addr = inet_addr(ADDR);
	serv_addr.sin_port = htons(PORT); 
	bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	// Start listening, waiting for requests from clients
	listen(serv_sock, QUEUELEN);
	std::cout << "Server listening on " << ADDR << ":" << PORT << std::endl;

	// Accept a request
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	// int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	clnt_sock = accept(serv_sock, (struct sockaddr*)(&clnt_addr), &clnt_addr_size);

	char buf[BUFSIZ];
	int off = 0;
	int sum = 0;
	while (true){
		int ret = recv(clnt_sock, buf + off, BUFSIZ - off, 0);
		if (ret < 0) break;
		off += ret;
		if (off >= BUFSIZ) {
			std::cout << "Message received" << endl;
			off = 0;
		}
	}

	// Close the sockets
	close(clnt_sock);

	
	close(serv_sock);
	return 0;
}
