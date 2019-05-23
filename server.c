//Max Neviantsev 317497337
//Daniel Kovalevski 204363204

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
#include <sys/stat.h>
#include <dirent.h> 
#include <sys/time.h>
#include <sys/select.h> 

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20

int main(void)
{
	fd_set master;
	fd_set read_fds;
	int pid, num, new_File;
	char action[14], file_name[20];
	char* stop = "stop";
	long file_size, file_real_size;
	struct stat st;
	int listenS = socket(AF_INET, SOCK_STREAM, 0);
	if (listenS < 0)
	{
		perror("socket error");
		return 1;
	}
	struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	if (bind(listenS, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("bind error");
		return 1;
	}
	if (listen(listenS, QUEUE_LEN) < 0)
	{
		perror("listen error");
		return 1;
	}
	struct sockaddr_in clientIn;
	int clientInSize = sizeof clientIn;
	while (1){
		int newfd = accept(listenS, (struct sockaddr*)&clientIn, (socklen_t*)&clientInSize);
		if (newfd < 0){
			perror("accept error");
			return 1;
		}
		if((pid = fork()) == -1){
			close(newfd);
			continue;
		}
		else if(pid > 0){
			 close(newfd);
			 continue;
		}
		else if(pid == 0){
		memset(action, 0 , sizeof(action));
		if (recv(newfd, action, sizeof(action), 0) < 0){
			perror("recive");
			return 1;
		}
		num = atoi(action);
		struct dirent *de; 
		DIR *dr = opendir("."); 
		switch(num){

		  case 1:  

		    if (dr == NULL){ 
		        printf("Directory error" ); 
		        return 0; 
		    } 
		
		    while ((de = readdir(dr)) != NULL){

		    	strcpy(file_name, de->d_name);
		        if(send(newfd, &file_name, sizeof(file_name), 0) < 0)
		        {
		            perror("file name sending error");
		            return 1;
		        }
		        new_File = open(file_name, O_RDONLY);
		        if(new_File < 0){
		            perror("open");
		            return 1;
		        }
		        fstat(new_File, &st);
		        file_size = st.st_size;
		        if(send(newfd, &file_size, sizeof(file_size), 0) < 0){
		                perror("file size sending error");
		                return 1;
		        }
		    }
		    //strcpy(stop,"stop");
		    if(send(newfd, &stop, sizeof(stop), 0) < 0){
		            perror("end of sending error");
		            return 1;
		    }
		    closedir(dr); 
		    break;

		case 2:

		    memset(file_name, 0 , sizeof(file_name));

		    if (recv(newfd, &file_name, sizeof(file_name), 0) < 0){
		        perror("recive file name error");
		        return 1;
		    }

		    if (recv(newfd, &file_real_size, sizeof(file_real_size), 0) < 0){
		        perror("recive file size error");
		        return 1;
		    }

		    int count = 1;
		    int total = 0;
		    char* buffer = (char*) malloc(file_real_size);
		    memset(buffer, 0, file_real_size);

		    FD_ZERO(&master);
		    FD_ZERO(&read_fds);
		    FD_SET(0,&master);
		    FD_SET(newfd,&master);

		    while(count != 0){
		        read_fds = master;
		        if (select(newfd + 1,&read_fds,NULL,NULL,NULL) == -1){
		        perror("select:");
		        exit(1);
		        }

		        if (FD_ISSET(newfd, &read_fds)){
		        count = recv(newfd, buffer + total, file_real_size - total, 0);
		        total += count;
		        }
		    }

		    if(open(file_name,O_EXCL) != -1) // if doesn't exist
		    	printf(" %s alreay exists\n", file_name);

		    else{ // file writing
		        int new_File = open(file_name, O_RDWR|O_CREAT, 0666);
		        if(new_File < 0){
		            perror("open file error");
		            return 1;
		    	}
		        write(new_File, buffer, file_real_size);
		        printf("file uploaded\n");
		        close(new_File);
		    }

		    free(buffer);
		    close(newfd);
		    continue;
		}
		close(listenS);
		return 0;
   	 }
     }
}
       