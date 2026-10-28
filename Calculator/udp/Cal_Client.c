#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int main(){
    int s,a,b,stop;
    char op, msg[100];
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    
    s = socket(AF_INET,SOCK_DGRAM,0);
    
    server.sin_family = AF_INET;
    server.sin_port =htons(8000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    do{
      printf("Enter first Number: ");
      scanf("%d",&a);
      
      printf("Enter operator (+ - * /): ");
      scanf(" %c",&op);
      
      printf("Enter second Number: ");
      scanf("%d",&b);
      
      sprintf(msg,"%d %c %d",a,op,b);
      sendto(s,msg,strlen(msg)+1,0,(struct sockaddr *)&server,len);
      recvfrom(s,msg,sizeof(msg),0,(struct sockaddr *)&server, &len);
      
      printf("Result = %s\n",msg);
      
      printf("Enter 1 to continue, 0 to Stop: ");
      scanf("%d",&stop);
      
      if(stop == 0){
          strcpy(msg, "exit");
          sendto(s,msg,strlen(msg)+1,0,(struct sockaddr *)&server,len);
      }
    }while(stop);
    
    close(s);
    return 0;
}