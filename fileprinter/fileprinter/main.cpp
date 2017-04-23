//
//  main.cpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/12/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#include <iostream> //namespace std
#include <sys/stat.h> //for mkdir
#include <unistd.h> //write(int fd, const void *buf, size_t count); >>same as send but w/o flags
#include <fstream>
#include <ctime> //for time
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <stdlib.h>    //namespace std
#include <sys/socket.h>  //socket
#include <sys/types.h> //socket prerec typedefs
#include <arpa/inet.h> //inet_addr
#include <netinet/in.h>
#include <errno.h> //errors
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>



using namespace std;

//constants
int MAX = 1080;
const int BACKLOG = 6;
const static char first[] = "Must enter 'helo' before proceeding.\n"; //first sent msg to client
const static char help[] = "214- OPTIONS:\n Type one of the following & hit ENTER.\n"
                            "'helo'  to greet the server & set up for SMTP transaction.\n"
                            "'mail'  to initiate the SMTP transaction.\n"
                            "'rcpt' to identify individual recipient of the message.\n"
                            "'data'  to begin data field of message.\n"
                            "'quit' to close the connection.\n"; //help dialogue
const static char helo[] = "250- HELO.\n"; //necessary to receive from client before you can start SMTP connection
const static char mail[] = "250- MAIL FROM?: \n"; //used to initiate SMTP transaction
const static char rcpt[] = "250- RCPT TO?: \n"; //to identify individual recipient of message
const static char data[] = "354- DATA: \n"; //used to ask for data section of e-mail
const static char quit[] = "221- CONNECTION TERMINATED.\n"; //terminates client-server communication
const static char order[] = "503- BAD SEQUENCE OF COMMANDS.\n"; //for when commands are out of order
const static char syntax_er[] = "500- COMMAND NOT RECOGNIZED.\n"; //for when anything other than accepted commands are received
const static char okay[] = "250- OK.\n"; //general ok message







//the multithread function
void *connection_handler(void *);



//my methods
void helopt(int tempsock);


void mailfrom(ofstream &os, string n);
void mailto(ofstream &os, string n);
void mailcontent(ofstream &os, string n);
bool alreadyhave(const char* filename);


void sigchld_handler(int s){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


int main(int argc, char * argv[]) {
    
    
    int listenfd, status, sock, sendfd, rcvfd;
    string name, folname, fistr, rcpt, datamsg, portnum;
    const char* dbname;
    const char* finame;
    ofstream of;
    time_t thatime = time(0);
    char* dt = ctime(&thatime); //used for putting timestamp on email
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    socklen_t client_len;
    struct sockaddr_in client_addr; // connector's address information
    pid_t childp; //represents process ID
    string send_buf;
    char rcv_buf[MAX];
    
    
    //argc should be 2 for correct execution
    if (argc != 2) {
        fprintf(stderr,"forgot port number\n");
        exit(1);
    }
    
    //creates TCP socket & if socket value < 0, writes error.
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Cannot create socket\n");
        exit(1);
    } cout << "200: Socket created.\n";
    
    //memory allocation
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;// don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;// TCP socket
    hints.ai_flags = AI_PASSIVE; // fill in IP- assign the address of my local host to the socket structures

    
    //asks for argument in command line of port number & checks for error
    // servinfo points to a linked list of 1 or more struct addrinfos
    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    portnum = argv[1];
    cout << "Using port : " << portnum << "\n" ;
    
    
    
    
    // loop through all the results and bind to the first one we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        
        //tries to create socket, if it cannot-returns error, otherwise prints success
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket error");
            continue;
        }
        //setsockopt(int socket, int level, int option_name,const void *option_value, socklen_t option_len)
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt error");
            exit(1);
        }
        //bind(int socket, const struct sockaddr *address, socklen_t address_len)
        //checks if socket binds & notifies if failure
        if ( (::bind(listenfd, p->ai_addr, p->ai_addrlen)) == -1) {
            close(listenfd);
            perror("server: bind error");
            continue;
        }
   
        break;
    }

    //exiting the loop means the server failed to bind to port
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    
    
    //free the linked-list
    freeaddrinfo(servinfo);
    
    
    
    //listen at the port, allows 6 pending connectionis in the queue
    listen(listenfd, BACKLOG);
    printf("%s\n", "Server is waiting on client.");
    
    
    
    
    //makes db folder
    mkdir("db", 0700);
    
    //never-ending for loop to receive all messages from the client
    for (;;) {
        
        client_len = sizeof(client_addr);
        //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        sock = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
        sendfd = sock;
        if(sock>0){
            printf("%s\n", "Connection Accepted.");

        }
        
        

        //should account for multiple client processes
        if ((childp = fork()) == 0){
            
        //Initial message sent to client to inform that first command must = 'helo'
        //success or lack thereof printed to server window
        if (send(sendfd, first, sizeof(first), 0) == -1){
            printf("%s\n", "Initial message failed.");

        }printf("%s\n", "Initial message sent to client.");    
            
            
            
            //receive word from client
            if(recv(sendfd, rcv_buf, MAX, 0)== -1){
              cout <<"rcv error";
        }else{
            cout << "msg rcvd";
        }
            
            //go to helo method
            
            //helopt(sendfd);
        
            
            
            //receive username from client
            
        
        }
        else if (childp > 0){
            close(sock);
        }
		      
    }	

}
    
    
    
    
    
    
    
//makes sure the user inputs 'helo' as the first command
void helopt(int tempsock){
    int helosock = tempsock;
    char rcvd[MAX];
    
    if (recv(helosock, rcvd, MAX, 0) == -1){
        cout << "Receive error.\n";
    }else if (strcmp(rcvd, "helo") == 0){
        cout << "Received helo.\n";
        
    }
    
    
    
}

    
    
    
    /*
    
    
    
    

            cout << "Enter name\n";
            cin >> name;
            folname = "db/" + name;
        
            //convert username from string to character array
            dbname = folname.c_str();
    
   
    
    
    
    

    
    if(alreadyhave(dbname)){
        //need to put incremented filenames
        fistr = "db/" + name + "/" + name + "1";
        
    }else{
        //makes username folder INSIDE DB folder
        mkdir(dbname, 0700);
        fistr = "db/" + name + "/" + name;

    }
    
    
    //opens file to print
    finame = fistr.c_str();
    
    
    of.open(finame);
    
    of << dt;
    if(of.is_open()){
        mailfrom(of, name);
    
        
        
        cout << "Enter rcpt\n";
        cin >> rcpt;
    
    
        mailto(of, rcpt);
        
        
        cout << "data:\n";
        mailcontent(of, datamsg);
        
    }
    
    return 0;
}

*/






//method writes who the mail is from
void mailfrom(ofstream &os, string n){
    
    if(os.is_open()){
        os << "Mail from: " + n + "@447.edu\n";
    }
}


//writes who the mail is to
void mailto(ofstream &os, string n){
    
    if(os.is_open()){
        os << "Mail to: " + n + "@447.edu\n";
    }
}


//gets data for message content
void mailcontent(ofstream &os, string n){
    
    //while loop here reads in each line and prints it until a single period is entered
    while(n != "."){
        getline (cin, n);
        os << n << endl;
      
    }

}


//should return 0 if file exists and -1 if not
bool alreadyhave(const char* filename)
{
    struct stat fileInfo;
    return stat(filename, &fileInfo) == 0;
}





// g++ -o sr main.cpp



// g++ -o cl client.cpp




