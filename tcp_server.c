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
//#include <netdb.h>

typedef struct tcp_stats_struct {
	struct timespec endtime_spec;
	uint64_t  numBytesRcvd;  
} tcp_stats;



int HandleTCPClient(int clntSocket, char *buffer, tcp_stats *stats){
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
		
	return 0;
}

int main(int argc, char *argv[]) {

	if (argc != 2) // Test for correct number of arguments
		usermsg_exit("Parameter(s)", "<Server Port>");

	in_port_t servPort = atoi(argv[1]); // First arg:  local port


	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number
	int listener;     // listening socket descriptor
	int newfd;        // newly accept()ed socket descriptor
	struct sockaddr_storage clntAddr; //remoteaddr; // client address
	socklen_t addrlen;

	uint64_t nbytes = 0;
	int i, j, rv;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// Create socket for incoming connections
	int clntSock;
	char* buffer;
	

	if ((listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		sysmsg_exit("socket() failed");

	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(servPort);          // Local port


	// Bind to the local address
	if (bind(listener, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		sysmsg_exit("bind() failed");

	// Mark the socket so it will listen for incoming connections
	if (listen(listener, 10) < 0)
		sysmsg_exit("listen() failed");

	int flag = 1;
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int)) == -1) { 
		perror("setsockopt"); 
		exit(1); 
	}

	memset(&buffer, 0, sizeof(buffer));
	buffer= (char *)malloc(SERVER_BUFSIZE * sizeof(char));

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	while(1) { // Run forever
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}


		for(int sock_id = 0; sock_id <= fdmax; sock_id++) {
			if (FD_ISSET(sock_id, &read_fds)) { // we got one!!
				if (sock_id == listener) { // handle new connections
					addrlen = sizeof(clntAddr);
					newfd = accept(listener, (struct sockaddr *)&clntAddr, &addrlen);

					if (newfd < 0) {
						sysmsg_exit("accept() failed");
					} 
					else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}

						char clntName[INET_ADDRSTRLEN]; // String to contain client address

						if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&clntAddr)->sin_addr), clntName, sizeof(clntName) ) != NULL)
							printf("Handling client %s/%d, socket id %d\n", clntName, ntohs( ((struct sockaddr_in *)&clntAddr)->sin_port ), newfd);
						else
							puts("Unable to get client address");
					}
				}
				else{
					tcp_stats tcp_items;
					HandleTCPClient(sock_id, buffer, &tcp_items);
					if(tcp_items.numBytesRcvd >= BUFSIZE*ITERS)
						nbytes+=tcp_items.numBytesRcvd;
					else
						printf("-------\nThis round bufferzie*iterations: %" PRIu32 "\n-------\n", BUFSIZE*ITERS);
						printf("-------\nThis round rx bytes: %" PRIu64 "\n-------\n", tcp_items.numBytesRcvd);
					close(sock_id); // close sock
					FD_CLR(sock_id, &master); // remove from master set
				}
			}    
		}

		if( nbytes>=(fdmax-1)*BUFSIZE*ITERS )
			break;         
	}

	puts("closing clntSock then");
	close(listener);

}
