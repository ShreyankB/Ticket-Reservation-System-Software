#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "./header/server/server_ops.h"

#define PORT 9912

int main()
{
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		printf("socket creation failed\n");
		exit(0);
	}
	int optval = 1;
	int optlen = sizeof(optval);
	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, optlen) == -1)
	{
		printf("set socket options failed\n");
		exit(0);
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(PORT);

	if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
	{
		printf("binding port failed\n");
		exit(0);
	}
	if (listen(sd, 100) == -1)
	{
		printf("listen failed\n");
		exit(0);
	}
	while (1)
	{
		int cd;
		if ((cd = accept(sd, (struct sockaddr *)NULL, NULL)) == -1)
		{
			printf("connection error\n");
			exit(0);
		}
		if (fork() == 0)
			start(cd);
	}

	close(sd);
	return 0;
}
