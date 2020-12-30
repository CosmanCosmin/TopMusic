#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include <fstream>
using namespace std;
int main(int argc, char* argv[]){
    ifstream fin("help.txt");
    int port;
    int sd;
    struct sockaddr_in server;
    char message[10000];
    //check connection syntax
    if(argc != 3){
        cerr << "Syntax error\n";
        return -1;
    }
    //Set port
    port = atoi(argv[2]);
    //Create socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        cerr << "Couldn't create socket...\n";
        return -1;
    }
    //Fill the struct
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);
    //Connect to server
    if (connect(sd, (struct sockaddr*) &server, sizeof (struct sockaddr)) == -1){
        cerr << "Couldn't connect to server...\n";
        return -1;
    }
    string line;
    while (getline(fin, line)) cout << line << "\n";
    //Read and Send commands
    while (true) {
        bzero(message, 10000);
        read(0, message, 10000);
        int messageSize = strlen(message) + 1;
        if(write(sd, &messageSize, sizeof(int)) <= 0){
            cerr << "Write to server error\n";
            return -1;
        }
        if (write(sd, message, messageSize) <= 0) {
            cerr << "Write to server error\n";
            return -1;
        }
        if (strcmp(message, "quit\n") == 0) {
            close(sd);
            break;
        }
        int receivedSize;
        if(read(sd, &receivedSize, sizeof(int)) <= 0){
            cerr << "Read error from server\n";
            return -1;
        }
        if (read(sd, message, receivedSize) < 0) {
            cerr << "Read error from server\n";
            return -1;
        }
        cout << "-" << message;
    }
    return 0;
}