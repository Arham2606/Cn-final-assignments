#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

int main()
{
    int serverfd = socket(AF_INET , SOCK_STREAM , 0);
    if(serverfd==-1){
        perror("socket");
        return 1;
    }

    printf("Server socket descriptor made successfuly \n ");

    struct sockaddr_in address = {0};
    address.sin_family=AF_INET;
    address.sin_port=htons(5000);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("Server addres configure successfully \n ");

    if(bind(serverfd , (struct sockaddr *)&address , sizeof(address)) ==-1){
        perror("bind");
        close(serverfd);
        return 1;
    }

    printf("Local port bind successfully \n ");

    if(listen(serverfd , 5)==-1){
        perror("listen");
        close(serverfd);
        return 1;
    }

    printf("Server is listening for the client \n");

    int clientfd = accept(serverfd ,  NULL , NULL);
    if(clientfd ==-1){
        perror("accept");
        close(serverfd);
        return 1;
    }

    printf("Client Connected \n");

    char buffer[1024];
    ssize_t recived = recv(clientfd , buffer , sizeof(buffer) - 1 , 0);
    if(recived<=0){
        if(recived ==-1){
            perror("recv");
        }
        else{
            printf("Client disconnected without sending data \n");
        }

        close(clientfd);
        close(serverfd);
        return 1;
    }

    buffer[recived] = '\0';

    double num1 ,num2;
    char operator;


    if(scanf(buffer , "%lf , %lf , %c" , &num1 , &num2 ,  &operator) != 3){
        const char *error = "Invalid request";
        send(clientfd , error, strlen(error) , 0);
        close(clientfd);
        close(serverfd);
        return 1;
    }

    printf("Recived request %f , %c , %f " , num1 , operator ,num2 );
    
    double result;
    switch(operator){
        case '+':
        result = num1+num2;
        break;
        case '-':
        result = num1-num2;
        break;
        case '*':
        result =num1*num2;
        break;
        case '/':
        if(num2==0){
            const char *error = "Cannot divide any number by 0 \n";
            send(clientfd , error , strlen(error), 0);
            close(clientfd);
            close(serverfd);
            return 1;
        }

        result = num1/num2;
        break;

        default:{
            const char *invalid ="Invalid operation";
            send(clientfd , invalid , strlen(invalid) ,0);
            close(clientfd);
            close(serverfd);
            return 1;
        }

        char response[1024];
        sprintf(response , "%.2f" , result);
        send(clientfd , response , strlen(response) , 0);
        close(clientfd);
        close(serverfd);
    }
    return 0;
}