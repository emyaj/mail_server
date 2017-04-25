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
#include <cstring>
#include <time.h>



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
const static char auth_username[] = "334- dXNlcm5hbWU6\n"; //server response for "username" in base 64
const static char auth_password[] = "334- cGFzc3dvcmQ6\n"; //server response for "password" in base 64
const static char auth_first[] = "330- PASSWORD: "; //first log-in
const static char valid[] = "235- AUTHENTICATION SUCCEEDED\n"; //when log in is successful
const static char invalid[] = "535- AUTHENTICATION CREDENTIALS INVALID\n";


//the multithread function
void *connection_handler(void *);

//my methods
void helofirst(int tempsock);
void helopt(int tempsock);
void wannaquit(int tempsock);
void mailfrom(int tempsock);
void mailto(ofstream &os, int tempsock);
void mailcontent(ofstream &os, int tempsock);
bool alreadyhave(const char* filename);
void auth(int tempsock);
bool pcheck(char buf[]);

void Sleep(float s)
{
    int sec = int(s*1000000);
    usleep(sec);
}


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
    
    
    int listenfd, status, sock, sendfd;
    string name, folname, fistr, rcpt, datamsg, portnum;
    ofstream of;
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    socklen_t client_len;
    struct sockaddr_in client_addr; // connector's address information
    pid_t childp; //represents process ID
    string send_buf;
    
    
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
        //above works
            
            
            //go to helo method
            
            helofirst(sock);
        
            
            close(sock);
            printf("%s\n", "Connection ended.");
            
     
            
        
        }
        else if (childp > 0){
            close(sock);
        }
		      
    }	

}





//makes sure the user inputs 'helo' as the first command
void helofirst(int tempsock){
 
    char rcvd[MAX];

    while(recv(tempsock, rcvd, MAX, 0) > 0){
        
        if (strcmp(rcvd, "helo\n") == 0){
            printf("%s\n", "Recognized helo.");
            send(tempsock, helo, sizeof(helo), 0);
            memset(rcvd, 0, MAX);
            helopt(tempsock);
            
        //once this is working go to a method afterwards that loops through help menu
        }else {
            printf("%s\n", "Strcmp failed to recognize helo.");
            send(tempsock, order, sizeof(order), 0);
            memset(rcvd, 0, MAX);
            
        }
    }
}

//cycles through helo/help/mail/quit options
void helopt(int tempsock){
    
    char rcvd[MAX];

    //loops through helo, help and quit options after taken through initial helo, else out of order
    while(recv(tempsock, rcvd, MAX, 0) > 0){
        if (strcmp(rcvd, "helo\n") == 0){
            printf("%s\n", "Recognized helo.");
            send(tempsock, helo, sizeof(helo), 0);
            memset(rcvd, 0, MAX);
         
        }else if(strcmp(rcvd, "help\n") == 0){
            //send help message
            printf("%s\n", "Recognized help.");
            send(tempsock, help, sizeof(help), 0);
            memset(rcvd, 0, MAX);
            
        }else if(strcmp(rcvd, "auth\n") == 0){
            //send auth message
            printf("%s\n", "Recognized auth.");
            //request username
            send(tempsock, auth_username, sizeof(auth_username), 0);
            memset(rcvd, 0, MAX);
            //needs to enter auth method here
            auth(tempsock);
            
        }else if(strcmp(rcvd, "mail\n") == 0){
            //send out of order message
            printf("%s\n", "Recognized mail.");
            send(tempsock, order, sizeof(order), 0);
            memset(rcvd, 0, MAX);

        }else if(strcmp(rcvd, "quit\n") == 0){
            //send quit message
            printf("%s\n", "Recognized quit.");
            //needs to actually close socket here
            wannaquit(tempsock);
            
        }else{
            printf("%s\n", "Strcmp failed to recognize helo, help, auth, or quit.");
            send(tempsock, syntax_er, sizeof(syntax_er), 0);
            memset(rcvd, 0, MAX);
        }
    }
}





//auth method
void auth(int tempsock){
    
    char rcvd[MAX];
    int pinit[5];
    string username, userfile, userdir, userpass, passfile, temp;
    time_t thatime = time(0);
    char* dt = ctime(&thatime); //used for putting timestamp on email
    ofstream op; //writes to pass file
    ofstream of; //writes to mail file
    char at[] = "@447.edu";

        //receive username
        if(recv(tempsock, rcvd, MAX, 0) > 0){
            //the following line removes the newline that causes a '?'
            rcvd[strlen(rcvd) - 1] = '\0';
            username = string(rcvd);
            
            
            
            //ERROR HERE STRNCMP DOES NOT WORK
            if (username.find(at)  == (sizeof(username)-8)){
                cout << "valid username\n";
            }else{
                cout << "invalid username\n";
            }
            
            
            
            //tests to see what buffer is receiving & what dir/filenames will be
            
            cout << "username: " << username << endl;
            
            userdir = "db/" + username;
            cout << "userdir: " << userdir << endl;
            
            userfile = userdir + "/" + username + ".txt";
            cout << "userfile: " << userfile << endl;
            
            passfile = userdir + "/user_pass";
            cout << "passfile: " << passfile << endl;

            //clear rcvd buf
            memset(rcvd, 0, MAX);
            
            if(alreadyhave(userdir.c_str())){
                //have logged in before, reply with 334 dXNlcm5hbWU6
                send(tempsock, auth_password, sizeof(auth_password), 0);
                
                
                //here will check to see if passwords match
                if(recv(tempsock, rcvd, MAX, 0) > 0){
                    
                    if(pcheck(rcvd)){
                        //password authorized
                        send(tempsock, valid, sizeof(valid), 0);
                        mailfrom(tempsock);
                    }else{
                        //password invalid
                        send(tempsock, invalid, sizeof(invalid), 0);
                        //back to helopt()
                        helopt(tempsock);
                    }
                    
                }
                
            }else{//have not logged in before
                
                //should make folder inside db folder with email address as title
                mkdir(userdir.data(), 0700);
                //should make file inside userdir folder with password
                op.open(passfile);
                of.open(userfile);
                
                //print date to userfile
                of << dt;
                
                // first log in replies with 330 and 5-digit randomly generated password
                cout << "Rand generated: ";
                
                //sending "PASSWORD: xxxxx
                send(tempsock, auth_first, sizeof(auth_first), 0);
                
                
                //gives different random number each time--ignore warning
                srand(time(NULL));

                for (int i=0; i<5; ++i){
                    pinit[i] = rand() % 10; //range 0 to 9
                    cout << pinit[i];
                    op << pinit[i];
                }
                cout << endl;
                
                //if received 330 code & temp password-->terminate connection
                cout << "Socket closing...\n";
                //wait 5 seconds and close connection
                Sleep(5);
                close(tempsock);
            }

        }
    
}

bool pcheck(char buf[]){
    // add '447' to password & encode in base-64
    // stores the encoded password w the corresponding username in hidden file “.user_pass” in db folder
    

    
    char pc[5];
    
    //convert both FROM base64
    
    //add 447 to each one
    
    //convert both TO base64
    
    //check if they match
    if(strcmp(buf, pc) == 0){
       
        //passwords match
        return true;
     
    }else{
        
        return false;
    }
    
    
}



//after entering mail command, user taken to this method
void mailfrom(int tempsock){
    
    cout << "Entered mail method\n";
    
    char rcvd[MAX];
    string uname, fold, fname;
    ofstream of;
    time_t thatime = time(0);
    char* dt = ctime(&thatime); //used for putting timestamp on email
    int var = 1;

    
    //receive here who the mail is from
    if(recv(tempsock, rcvd, sizeof(rcvd), 0) > 0){
        //the following line removes the newline that causes a '?'
        rcvd[strlen(rcvd) - 1] = '\0';
        uname = string(rcvd);
        
        //writes on server window who the sender is
        cout << "Message from: " << uname << "\n";
        send(tempsock, okay, sizeof(okay), 0);
        
        //sets foldername to senders email address
        fold =  "db/" + uname;

        
        //checks if folder is already there
        if(alreadyhave(fold.c_str())){
            cout << "This folder already exists\n";
            fname =  fold + "/" + uname + to_string(var);
            var++;
        }else{
            //make folder for user
            mkdir(fold.data(), 0700);
            fname = fold + "/" + uname;
        }
        
        //clear memory buffer
        memset(rcvd, 0, MAX);

        //opens email file
        of.open(fname);
        
        //prints time and who from if open then sends to rcpt method
        if(of.is_open()){
            of << dt;
            of << "Mail from: " << uname << endl;
            //send to rcpt method
            mailto(of, tempsock);
        }
        
    }else{
        send(tempsock, order, sizeof(order), 0);
        //goes back to helo options
        helopt(tempsock);
    }
    
}


//method writes who the mail is from
void mailto(ofstream &os, int tempsock){
    
    char rcvd[MAX];
    string recip;
    
    while(recv(tempsock, rcvd, sizeof(rcvd), 0) > 0){
    
        if(strcmp(rcvd, "rcpt\n") == 0){
            
            send(tempsock, rcpt, sizeof(rcpt), 0);
            //clears buffer to receive again
            memset(rcvd, 0, MAX);

            //this should receive the recipient's name
            recv(tempsock, rcvd, sizeof(rcvd), 0);
            rcvd[strlen(rcvd) - 1] = '\0'; //makes buffer ignore '\n'
            recip = string(rcvd);
            os << "Mail rcpt: " << recip << "@447.edu\n";

            //clear buffer memory again
            memset(rcvd, 0, MAX);
            
            //send ok message
            send(tempsock, okay, sizeof(okay), 0);

            //go to data method
            mailcontent(os, tempsock);
            
        }else{
            send(tempsock, order, sizeof(order), 0);
            //goes back to helo options
            helopt(tempsock);
        }
    
    }
}

//gets data for message content
void mailcontent(ofstream &os, int tempsock){
    
    char rcvd[MAX];
    string dta;
    char lin[] = "Line received.\n";
    char end[] = "End of mail determined.\nWrite another email by entering 'mail', or enter 'quit' to quit.\n";
    
    //while loop here reads in each line and prints it until a single period is entered
    if(recv(tempsock, rcvd, sizeof(rcvd), 0) > 0){
        
        //needs to recieve data command before you can enter the data section
        if(strcmp(rcvd, "data\n") == 0){
            send(tempsock, data, sizeof(data), 0);
            memset(rcvd, 0, MAX);

        } else{
            send(tempsock, order, sizeof(order), 0);
            memset(rcvd, 0, MAX);
            helopt(tempsock);
        }
        
        
        //should receive first line of file & loop through until getting single '.'
        while( recv(tempsock, rcvd, sizeof(rcvd), 0) > 0){

            dta = string(rcvd);
            
            if(strcmp(rcvd, ".\n") == 0){
                //means end of mail message
                send(tempsock, end, sizeof(end), 0);
                helopt(tempsock);
            }
            else{
                os << dta;
                send(tempsock, lin, sizeof(lin), 0);
                memset(rcvd, 0, MAX);
            }
            
        }
    }
}


//should return 0 if file exists and -1 if not
bool alreadyhave(const char* filename){
    
    struct stat fileInfo;
    return stat(filename, &fileInfo) == 0;
}


void wannaquit(int tempsock){
    send(tempsock, quit, sizeof(quit), 0);
    close(tempsock);
}


// g++ -o sr main.cpp
// g++ -o cl client.cpp




