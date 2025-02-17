#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <vector>
#include <string>
#include <mysql.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

class DatabaseConnection {
private:
    MYSQL* conn;

public:
    DatabaseConnection();
    ~DatabaseConnection();

    bool checkConnection();
    bool executeQuery(const std::string& query);
    MYSQL_RES* fetchResults(const std::string& query);
    std::vector<std::vector<std::string>> fetchQuery(const std::string& query); // New method
    MYSQL* getConnection();
};
#endif