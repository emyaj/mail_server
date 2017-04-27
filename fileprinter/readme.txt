readme.txt
Jayme Cartwright

Command line:
1. g++ -o sr main.cpp base64.cpp
   ./sr (portnum goes here)

2. g++ -o cl client.cpp
   ./cl (hostname) (same portnum as step 1)	

3. Window will instruct the user they must first say helo before proceeding 

4. type helo
   server replies with helo

5. then taken to helopt()
able to type anything but will send syntax error message if it doesnt recognize any of the following:
helo-just says helo agian
help-gives list of commands
auth-takes to auth method to enter passwords
mail-will send error that you need to go through auth first
quit-end the program

6. after sending auth command:
server responds asking for username

7. Enter username
If the username is invalid (ie not '@447.edu') will send error until valid username is entered

7a. First log in
Server generates random 5-digit password and sends to client, then closes connection
reestablish connection by again entering ./cl (hostname) (same portnum as step 1) 
then follow steps 4-7 again

8. Either reestablished connection or user has logged in before
server will prompt for password
enter same 5-digit sequence as given to you on first log in 

9. If authentication fails, user is taken back to helopt()
Otherwise, the next command that should be issued is mail

10. Once mail is received the server will ask who the mail from?
next user should enter the same username entered for log in-while loop until you do
Server will reply with OK

11. User then issues command rcpt
Server replies with rcpt to?
User then enters the name of the recipient

11a. if the recipient username is invalid (ie not '@447.edu') will receive error until vaild

12. User then should issue command data
Server will say OK
User then writes whatever they want, line-by-line
when user wants to utilize a new line, they simply hit enter
The server will recognize it received a line and will keep recognizing new lines until single . 

13. User is then instructed to quit