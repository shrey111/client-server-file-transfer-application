#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[]){//E.g., 1, client
  char message[10000];
  char message2[10000];
  char recievecommands[25];
  int bytesreceived = 0;
  int server, portNumber;
  //memset(message,'0',sizeof(message));
  socklen_t len;
  struct sockaddr_in servAdd;
  if(argc != 3){
    printf("Call model:%s <IP> <Port#>\n",argv[0]);
    exit(0);
  }
  if ((server=socket(AF_INET,SOCK_STREAM,0))<0){
    fprintf(stderr, "Cannot create socket\n");
    exit(1);
  }
  servAdd.sin_family = AF_INET;

  sscanf(argv[2], "%d", &portNumber);

  servAdd.sin_port = htons((uint16_t)portNumber);

  //convert IPv4 and IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, argv[1],&servAdd.sin_addr) < 0){
    fprintf(stderr, " inet_pton() has failed\n");
    exit(2);
  }

  if(connect(server, (struct sockaddr *) &servAdd,sizeof(servAdd))<0){
    fprintf(stderr, "connect() failed, exiting\n");
    exit(3);
  }
  while(1){

    printf("Command: \t");
    scanf("%[^\n]%*c", &message[0]);
    strncpy(message2,message,sizeof(message));
    int argc=0;
    char *command[5];
    char *sendcontent='\0';
    //fprintf(stderr, "%s\n", message);
    char *tkn = strtok(message," \t\n"); //separate using delimiter space
    while(tkn!= NULL)
    {
          command[argc] = tkn;
          tkn = strtok(NULL," \t\n");
          argc++;
    }
    command[argc]='\0';

    if(strcmp(command[0], "quit") == 0){
  	    send(server, message2, sizeof(message2), 0);
  	}else if(strcmp(command[0], "get") == 0 || strcmp(command[0], "put") == 0){
  	  bzero(recievecommands, sizeof(recievecommands));
      strncpy(recievecommands,command[1],strlen(command[1]));
      send(server, message2, sizeof(message2), 0);
  	}else{
      printf("Command Not Found\n");
      bzero(message, sizeof(message));
      continue;
    }

    if(strcmp(command[0], "quit") == 0){
      close(server);
      printf("Connecting terminated.\n");
      exit(1);
    }

    if((bytesreceived = recv(server, message, sizeof(message), 0)) > 0){

    	if(strcmp(message, "File Uploaded") == 0){
    		printf("Server: \t%s\n", message);
    		bzero(message, sizeof(message));
    	}else if(strcmp(message, "End of file") == 0){
        printf("Server: \t%s\n", message);
        bzero(message, sizeof(message));
        }else if(strcmp(message, "Client Sent") == 0){
		   	printf("Server: \t%s\n", recievecommands);
		    FILE *fp1 = fopen(recievecommands,"r");
		    if(fp1==NULL){
		     printf("File open error\n");
		     continue;
		    }else{
		          
		            fseek(fp1,0L,SEEK_END);
		            int curr_pos = ftell(fp1);
		            fseek(fp1,0L,SEEK_SET);
		            sendcontent = malloc(curr_pos+1);
		          
		          int nread = fread(sendcontent,1,curr_pos,fp1);
		          if(nread>=curr_pos){
		            printf("sending\n");
		            send(server,sendcontent,nread,0);
		            //bzero(sendcontent, sizeof(sendcontent));
		            fprintf(stderr,"File Uploaded\n");
		            //fwrite(filecontent , 1 , nread , fp2 );
		          }

		          if(nread<256){
		            if(feof(fp1)){
		              printf("End of File\n");
		            }
		            if(ferror(fp1)){
		              printf("Error");
		            }
		          }
		    }
		    fclose(fp1);
		    bzero(message, sizeof(message));
      }else if(strcmp(message, "File Not Found") == 0){
    		printf("Server: \t%s\n", message);
    		bzero(message, sizeof(message));
    	}else if(strcmp(message, "Command Not Found") == 0){
    		printf("Server: \t%s\n", message);
    		bzero(message, sizeof(message));
    	}else{
    	  	printf("Bytes received %d\n", bytesreceived);
            FILE *fp = fopen(recievecommands,"w+");
		    if(fp==NULL){
		      printf("Error opening file\n");
		    }
		    fwrite(message , 1 , bytesreceived , fp );
	      	printf("File downloaded\n");
	      	bzero(message, sizeof(message));		
        	fclose(fp);
    	}

      
    }


  }
}