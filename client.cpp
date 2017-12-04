#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUFSIZ = 1024;
const char* ADDR = "127.0.0.1";
const int PORT = 50051;

int sock;

int main(){
	// Create a socket
	// domain, type, protocol(0 means default protocol determined by type)
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// Send request
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;  //IPv4
	serv_addr.sin_addr.s_addr = inet_addr(ADDR);  
	serv_addr.sin_port = htons(PORT); 
	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	char buf[BUFSIZ];
	memset(buf, 0xFF, BUFSIZ);
	while (true){
		int ret = send(sock, buf, BUFSIZ);
		if (ret < 0) break;
	}

	close(sock);
	return 0;
}
