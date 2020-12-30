#include "Commands.h"

Commands::Commands() {
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "cosmin", "cosmin");
    con->setSchema("TopMusic");
}

bool Commands::login(char* username, char* password) {
    string name = username;
    string pass = password;
    sql::Statement *statement;
    sql::ResultSet *results;
    bool flag = false;
    statement = con->createStatement();
    results = statement->executeQuery("SELECT username, password FROM credentials");
    while (results->next()){
        if((results->getString("username") == name) && (results->getString("password") == pass)) {
            flag = true;
        }
    }
    delete results;
    delete statement;
    return flag;
}

bool Commands::Register(char* username, char* password) {
    if (!this->checkUsername(username)){
        return false;
    }
    sql::PreparedStatement *prepStatement;
    prepStatement = con->prepareStatement("INSERT INTO credentials(username, password, admin, restricted) VALUES (?, ?, ?, ?)");
    prepStatement ->setString(1, username);
    prepStatement ->setString(2, password);
    prepStatement ->setString(3, "N");
    prepStatement ->setString(4, "N");
    prepStatement ->executeUpdate();
    delete prepStatement;
    return true;
}

Commands::~Commands() {
    delete con;
}

bool Commands::checkUsername(char* username) {
    string name = username;
    bool flag = true;
    sql::Statement *statement;
    sql::ResultSet *results;
    statement = con->createStatement();
    results = statement->executeQuery("SELECT username FROM credentials");
    while (results->next()){
        if(results->getString("username") == name) flag = false;
    }
    delete results;
    delete statement;
    return flag;
}

int Commands::getUserID(char* username) {
    string name = username;
    sql::PreparedStatement *statement;
    sql::ResultSet *results;
    statement = con->prepareStatement("SELECT id FROM credentials WHERE username  = ?");
    statement->setString(1, username);
    results = statement->executeQuery();
    if (results->rowsCount() == 0) {
        return -1;
    }
    results->next();
    int userid = results->getInt("id");
    delete results;
    delete statement;
    return userid;
}

void Commands::addSong(char* songName, char* artist, char* URL, char* description, vector<string> genres) {
    string name = songName;
    string singer = artist;
    string link = URL;
    string about = description;
    sql::PreparedStatement *statement;
    statement = con->prepareStatement("INSERT INTO songs(name, artist, url, description) VALUES (?, ?, ?, ?)");
    statement->setString(1, name);
    statement->setString(2, singer);
    statement->setString(3, link);
    statement->setString(4, about);
    statement->executeUpdate();
    sql::PreparedStatement *genreInsert;
    int songid = getSongID(songName, artist);
    genreInsert = con->prepareStatement("INSERT INTO genres(songid, genre) VALUES (?, ?)");
    genreInsert->setInt(1, songid);
    for(auto it : genres){
        genreInsert->setString(2, it);
        genreInsert->executeUpdate();
    }
    delete statement;
    delete genreInsert;
}

int Commands::getSongID(char* name, char* artist) {
    string song = name;
    string singer = artist;
    sql::PreparedStatement *songidquery;
    sql::ResultSet *results;
    songidquery = con->prepareStatement("SELECT songid FROM songs WHERE (name  = ?) AND (artist = ?)");
    songidquery->setString(1, song);
    songidquery->setString(2, singer);
    results = songidquery->executeQuery();
    if (results->rowsCount() == 0 ) {
        return -1;
    }
    results->next();
    int songid = results->getInt("songid");
    delete results;
    delete songidquery;
    return songid;
}

bool Commands::vote(char * username, char * songName, char* artist) {
    int user = getUserID(username);
    int song = getSongID(songName, artist);
    if (song == -1) {
        return false;
    }
    sql::PreparedStatement *statement;
    statement = con->prepareStatement("INSERT INTO votes(songid, userid) VALUES (?, ?)");
    statement->setInt(1, song);
    statement->setInt(2, user);
    statement->executeUpdate();
    delete statement;
    return true;
}

void Commands::revoke(char* username) {
    string user = username;
    sql::PreparedStatement *statement;
    statement = con->prepareStatement("UPDATE credentials SET restricted = 'Y' WHERE username = ?");
    statement->setString(1, user);
    statement->executeUpdate();
    delete statement;
}

bool Commands::isAdmin(char* username) {
    string name = username;
    sql::PreparedStatement *statement;
    sql::ResultSet *results;
    statement = con->prepareStatement("SELECT admin FROM credentials WHERE username  = ?");
    statement->setString(1, username);
    results = statement->executeQuery();
    results->next();
    string rights = results->getString("admin");
    delete results;
    delete statement;
    return (rights == "Y");
}

string Commands::top() {
    string topList = "Top songs from all genres:\n";
    sql::Statement *statement;
    sql::ResultSet *results;
    statement = con->createStatement();
    results = statement->executeQuery("select name, artist, count(*) as votes from songs natural join votes group by songid order by count(*) desc");
    int count = 0;
    while (results->next()){
        cout << ++count << "." << results->getString("name") << " - " << results->getString("artist") << " - " << results->getInt("votes") <<"\n";
        topList = topList + to_string(count) + ". " + results->getString("name") + " - "
                + results->getString("artist") + " - " + to_string(results->getInt("votes")) + "\n";
    }
    delete statement;
    delete results;
    return topList;
}

string Commands::top(char* genre) {
    string topList = "Top songs from your selected genre:\n";
    sql::PreparedStatement *statement;
    sql::ResultSet *results;
    int count = 0;
    statement = con->prepareStatement("select name, artist, count(*) as votes from (select songid, name, artist from songs natural join genres where genre = ?) as T natural join votes group by songid order by count(*) desc");
    statement->setString(1, genre);
    results = statement->executeQuery();
    while (results->next()){
        cout << ++count << "." << results->getString("name") << " - " << results->getString("artist") << " - " << results->getInt("votes") <<"\n";
        topList = topList + to_string(count) + ". " + results->getString("name") + " - "
                  + results->getString("artist") + " - " + to_string(results->getInt("votes")) + "\n";
    }
    delete statement;
    delete results;
    return topList;
}

bool Commands::makeAdmin(char* username) {
    if(getUserID(username) == -1) {
        return  false;
    }
    string user = username;
    sql::PreparedStatement *statement;
    statement = con->prepareStatement("UPDATE credentials SET admin= 'Y' WHERE username = ?");
    statement->setString(1, user);
    statement->executeUpdate();
    delete statement;
    return true;
}

bool Commands::removeVote(char* username, char* songName, char* artist) {
    int user = getUserID(username);
    int song = getSongID(songName, artist);
    sql::PreparedStatement *statement;
    sql::ResultSet *results;
    statement = con->prepareStatement("SELECT * FROM votes WHERE songid = ? and userid = ?");
    statement->setInt(1, song);
    statement->setInt(2, user);
    results = statement->executeQuery();
    if (results->rowsCount() == 0) {
        return false;
    }
    statement = con->prepareStatement("DELETE FROM votes WHERE songid = ? and userid = ?");
    statement->setInt(1, song);
    statement->setInt(2, user);
    statement->executeUpdate();
    delete statement;
    return true;
}

void Commands::comment(char* songName, char* artist, char* username, char* comment) {
    int song = getSongID(songName, artist);
    int user = getUserID(username);
    string comm = comment;
    sql::PreparedStatement *statement;
    statement = con->prepareStatement("INSERT INTO comments(songid, userid, comment) VALUES (?, ?, ?)");
    statement->setInt(1, song);
    statement->setInt(2, user);
    statement->setString(3, comment);
    statement->executeUpdate();
    delete statement;
}

bool Commands::canVote(char* username) {
    string name = username;
    sql::PreparedStatement *statement;
    sql::ResultSet *results;
    statement = con->prepareStatement("SELECT restricted FROM credentials WHERE username = ?");
    statement->setString(1, name);
    results = statement->executeQuery();
    results->next();
    string rights = results->getString("restricted");
    delete statement;
    delete results;
    return (rights == "N");
}

string Commands::showComments(char* songName, char* artist) {
    int songID = getSongID(songName, artist);
    string comments = "Comments for this song:\n";
    sql::PreparedStatement *statement;
    sql::ResultSet *results;
    statement = con->prepareStatement("select l.username, s.name, c.comment from credentials l natural join songs s join comments c on l.id = c.userid and s.songid = c.songid where s.songid = ?");
    statement->setInt(1, songID);
    results = statement->executeQuery();
    if (results->rowsCount() == 0){
       return "This song doesn't exist or there are no comments for it yet!\n";
    }
    while (results->next()){
            comments = comments + results->getString("username") + ": " + results->getString("comment") + "\n";
    }
    delete statement;
    delete results;
    return comments;
}

bool Commands::removeSong(char* songName, char* artist) {
    int songID = getSongID(songName, artist);
    if (songID == -1) {
        return false;
    }
    sql::PreparedStatement *statement;
    statement = con->prepareStatement("DELETE FROM genres WHERE songid = ?");
    statement->setInt(1, songID);
    statement->executeUpdate();
    statement = con->prepareStatement("DELETE FROM votes WHERE songid = ?");
    statement->setInt(1, songID);
    statement->executeUpdate();
    statement = con->prepareStatement("DELETE FROM songs WHERE songid = ?");
    statement->setInt(1, songID);
    statement->executeUpdate();
    statement = con->prepareStatement("DELETE FROM comments WHERE songid = ?");
    statement->setInt(1, songID);
    statement->executeUpdate();
    return true;
}
