#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/tcp.h>
#include "practical.h"
#include <fcntl.h> // for non-blocking

typedef struct tcp_stats_struct {
   struct timespec endtime_spec;
   uint64_t  numBytesRcvd;  
} tcp_stats;

int main(int argc, char *argv[]) {

	if (argc !=  4) // Test for correct number of arguments
    	usermsg_exit("Parameter(s)",
        	"<Server Address> <Echo Word> [<Server Port>]");

  	char *servIP = argv[1];     // First arg: server IP address (dotted quad)
  	char *echoString = argv[2]; // Second arg: string to echo

  	int iter=0;
  	uint64_t txbytes = 0;

  	struct timespec starttime_spec, endtime_spec; // 16 bytes
	struct tcp_info sock_tcpinfo;

	char sendbuffer[BUFSIZE];
	memset(&sendbuffer, 0, sizeof(sendbuffer));

  	// Third arg (optional): server port (numeric).  7 is well-known echo port
  	in_port_t servPort = (argc == 4) ? atoi(argv[3]) : 7;

  	// Create a reliable, stream socket using TCP
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
		sysmsg_exit("socket() failed");

	//fcntl(sock, F_SETFL, O_NONBLOCK);

  	// Construct the server address structure
	struct sockaddr_in servAddr;            // Server address
	memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
  	servAddr.sin_family = AF_INET;          // IPv4 address family

  	// Convert address
	int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
	if (rtnVal == 0)
		usermsg_exit("inet_pton() failed", "invalid address string");
  	else if (rtnVal < 0)
    	sysmsg_exit("inet_pton() failed");

  	servAddr.sin_port = htons(servPort);    // Server port

	// Establish the connection to the echo server
	if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		sysmsg_exit("connect() failed");

	socklen_t tcp_info_length = sizeof(struct tcp_info);

    if (getsockopt(sock, IPPROTO_TCP, TCP_INFO, (void *)&sock_tcpinfo, &tcp_info_length) < 0)
    	usermsg_exit("getsockopt - %s", strerror(errno));
    else // This only works on Linux or FreeBSD or NetBSD systems with the help inside tcp.h 
    	printf("tcp rtt: %u ms\n",sock_tcpinfo.tcpi_rtt); // in usecs

	clock_gettime(CLOCK_REALTIME, &starttime_spec);
	for(iter=0;iter<ITERS;iter++){

		//api ref: ssize_t send(int sockfd, const void *buf, size_t len, int flags);
		ssize_t numBytes = send(sock, sendbuffer, sizeof(sendbuffer), 0);

		//if (numBytes < 0)
		//	sysmsg_exit("send() failed");
  		//else if (numBytes != sizeof(sendbuffer))
			//usermsg_exit("send()", "sent unexpected number of bytes");

		txbytes+=numBytes;
		//printf("tx bytes:%ld\n", numBytes);

	}
	clock_gettime(CLOCK_REALTIME, &endtime_spec);
	printf("tx total:%lu bytes\n", txbytes);



    char buffer[BUFSIZE];
	memset(&buffer, 0, sizeof(buffer)); 

    /*
	unsigned int rcvbytes = 0; // Count of total bytes received
	fputs("Received bytes ", stdout);     // Setup to print the echoed string
	rcvbytes = recv(sock, buffer, BUFSIZE - 1, 0);

	if (rcvbytes < 0)
		sysmsg_exit("recv() failed");
	else if (rcvbytes == 0)
		usermsg_exit("recv()", "connection closed prematurely");

	buffer[rcvbytes]='\0';
	fputs(buffer,stdout);      // Print the echo buffer
	*/

	uint64_t starttime=(1000000) * (uint64_t)starttime_spec.tv_sec + (uint64_t)starttime_spec.tv_nsec/1000;
	uint64_t endtime=  (1000000) * (uint64_t)endtime_spec.tv_sec + (uint64_t)endtime_spec.tv_nsec/1000;
	
    //printf("time diff: %" PRIu64"\n", endtime-starttime);
    double rate= 8.0 *(double)txbytes/(double) (endtime-starttime);

    printf("\nrate: %f Mbps\nbuffer size: %"PRIu32"\n",rate,BUFSIZE);

	fputc('\n', stdout); // Print a final linefeed
	close(sock);
  	exit(0);
}
