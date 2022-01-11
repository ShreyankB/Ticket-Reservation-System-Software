#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "./header/client/login_signup.h"

#define PORT 9912
#define IP "127.0.0.1"

int main()
{
	char *ip = IP;
	struct sockaddr_in ca;
	int sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd == -1)
	{
		printf("socket creation failed\n");
		exit(0);
	}

	ca.sin_family = AF_INET;
	ca.sin_port = htons(PORT);
	ca.sin_addr.s_addr = inet_addr(ip);

	if (connect(sd, (struct sockaddr *)&ca, sizeof(ca)) == -1)
	{
		printf("connect failed\n");
		exit(0);
	}
	printf("connection established\n");

	while (client_ops(sd) != 3)
		;

	close(sd);
	return 0;
}
