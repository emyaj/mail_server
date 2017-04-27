//
//  udpserv.cpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/26/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#include <stdio.h>
#include <sys/socket.h>
#include <iostream> //namespace std
#include <sys/stat.h> //for mkdir
#include <fstream>
#include <string.h>    //strlen
#include <stdlib.h>    //namespace std
#include <arpa/inet.h> //inet_addr
#include <netinet/in.h>
#include <errno.h> //errors
#include <signal.h>
#include <sys/types.h> //socket prerec typedefs
#include <iostream> //namespace std

#include <unistd.h> //write(int fd, const void *buf, size_t count); >>same as send but w/o flags
#include <ctime> //for time
#include <sys/wait.h>
#include <netdb.h> //addrinfo
#include <cstring>
#include <time.h>


#include "base64.hpp"


int MAX= 1080;
const static char okay[] = "HTTP/1.1 200 OK";
const static char notokay[] = "HTTP/1.1 200 INVALID";

bool alreadyhave(const char* filename);
bool acheck(string username);


using namespace std;


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}



int main(int argc, char * argv[]) {

    int sock, status;
    ssize_t numbytes;
    string portnum, fname, dname;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char rcv[MAX];
    time_t thatime = time(0);
    char* dt = ctime(&thatime); //used for putting timestamp on email
    ifstream ifs;
    ofstream ofs;
    
    //argc should be 2 for correct execution
    if (argc != 2) {
        fprintf(stderr,"forgot port number\n");
        exit(1);
    }
    //memory allocation
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;// don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;// TCP socket
    hints.ai_flags = AI_PASSIVE; // fill in IP- assign the address of my local host to the socket structures
    


    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }cout << "200: Socket created.\n";


    mkdir("db/recpt" ,0700);
    
    portnum = argv[1];
    cout << "Using port : " << portnum << "\n" ;
    
    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        if (::bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    
    
    
    
    addr_len = sizeof their_addr;
    
    //receive from client
    if ((numbytes = recvfrom(sock, rcv, MAX-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    //rcv[strlen(rcv) - 1] = '\0';

    
    if(!acheck(string(rcv))){
        sendto(sock, notokay, sizeof(notokay), 0, (struct sockaddr *)&their_addr, addr_len);

    }else{
        sendto(sock, okay, sizeof(okay), 0, (struct sockaddr *)&their_addr, addr_len);

    }
    
    //rcv[strlen(rcv) - 1] = '\0';
    dname ="db/recpt/" + string(rcv);
    cout << "dname: "<< dname << endl;
    fname = dname + "/" + string(rcv);
    cout << "fname: "<< fname << endl;

    mkdir(fname.c_str(),0700);
    
    
    ofs.open(fname);
    
    ofs << dt;
    
    //send to client
    //sendto(sock, okay, sizeof(okay), 0, (struct sockaddr *)&their_addr, addr_len);
    //sendto(sock, okay, sizeof(okay), 0, (struct sockaddr *)&their_addr, addr_len);

    
    
    cout << "listener: packet is " << numbytes << " bytes long\n" ;
    cout << "rcv: "<< rcv << endl;
    
    
    
    
    
    
    
    
    freeaddrinfo(servinfo);
    close(sock);
    return 0;
    
    
    
}


bool acheck(string username){
    
    char at[] = "@447.edu";
    string atcheck;
    
    //loop stores everything after the @ into 'atcheck'
    for(int i=0; i< sizeof(username); i++){
        if (username[i]=='@'){
            //cout << "@ sign found\n";
            for(int j=i; j < (sizeof(username)-i); j++){
                atcheck += username[j];
            }
        }
    }
    
    //makes sure email is @447.edu
    if (strcmp(atcheck.c_str(), at)==0){
        cout << "valid username\n";
        return true;
    }else{
        cout << "ERROR: invalid username\n";
        return false;
    }
}

//should return 0 if file exists and -1 if not
bool alreadyhave(const char* filename){
    
    struct stat fileInfo;
    return stat(filename, &fileInfo) == 0;
}
