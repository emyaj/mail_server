//
//  main.cpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/12/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <ctime>

using namespace std;

void mailfrom(ofstream &os, string n);
void mailto(ofstream &os, string n);
void mailcontent(ofstream &os, string n);
bool alreadyhave(const char* filename);



int main(int argc, const char * argv[]) {

    string name, folname, fistr, rcpt, datamsg;
    const char* dbname;
    const char* finame;
    ofstream of;
    time_t thatime = time(0);
    char* dt = ctime(&thatime);
    
    
    cout << "Enter name\n";
    cin >> name;
    
    //makes db folder
    mkdir("db", 0700);
    
    
    folname = "db/" + name;
    
    //convert username from string to character array
    dbname = folname.c_str();
    
    if(alreadyhave(dbname)){
        //need to put incremented filenames here
        
        
    }else{
        //makes username folder INSIDE DB folder
        mkdir(dbname, 0700);
    }
    
    
    //WORKS! :)
    
    //opens file to print
    fistr = "db/" + name + "/" + name;
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
