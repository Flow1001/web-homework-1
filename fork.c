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
	int i;
	int pid, listenfd, socketfd;
	size_t length;
	struct sockaddr_in cli_addr;
	struct sockaddr_in serv_addr;

	//let /tmp be the root of web
	if(chdir("/tmp") == -1)
	{
		printf("ERROR: Can't change to directory %s\n",argv[2]);
		exit(4);
	}

	//execute in background
	if(fork() != 0)
		return 0;

	//let parent don't have to wait child
	signal(SIGCLD, SIG_IGN);

	//open network socket
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit(3);

	//set up
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(80);

	//open net listener
	if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
		exit(3);

	//starting listen net
	if(listen(listenfd,64) < 0)
		exit(3);

	for( ; ; )
	{
		length = sizeof(cli_addr);
		//waiting client connection
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length))<0)
			exit(3);

		//child to process requirements
		if((pid = fork()) < 0)
		{
			exit(3);
		}
		else
		{
			if(pid == 0)
			{//child
				close(listenfd);
				handle_socket(socketfd);
			}
			else
			{//parent
				close(socketfd);
			}
		}
	}
}

}
