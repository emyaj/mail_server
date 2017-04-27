readme.txt
Jayme Cartwright

Command line:
1. type make to execute the makefile 

2. in one terminal: ./sr (portnum)
   in a second terminal: ./cl (hostname) (same portnum)


3. Window will instruct the user they must first say helo before proceeding 
   type helo
   server replies with helo

4. then taken to helopt()
able to type anything but will send syntax error message if it doesnt recognize any of the following:
helo-just says helo agian
help-gives list of commands
auth-takes to auth method to enter passwords
mail-will send error that you need to go through auth first
quit-end the program

5. after sending auth command:
server responds asking for username

6. Enter username
If the username is invalid (ie not '@447.edu') will send error until valid username is entered

6a. First log in
Server generates random 5-digit password and sends to client, then closes connection
reestablish connection by again entering ./cl (hostname) (same portnum as step 1) 
then follow steps 4-7 again

7. Either reestablished connection or user has logged in before
server will prompt for password
enter same 5-digit sequence as given to you on first log in 

8. If authentication fails, user is taken back to helopt()
Otherwise, the next command that should be issued is mail

9. Once mail is received the server will ask who the mail from?
next user should enter the same username entered for log in-while loop until you do
Server will reply with OK

10. User then issues command rcpt
Server replies with rcpt to?
User then enters the name of the recipient

10a. if the recipient username is invalid (ie not '@447.edu') will receive error until vaild

11. User then should issue command data
Server will say OK
User then writes whatever they want, line-by-line
when user wants to utilize a new line, they simply hit enter
The server will recognize it received a line and will keep recognizing new lines until single . 

12. User is then instructed to quit



==============
UDP
1. execute both commands
   ./us (port)
   ./rc (host) (port)