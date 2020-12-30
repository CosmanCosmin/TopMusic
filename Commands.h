#include <iostream>
#include <string>
#include <vector>
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#ifndef PROIECTRETELETOPMUSIC_LOGIN_H
#define PROIECTRETELETOPMUSIC_LOGIN_H
using namespace std;

class Commands {
private:
    sql::Driver *driver;
    sql::Connection *con;
public:
    Commands();
    ~Commands();
    bool login(char*, char*);
    bool checkUsername(char*);
    bool Register(char*, char*);
    int getUserID(char*);
    int getSongID(char*, char*);
    void addSong(char*, char*, char*, char*, vector<string>);
    bool vote(char*, char*, char*);
    bool canVote(char*);
    void revoke(char*);
    bool isAdmin(char*);
    string top();
    string top(char*);
    string showComments(char*, char*);
    bool makeAdmin(char*);
    bool removeVote(char*, char*, char*);
    void comment(char*, char*, char*, char*);
    bool removeSong(char*, char*);
};
#endif //PROIECTRETELETOPMUSIC_LOGIN_H
