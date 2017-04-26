//
//  client.cpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/19/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr- utility function prototypes
#include <unistd.h> // write
#include <netinet/in.h> //struct sockaddr_in; byte ordering macros
#include <sys/types.h> //prerequisite typedefs
#include <netdb.h> //nw info lookup for prototypes and structures
#include <stdlib.h> //namespace std
#include <errno.h> //names for 'erno' values (error numbers)
#include <signal.h>
#include <iostream> //namespace std


using namespace std;


int MAX = 1080;


void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}



int main(int argc , char *argv[]){
    
    int sock, status;
    struct addrinfo hints, *servinfo, *p;
    char str_ip[INET6_ADDRSTRLEN];
    string portnum, rbuf;
    char rcv_buf[MAX], send_buf[MAX];


    //initial argument check to see if all are there
    if (argc != 3) {
        fprintf(stderr,"usage: hostname or port\n");
        exit(1);
    }
    
    //creates TCP socket & if socket value < 0, writes error.
    if((sock = socket(AF_INET, SOCK_STREAM, 0))< 0){
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    cout << "Socket created\n";
    
    
    //memory allocation
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; //TCP stream socket

    //takes arguments from command line for getaddrinfo()
    
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    
    
    // loop through results & bind to first
    for(p = servinfo; p != NULL; p = p->ai_next) {

        //creates socket, else gives error
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client error: socket");
            continue;
        }
        //tries to connect, if it cannot, closes socket & gives error
        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("client error: connect");
            continue;
        }
        break; 
    }
    
    //exiting the loop means the client failed to connect
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    
    //should convert IPv4 and IPv6 addresses from binary to text form, stores in var s
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), str_ip, sizeof str_ip);
    printf("client: connecting to %s\n", str_ip);
    
    freeaddrinfo(servinfo);
    
    
    
    //receive initial print statement to prompt for 'helo' command
    recv(sock, rcv_buf, MAX, 0);
    cout << rcv_buf;
    //clear buffer
    memset(rcv_buf, 0, MAX);


    
    
    //infinite unless entering quit
    while (fgets(send_buf, MAX, stdin) != NULL){
        
       if (send(sock, send_buf, MAX, 0) == -1){
                printf("%s\n", "Message failed to send to server.");
                
        } else{
            //small check to make sure message is sending from client to server
            //printf("%s\n", "Message sent to server.");
            //clear buffer
            memset(send_buf, 0, MAX);
        }
    
        //should receive message from server & print error if no success receiving
        if (recv(sock, rcv_buf, MAX, 0) == -1){
            cout << "Receive error.\n";
        }else if(strcmp(rcv_buf, "221- CONNECTION TERMINATED.\n") == 0){
            cout << "Closing client socket.\n";
            close(sock);
            break;
        }else{//otherwise go on to send response & subsequently clear buffer
        cout << rcv_buf;
        memset(rcv_buf, 0, MAX);
        }
    }
}


// g++ -o sr main.cpp

// g++ -o cl client.cpp
// ./cl 127.0.0.1 2345
