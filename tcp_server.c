#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "practical.h"
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

typedef struct tcp_stats_struct {
   struct timespec endtime_spec;
   uint64_t  numBytesRcvd;  
} tcp_stats;

int HandleTCPClient(int clntSocket, char *buffer, tcp_stats *stats){ //, uint64_t *last_received) {	
	int counter=0;
  	// Receive message from client
  	ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
  	if (numBytesRcvd < 0)
    	sysmsg_exit("recv() failed");
    else
		counter += 1;

	uint64_t limit=(uint64_t)BUFSIZE*(uint64_t)ITERS;

	while (numBytesRcvd > 0) { // 0 indicates end of stream
		stats->numBytesRcvd += (uint64_t)numBytesRcvd;
		//printf("count:%d, %" PRIu64 "\n",counter,stats->numBytesRcvd);

		// TCP guarantees to deliver all packets, but there can be more?!
		if(stats->numBytesRcvd >= limit ){
			printf("count:%d",counter);
			break;
      	}
    	// See if there is more data to receive
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
		counter += 1;

    	if (numBytesRcvd < 0)
      		sysmsg_exit("recv() failed");
  	}
  	clock_gettime(CLOCK_REALTIME, &stats->endtime_spec);
		
	return counter;
}

int main(int argc, char *argv[]) {

	if (argc != 2) // Test for correct number of arguments
		usermsg_exit("Parameter(s)", "<Server Port>");

	in_port_t servPort = atoi(argv[1]); // First arg:  local port

	tcp_stats tcp_items;
	char* buffer;

	int clntSock;
	// Create socket for incoming connections
	int servSock; // Socket descriptor for server
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		sysmsg_exit("socket() failed");

	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(servPort);          // Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		sysmsg_exit("bind() failed");

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0)
		sysmsg_exit("listen() failed");

	int flag = 1;
	if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int)) == -1) { 
		perror("setsockopt"); 
		exit(1); 
	}

	uint64_t num_bytes = 0;
	uint64_t last_received=0;

	memset(&buffer, 0, sizeof(buffer));
	buffer= (char *)malloc(SERVER_BUFSIZE * sizeof(char));

	while(1) { // Run forever
		struct sockaddr_in clntAddr; // Client address
		// Set length of client address structure (in-out parameter)
		socklen_t clntAddrLen = sizeof(clntAddr);	

		// Wait for a client to connect
		clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
		if (clntSock < 0)
	  		sysmsg_exit("accept() failed");

		// clntSock is connected to a client!
		char clntName[INET_ADDRSTRLEN]; // String to contain client address
		if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
			sizeof(clntName)) != NULL)
	  		printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
		else
	  		puts("Unable to get client address");

		HandleTCPClient(clntSock, buffer, &tcp_items); 

		if(tcp_items.numBytesRcvd >= BUFSIZE*ITERS)
			break;
		else
			printf("-------\nThis round bufferzie*iterations: %" PRIu32 "\n-------\n", BUFSIZE*ITERS);
			printf("-------\nThis round rx bytes: %" PRIu64 "\n-------\n", tcp_items.numBytesRcvd);
	}


	/*num_bytes=tcp_items.numBytesRcvd;
	size_t length = snprintf( NULL, 0, "%" PRIu64, num_bytes);
	char* num_bytes_str = (char *) malloc( length + 1 );
	snprintf( num_bytes_str, length + 1, "%" PRIu64, num_bytes);
	printf("total bytes:%s\n",num_bytes_str);
	
	ssize_t numBytesSent = send(clntSock, num_bytes_str, sizeof(num_bytes_str), 0);
	if (numBytesSent < 0)
		sysmsg_exit("send() failed");
	free(num_bytes_str);*/

	puts("closing clntSock then");
    close(clntSock); // Close client socket
    close(servSock);

}
