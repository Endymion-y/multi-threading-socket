#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <thread>
#include <atomic>
#include <boost/lockfree/queue.hpp>

const int MAXLINE = 4096;
const int QUEUESIZE = 128;
const char* ADDR = "127.0.0.1";
const int PORT = 50051;

const int input_thread_count = 2;
const int send_thread_count = 4;

int sock;
boost::lockfree::queue<int> q(QUEUESIZE);

std::atomic<bool> done(false);	// Indicates whether input has done
std::atomic<int> send_data(1);
const int rdata = 10000;
const int iterations = rdata / input_thread_count;
// assert((iterations * input_thread_count == rdata));

void do_input(){
	for (int i = 1; i <= iterations; i++){
		int val = send_data++;
		while (!q.push(val))
			/* Waiting */ ;
	}
}

void do_send(){
	int val;
	char* p = (char*)(&val);
	while (!done){
		// Data enqueuing
		while (q.pop(val)){
			if ((write(sock, p, sizeof(val))) < 0){
				std::cerr << "Failed to send data to server" << std::endl;
				return;
			}
		}
	}
	// No more data enqueuing
	while (q.pop(val)){
		if ((write(sock, p, sizeof(val))) < 0){
			std::cerr << "Failed to send data to server" << std::endl;
			return;
		}
	}
}

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

	std::thread input_threads[input_thread_count];
	for (int i = 0; i < input_thread_count; i++){
		input_threads[i] = std::thread(do_input);
	}

	std::thread send_threads[send_thread_count];
	for (int i = 0; i < send_thread_count; i++){
		send_threads[i] = std::thread(do_send);
	}

	for (int i = 0; i < input_thread_count; i++){
		input_threads[i].join();
	}
	done = true;
	for (int i = 0; i < send_thread_count; i++){
		send_threads[i].join();
	}

	close(sock);
	return 0;
}