#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h> 
#include <sys/stat.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001

int main(int argc, char *argv[])
{
	if((argc != 3) && atoi(argv[1]) != 1) {
		printf("insert 2 parameters");
		return 1;
	}

	char file_name[20], str[20];
	int new_File , num, n_read, n_write;
	long file_size, file_real_size;
	struct stat st;
	int sock = socket(AF_INET, SOCK_STREAM, 0), nrecv;
	struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	if (connect(sock, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("connection error");
		return 1;
	}

	memset(file_name, 0 , sizeof(file_name));
	memset(str, 0 , sizeof(str));
	strcpy(str,argv[1]);

	if (send(sock, str, sizeof(str), 0) < 0)
	{
		perror("sending error in statement number");
		return 1;
	}
	num = atoi(str);
	switch (num){

		case 1:
		while(strcmp(file_name,"stop")!=0){
			if (recv(sock, &file_name, sizeof(file_name), 0) < 0)
			{
				perror("recive");
				return 1;
			}

			if(strcmp(file_name,"stop")==0)
				break;			
			// get the file size
			if (recv(sock, &file_real_size, sizeof(file_real_size), 0) < 0)
			{
				perror("recive");
				return 1;
			}
			printf("%s - %ld\n", file_name,file_real_size);
		}
		close(sock);
		return 0;

	case 2:
		memset(file_name, 0 , sizeof(file_name));
		strcpy(file_name,argv[2]);

		if(open(file_name,O_EXCL) == -1) // if exist
			printf("%s does not exist\n", file_name);
			
		if(send(sock, &file_name, sizeof(file_name), 0) < 0){
			perror("send1");
			return 1;
		}

		new_File = open(file_name, O_RDONLY);
		if(new_File < 0){
			perror("open");
			return 1;
		}
		// send the file size
		fstat(new_File, &st);
		file_size = st.st_size;
		if(send(sock, &file_size, sizeof(file_size), 0) < 0){
			perror("send1");
			return 1;
		}

		char* buf = (char*) malloc(file_size + 1);
		memset(buf, 0 , file_size + 1);
		n_read = 1;
		int k;
		while(n_read != 0){
			n_read = read(new_File, buf , file_size + 1);

			for(int i = 0; i < n_read; i += n_write)
				n_write = send(sock, buf + i, n_read - i , 0);
		}
		free(buf);
		close(sock);
	}
	close(sock);
	return 0;
}		
