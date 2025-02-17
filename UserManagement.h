#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include "DatabaseConnection.h"
#include "Features.h"
#include <iostream>
#include <iomanip>
#include <regex>   // for std::regex
#include <limits>
#include <ios>
#include <cstring>
#include <chrono>
#include <thread>

class UserManagement {
private:
    std::string username, newPassword, newName, password, loggedInUsername, role;
    int userID;
    bool enteredUsername;
    DatabaseConnection* db;
    void ensureRootUser();
    void createRootUser();
    std::unique_ptr<Features> feat;
public:
    UserManagement(DatabaseConnection* database);
    bool login(std::string& role);  // Use std::string
    void updateUserPassword();  // Use std::string
    void updateUserDetails(const std::string& role);
    void deleteUser();
    void viewUsers(const std::string& role);
    void registerUserWithDetails();
    int getUserID();
};

#endif // USERMANAGEMENT_H
