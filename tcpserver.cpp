#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <unordered_map>
#include "Commands.h"
#define PORT 4123
bool connected;
bool logged;
string currentUser;
void sigHandler(int){
    int waitCode;
    while (true){
        waitCode = waitpid(-1, nullptr, WNOHANG);
        if (0 != waitCode) {
            if (-1 == waitCode) {
                return;
            }
        } else {
            break;
        }
    }
}
char* readFromClient(int client){
    int receivedSize;
    if(read(client, &receivedSize, sizeof(int)) <= 0){
        cerr << "Read error from client...\n";
        close(client);
        connected = false;
    }
    char *message = new char[receivedSize];
    if(read(client, message, receivedSize) <= 0){
        cerr << "Read error from client...\n";
        close(client);
        connected = false;
    }
    return message;
}
void writeToClient(int client, char* message){
    int sendSize = strlen(message) + 1;
    if(write(client, &sendSize, sizeof(int)) <= 0) {
        cerr << "Write error to client...\n";
    }
    if(write(client, message, sendSize) <= 0) {
        cerr << "Write error to client...\n";
    }
    else {
        cout << "Answer sent successfully!\n";
    }
}
void handleLogin(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter username:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *username = new char [strlen(message) + 1];
    strcpy(username, message);
    username[strlen(username) - 1] = 0;
    strcpy(answer, "Enter password:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *password = new char[strlen(message) + 1];
    strcpy(password, message);
    password[strlen(password) - 1] = 0;
    Commands object;
    if(!object.login(username, password)){
        strcpy(answer, "Invalid login!\n");
        writeToClient(client, answer);
    }
    else{
        strcpy(answer, "Successfully logged in!\n");
        writeToClient(client, answer);
        logged = true;
        currentUser = username;
    }
}
void handleRegister(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter desired username:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *username = new char [strlen(message) + 1];
    strcpy(username, message);
    username[strlen(username)-1] = 0;
    strcpy(answer, "Enter desired password:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *password = new char[strlen(message) + 1];
    strcpy(password, message);
    password[strlen(password)-1] = 0;
    strcpy(answer, "Repeat the password:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *check = new char[strlen(message) + 1];
    strcpy(check, message);
    check[strlen(check)-1] = 0;
    if(strcmp(password, check) != 0){
        strcpy(answer, "Passwords must match!\n");
        writeToClient(client, answer);
    }
    else{
        Commands object;
        if(!object.Register(username, password)){
            strcpy(answer, "Username is already taken!\n");
            writeToClient(client, answer);
        }
        else{
            strcpy(answer, "Successfully registered!\n");
            writeToClient(client, answer);
        }
    }
}
void handleAdd(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter song name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* songName = new char [strlen(message) + 1];
    strcpy(songName, message);
    songName[strlen(songName)-1] = 0;
    strcpy(answer, "Enter artist name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* artist = new char [strlen(message) + 1];
    strcpy(artist, message);
    artist[strlen(artist)-1] = 0;
    strcpy(answer, "Enter youtube URL:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* URL = new char [strlen(message) + 1];
    strcpy(URL, message);
    URL[strlen(URL)-1] = 0;
    strcpy(answer, "Enter description:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* description = new char [strlen(message) + 1];
    strcpy(description, message);
    description[strlen(description)-1] = 0;
    bool run = true;
    vector<string> genres;
    while(run){
        strcpy(answer, "Enter genre(Put a dot(.) to stop):\n");
        writeToClient(client, answer);
        strcpy(message, readFromClient(client));
        message[strlen(message)-1] = 0;
        string genre = message;
        if(genre != "."){
            genres.push_back(genre);
        }
        else run = false;
    }
    Commands object;
    object.addSong(songName, artist, URL, description, genres);
}
int handleVote(int client){
    Commands object;
    if(!object.canVote(const_cast<char *>(currentUser.c_str()))){
        return -1;
    }
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter song name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* songName = new char [strlen(message) + 1];
    strcpy(songName, message);
    songName[strlen(songName)-1] = 0;
    strcpy(answer, "Enter artist name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* artist = new char [strlen(message) + 1];
    strcpy(artist, message);
    artist[strlen(artist)-1] = 0;
    if (object.vote(const_cast<char *>(currentUser.c_str()), songName, artist)) {
        return 1;
    }
    else {
        return 0;
    }
}
void handleRevoke(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter username:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *username = new char [strlen(message) + 1];
    strcpy(username, message);
    username[strlen(username)-1] = 0;
    Commands object;
    object.revoke(username);
}
void handleTop(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter a genre(put a . for the general top):\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    message[strlen(message)-1] = 0;
    string genre = message;
    Commands object;
    if(genre == "."){
        string topList = object.top();
        strcpy(answer, topList.c_str());
        writeToClient(client, answer);
    }
    else{
        string topList = object.top(const_cast<char *>(genre.c_str()));
        strcpy(answer, topList.c_str());
        writeToClient(client, answer);
    }
}
bool handleAdmin(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter username:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char *username = new char [strlen(message) + 1];
    strcpy(username, message);
    username[strlen(username)-1] = 0;
    Commands object;
    if(object.makeAdmin(username)){
        return true;
    }
    else {
        return false;
    }
}
bool handleUnvote(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter song name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* songName = new char [strlen(message) + 1];
    strcpy(songName, message);
    songName[strlen(songName)-1] = 0;
    strcpy(answer, "Enter artist name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* artist = new char [strlen(message) + 1];
    strcpy(artist, message);
    artist[strlen(artist)-1] = 0;
    Commands object;
    if(object.removeVote(const_cast<char *>(currentUser.c_str()), songName, artist)){
        return true;
    }
    else {
        return false;
    }
}
void handleComment(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter song name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* songName = new char [strlen(message) + 1];
    strcpy(songName, message);
    songName[strlen(songName)-1] = 0;
    strcpy(answer, "Enter artist name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* artist = new char [strlen(message) + 1];
    strcpy(artist, message);
    artist[strlen(artist)-1] = 0;
    strcpy(answer, "Leave a comment:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* comment = new char [strlen(message) + 1];
    strcpy(comment, message);
    comment[strlen(comment)-1] = 0;
    Commands object;
    object.comment(songName, artist, const_cast<char *>(currentUser.c_str()), comment);
}
void handleInformation(int client){
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter song name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* songName = new char [strlen(message) + 1];
    strcpy(songName, message);
    songName[strlen(songName)-1] = 0;
    strcpy(answer, "Enter artist name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* artist = new char [strlen(message) + 1];
    strcpy(artist, message);
    artist[strlen(artist)-1] = 0;
    Commands object;
    string comments = object.showComments(songName, artist);
    strcpy(answer, comments.c_str());
    writeToClient(client, answer);
}
bool handleDelete(int client) {
    char answer[10000];
    char message[10000];
    strcpy(answer, "Enter song name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* songName = new char [strlen(message) + 1];
    strcpy(songName, message);
    songName[strlen(songName)-1] = 0;
    strcpy(answer, "Enter artist name:\n");
    writeToClient(client, answer);
    strcpy(message, readFromClient(client));
    char* artist = new char [strlen(message) + 1];
    strcpy(artist, message);
    artist[strlen(artist)-1] = 0;
    Commands object;
    return object.removeSong(songName, artist);
}
int main(){
    unordered_map<string, int> commands = {{"quit", 0}, {"login", 1}, {"logout", 2}, {"register", 3}, {"add", 4}, {"vote", 5},
                                           {"revoke", 6}, {"top", 7}, {"admin", 8}, {"remove", 9}, {"comment", 10}, {"information", 11},
                                           {"delete", 12}};
    struct sockaddr_in server{}; //struct used by server
    struct sockaddr_in from{};
    char message[10000]; //message from client
    char answer[10000]; //answer for client
    int sd; //socket descriptor
    int i;
    Commands object;
    //1. Create a socket
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        cerr << "Couldn't create socket...\n";
        return -1;
    }
    //2. Prepare data structures
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
    //3. Fill data structures
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons (PORT);
    //4. Bind the socket
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    if (bind (sd, (struct sockaddr*) &server, sizeof (struct sockaddr)) == -1){
        cerr << "Couldn't bind the socket...\n";
        return -1;
    }
    //5. Make the server listen for clients
    if(listen(sd, 5) == -1){
        cerr << "Couldn't listen...\n";
        return -1;
    }
    if (signal(SIGCHLD, sigHandler) == SIG_ERR){
        cerr << "Signal error...\n";
        return -1;
    }
    while (true){
        int client;
        int length = sizeof (from);
        cout << "Waiting at port:" << PORT << "\n";
        fflush(stdout);
        //6. Accept a client
        client = accept(sd, (struct sockaddr *) &from, reinterpret_cast<socklen_t *>(&length));
        if (client < 0){
            cerr << "Accept error...\n";
            continue;
        }
        int pID = fork();
        switch (pID) {
            case -1:
                cerr << "Fork error...\n";
                exit(-1);
            case 0:
                connected = true;
                logged = false;
                //connection worked
                while(connected){
                    bzero(message, 100);
                    cout << "Waiting for command!\n";
                    strcpy(message, readFromClient(client));
                    cout << "Message from client:" << message;
                    //Preparing answer
                    bzero(answer, 100);
                    string command = message;
                    command = command.substr(0, command.length() - 1);
                    auto got = commands.find(command);
                    if (got == commands.end()) {
                        strcpy(answer, "Invalid syntax\n");
                        writeToClient(client, answer);
                    }
                    else {
                        cout << got->first << " " << got->second << "\n";
                        switch (got->second) {
                            case 0:
                                exit(0); //end case 0
                            case 1:
                                if (logged) {
                                    //Send answer
                                    strcpy(answer, "Already logged in!\n");
                                    writeToClient(client, answer);
                                }
                                else handleLogin(client);
                                break; //end case 1
                            case 2:
                                if(logged){
                                    logged = false;
                                    strcpy(answer, "Successfully logged out!\n");
                                    writeToClient(client, answer);
                                }
                                else{
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 2
                            case 3:
                                if (logged) {
                                    strcpy(answer, "You can't register while logged in!\n");
                                    writeToClient(client, answer);
                                }
                                else handleRegister(client);
                                break; //end case 3
                            case 4:
                                if (!logged) {
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                else {
                                    handleAdd(client);
                                    strcpy(answer, "Successfully added!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 4
                            case 5:
                                if (!logged) {
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                else {
                                    int result = handleVote(client);
                                    if(result == 1) {
                                        strcpy(answer, "Successfully voted!\n");
                                        writeToClient(client, answer);
                                    }
                                    else if (result == -1) {
                                        strcpy(answer, "You don't have the right to vote!\n");
                                        writeToClient(client, answer);
                                    }
                                    else {
                                        strcpy(answer, "Song doesn't exist!\n");
                                        writeToClient(client, answer);
                                    }
                                }
                                break; //end case 5
                            case 6:
                                if (logged && object.isAdmin(const_cast<char *>(currentUser.c_str()))) {
                                    handleRevoke(client);
                                    strcpy(answer, "Successfully revoked rights!\n");
                                    writeToClient(client, answer);
                                }
                                else {
                                    strcpy(answer, "You need to be logged in as admin!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 6
                            case 7:
                                if (logged) {
                                    handleTop(client);
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 7
                            case 8:
                                if (logged && object.isAdmin(const_cast<char *>(currentUser.c_str()))) {
                                    if (handleAdmin(client)) {
                                        strcpy(answer, "Successfully made the user an admin!\n");
                                        writeToClient(client, answer);
                                    }
                                    else {
                                        strcpy(answer, "User doesn't exist!\n");
                                        writeToClient(client, answer);
                                    }
                                }
                                else {
                                    strcpy(answer, "You need to be logged in as admin!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 8
                            case 9:
                                if (logged) {
                                    if (handleUnvote(client)) {
                                        strcpy(answer, "Successfully removed the vote!\n");
                                        writeToClient(client, answer);
                                    }
                                    else {
                                        strcpy(answer, "There is no vote to remove!\n");
                                        writeToClient(client, answer);
                                    }
                                }
                                else {
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 9
                            case 10:
                                if (logged) {
                                    handleComment(client);
                                    strcpy(answer, "Successfully added a comment!\n");
                                    writeToClient(client, answer);

                                }
                                else {
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 10
                            case 11:
                                if (logged) {
                                    handleInformation(client);
                                }
                                else {
                                    strcpy(answer, "You have to login first!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 11
                            case 12:
                                if (logged && object.isAdmin(const_cast<char *>(currentUser.c_str()))) {
                                    if (handleDelete(client)) {
                                        strcpy(answer, "Successfully deleted the song!\n");
                                        writeToClient(client, answer);
                                    }
                                    else {
                                        strcpy(answer, "This song doesn't exist!\n");
                                        writeToClient(client, answer);
                                    }
                                }
                                else {
                                    strcpy(answer, "You need to be logged in as admin!\n");
                                    writeToClient(client, answer);
                                }
                                break; //end case 12
                        }
                    }
                }
        }
        close(client);
    }
}