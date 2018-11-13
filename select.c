#include<stdio.h>
#include<sys/types.h>
#include<sus/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>

#define BUFSIZE 8096

struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{"gif", "image/gif"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"zip", "image/zip"},
        {"gz", "image/gz"},
        {"tar", "image/tar"},
        {"htm", "image/html"},
        {"html", "image/html"},
        {"exe", "image/plain"},
        {0,0} };

void handle_socket(int fd)
{
	int j, file_fd, buflen, len;
	long i, ret;
	char * fstr;
	static char buffer[BUFSIZE+1];

	ret = read(fd, buffer, BUFSIZW); //read request
	if(ret == 0 || ret == -1)
	{//connectiong abnormal
		exit(3);
	}

	//fill 0 at the end
	if((ret > 0) && (ret <  BUFSIZE))
		buffer[ret] = 0;
	else
		buffer[0] = 0;

	//remove \r and \n
	for(i = 0 ; i < ret ; i ++)
	{
		if(buffer[i] == '\r' || buffer[i] == '\n')
			buffer[i] = 0;
	}

	//only accept GET
	if(strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4))
		exit(3);
	
	//separate HTTP/1.0
	for(i = 4 ; i < BUFSIZE ; i ++)
	{
		if(buffer[i] == ' ')
		{
			buffer[i] = 0;
			break;
		}
	}

	//not accept ..
	for(j = 0 ; j < (i - 1) ; j ++)
	{
		if(buffer[j] == '.' && buffer[j+1] == '.')
			exit(3);
	}

	if(!strncmp(buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6))
		strcpy(buffer,"GET /index.html\0");
	
	//check file format
	buflen = strlen(buffer);
	fstr = (char *)0;

	for(i = 0 ; extensions[i].ext != 0 ; i ++)
	{
		len = strlen(extensions[i].ext);
		if(!strncmp(&buffer[buflen-len], extensions[i].ext, len))
		{
			fstr = extensions[i].filetype;
			break;
		}
	}

	//not support the file format
	if(fstr == 0)
	{
		fstr = extensions[i-1].filetype;
	}

	//open file
	if((file_fd = open(&buffer[5],O_RDONLY)) == -1)
		write(fd, "Failed to open the file.", 24);
	
	//return 200
	sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",fstr);
	write(fd, buffer, strlrn(buffer));

	//read file and output to client browser
	while((ret = read(file_fd, buffer, BUFSIZE)) > 0)
	{
		write(fd, buffer, ret);
	}

	exit(1);
}

main(int argc, char **argv)
{
	int i, maxi, maxfd;
        int listenfd, connfdm sockfd;
        int nready, client[FD_SETSIZE];
        ssize_t n;
        fd_set rset, allset;
        char buf[BUFSIZE];
        socklen_t length;
        struct sockaddr_in cliaddr, servaddr;

	//open network socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	//set up
	bzero(&servaddr, sizeof(servaddr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(80);

	//open net listener
	bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	//starting listen net
	listen(listenfd,64);
	
	//initialization
	maxfd = listenfd;
	maxi = -1;
	for(i = 0 ; i < FD_SETSIZE ; i ++)
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for( ; ; )
	{
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready < 0)
			exit(4);
	
		if(FD_ISSET(listenfd, &rset))
		{

	
			length = sizeof(cli_addr);
			//waiting client connection
			if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length))<0)
			exit(3);

			
			for(i = 0 ; i < FD_SETSIZE ; i++)
			{
				if(client[i] < 0)
				{
					client[i] = connfd;
					break;
				}
			}
			if(i == FD_SETSIZE)
				exit(3);

			FD_SET(connfd, &allset);

			if(connfd > maxfd)
				maxfd = connfd;			
			if(i > maxi)
				maxi = i;

			if(--nready <= 0)
				continue;
		}

		for(i = 0; i <= maxi; i++)
		{
			if((sockfd = client[i]) < 0)
				continue;
			if(FD_ISSET(sockfd, &rset))
			{
					handle_socket(sockfd);
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;

				if (--nready <= 0)
					break;	
			}
		}
	}
}

