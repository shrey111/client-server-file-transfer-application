#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

void serviceClient(int);

int main(int argc, char *argv[]){

int sd, client, portNumber,status;
socklen_t len;
struct sockaddr_in servAdd;

if(argc != 2){
  fprintf(stderr,"Call model: %s <Port#>\n",argv[0]);
  exit(0);
}

//Create TCP/IP secure stream
if((sd = socket(AF_INET, SOCK_STREAM, 0))<0){
  fprintf(stderr, "Could not create socket\n");
  exit(1);
}
//set servAdd empty
memset(&servAdd, '\0', sizeof(servAdd));
//Using IPV4 protocol
servAdd.sin_family = AF_INET;
//set localhost IP address
servAdd.sin_addr.s_addr = INADDR_ANY;
//Assign port number with input from command line
sscanf(argv[1], "%d", &portNumber);
//Host to network conversions of multibyte integers for short
servAdd.sin_port = htons((uint16_t)portNumber);
//'bind' is called by server to bind their local IP and a well-known port number to a socket
if(bind(sd, (struct sockaddr *) &servAdd,sizeof(servAdd))<0){
  printf("Failed to bind\n");
  exit(1);
}

//A maximum queue of 5 client connections to listen
if(listen(sd, 5)==0){
  printf("Waiting for connections....\n");
}else{
  printf("Failed to listen.\n");
}

while(1){
  if((client=accept(sd,NULL,NULL))<0){
    exit(1);
  }

  printf("Got Client....\n");
  if(!fork()){
    close(sd);
    serviceClient(client);
  }
}
//wait(&status);

}
void serviceClient(int client){
  char buffer[100000];
  char message2[100000];
  char sendcommands[25];
  int bytesreceived = 0;
  while(1){
    bytesreceived = recv(client, buffer, 100000, 0);
    strncpy(message2,buffer,sizeof(buffer));
    int argc=0;
    char *command[5];
    //fprintf(stderr, "%s\n", buffer);
    char *tkn = strtok(buffer," \t\n"); //separate using delimiter space
    while(tkn!= NULL)
    {
          command[argc] = tkn;
          tkn = strtok(NULL," \t\n");
          argc++;
    }
    command[argc]='\0';


    if(strcmp(command[0], "quit") == 0){
      break;
    }else if(strcmp(command[0], "get") == 0){
       FILE *fp = fopen(command[1],"r");
       int curr_pos;
       if(fp==NULL){
        send(client, "File Not Found", sizeof("File Not Found"), 0);
        continue;
        //fprintf(stderr, "Not Found");
       }else{
          unsigned char *filecontent;
          fseek(fp,0,SEEK_END);
          curr_pos = ftell(fp);
          fseek(fp,0,SEEK_SET);
          filecontent = malloc(curr_pos+1);
          //bzero(filecontent, 256);
          int nread = fread(filecontent,1,curr_pos,fp);
          
          if(nread>0){
            printf("sending\n");
            send(client,filecontent,nread,0);
          }

          if(nread<curr_pos){
            if(feof(fp)){
              send(client,"End of file",sizeof("End of file"),0);
            }
            if(ferror(fp)){
              printf("Error");
            }
          }
       }
       fclose(fp);
    }else if(strcmp(command[0], "put") == 0){
      bzero(sendcommands, sizeof(sendcommands));
      strncpy(sendcommands,command[1],strlen(command[1]));    
      send(client,"Client Sent",sizeof("Client Sent"),0);
    }else{
       FILE *fp2 = fopen(sendcommands,"w+");
       if(fp2==NULL){
        printf("File open error");
       }else{
        if(bytesreceived > 0){
          printf("Bytes received %d\n", bytesreceived);
          fwrite(message2 , 1 , bytesreceived , fp2 );
          fprintf(stderr,"File Uploaded\n");
        }
       }
      fclose(fp2);
    }
 
    bzero(buffer, sizeof(buffer));
    bzero(message2, sizeof(message2));    
    
  }

}