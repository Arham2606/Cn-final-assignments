#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int main(){
    int s, a, b, result;
    char op, msg[100];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    s = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8000);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s,(struct sockaddr *)&server,sizeof(server));
    printf("Server is waiting...\n");
    
    while(1){
        recvfrom(s,msg,sizeof(msg),0,(struct sockaddr *)&client, &len);
        
        if(strcmp(msg,"exit")==0){
          break;
        }
        sscanf(msg,"%d %c %d",&a,&op,&b);
        
        switch(op){
            case '+' :
                result = a+b;
                break;
                
            case '-' :
                result = a-b;
                break;
                
            case '*' :
                result = a*b;
                break;
                
            case '/' :
              if(b==0){
                strcpy(msg,"Cannot divide by zero");
                sendto(s,msg,strlen(msg)+1,0,(struct sockaddr *)&client,len);
                continue;
              }
                result = a/b;
                break;
                
            default:
                strcpy(msg,"Invalid operator");
                sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&client,len);
                continue;
            }
          sprintf(msg, "%d", result);
          sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&client,len);
          printf("Result sent: %s\n",msg);
        }
      close(s);
      return 0;
}