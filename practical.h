#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <inttypes.h>

struct tcp_stats_struct;

static const uint32_t SERVER_BUFSIZE = 4096000;// 40 MB
static const uint32_t BUFSIZE = 4096000;  //4MB
static const uint32_t ITERS= 100000; // 1K
//static const int DIGITS =10;

// Handle error with user msg
void usermsg_exit(const char *msg, const char *detail);
// Handle error with sys msg
void sysmsg_exit(const char *msg);
// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);
// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);
// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);
// Handle new TCP client
int HandleTCPClient(int clntSocket, char *buffer, struct tcp_stats_struct *stats);
// Create and connect a new TCP client socket
int SetupTCPClientSocket(const char *server, const char *service);

/*enum sizeConstants {
  MAXSTRINGLENGTH = 128,
};*/


#endif // PRACTICAL_H_
