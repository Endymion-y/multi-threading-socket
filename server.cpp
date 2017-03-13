#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <thread>
#include <atomic>
#include <boost/lockfree/queue.hpp>

const int QUEUELEN = 20;
const int BUFLEN = 1024;
const char* ADDR = "127.0.0.1";
const int PORT = 50051;

const int QUEUESIZE = 128;
const int recv_thread_count = 4;
const int output_thread_count = 2;

int clnt_sock;
boost::lockfree::queue<int> q(QUEUESIZE);

std::atomic<bool> done(false);    // Indicates whether all data received
std::atomic<int> sum(0);

void do_recv(){
    char buffer[BUFLEN];
    int offset = 0;
    int len;
    int val;
    int tar = sizeof(val);
    while ((len = read(clnt_sock, buffer + offset, tar - offset)) > 0){
        offset += len;
        if (offset == tar){
            // Received an int
            val = *((int*)buffer);
            // q.push(val);
            while (!(q.push(val)))
                /* Waiting */ ;
            // Restart
            offset = 0;
        }
    }
    if (len < 0){
        std::cerr << "Failed to read data from client" << std::endl;
        return;
    }
}

void do_output(){
    int val;
    while (!done){
        // Data enqueuing
        while (q.pop(val)){
            std::cout << val << ' ';
            sum += val;
        }
    }
    // No more data enqueuing
    while (q.pop(val)){
        std::cout << val << ' ';
        sum += val;
    }
}

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

    while (true){
        // Accept a request
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        // int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        clnt_sock = accept(serv_sock, (struct sockaddr*)(&clnt_addr), &clnt_addr_size);

        std::thread recv_threads[recv_thread_count];
        for (int i = 0; i < recv_thread_count; i++){
            recv_threads[i] = std::thread(do_recv);
        }

        std::thread output_threads[output_thread_count];
        for (int i = 0; i < output_thread_count; i++){
            output_threads[i] = std::thread(do_output);
        }

        for (int i = 0; i < recv_thread_count; i++){
            recv_threads[i].join();
        }
        done = true;
        std::cout << "Received from client: " << std::endl;
        for (int i = 0; i < output_thread_count; i++){
            output_threads[i].join();
        }

        std::cout << std::endl;
        std::cout << "sum = " << sum << std::endl;
       
        // Close the sockets
        close(clnt_sock);
    }

	
	close(serv_sock);
	return 0;
}