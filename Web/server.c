#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define MAXLINE 4096

#define errout(str, ...) {\
	fprintf(stderr, str "\n", ## __VA_ARGS__);\
	exit(-1);\
}

#define realerr(str, ...) {\
	fprintf(stderr, str "\nError Code: %d\nError message: %s\n", ## __VA_ARGS__, errno, strerror(errno));\
	exit(-1);\
}

void myprint(char const *);

int main(int argc, char **argv) {
	long SERVER_PORT = atol("PORT");
	printf("%d\n", SERVER_PORT);

	int listenfd, connfd, n;
	struct sockaddr_in servaddr;
	uint8_t sendline[MAXLINE + 1], recvline[MAXLINE + 1];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		realerr("Error while creating the socket");

	memset(&servaddr, 0, sizeof servaddr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof servaddr) == -1)
		realerr("Bind error");

	if (listen(listenfd, 10) == -1)
		realerr("Listen error");

	for (;;) {
		struct sockaddr_in addr;
		socklen_t addr_len;

		printf("Waiting for connection on port %d\n", SERVER_PORT);
		fflush(stdout);

		if ((connfd = accept(listenfd, NULL, NULL)) == -1)
			realerr("Accept failed");

		memset(recvline, 0, MAXLINE + 1);
		while ((n = read(connfd, recvline, MAXLINE)) > 0) {
			myprint(recvline);

			if (n > 4 && recvline[n - 1] == '\n' && recvline[n - 2] == '\r' && recvline[n - 3] == '\n' && recvline[n - 4] == '\r')
				break;

			memset(recvline, 0, MAXLINE + 1);
		}

		if (n == -1)
			realerr("Read error");

		sprintf(sendline, "HTTP/1.0 200 OK\r\n\r\nHello World!");

		if (write(connfd, sendline, strlen(sendline)) == -1)
			realerr("Write error");

		if (close(connfd) == -1)
			realerr("Close failed");
	}

	return 0;
}

void myprint(char const *str) {
	while (*str) {
		if (*str == '\r')
			printf("\\r");
		else if (*str == '\n')
			printf("\\n");
		else if (*str == '\v')
			printf("\\v");
		else if (*str == '\a')
			printf("\\b");
		else if (*str == '\b')
			printf("\\b");
		else
			printf("%c", *str);

		str++;
	}

	printf("\n");
}
