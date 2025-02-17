#ifndef FEATURES_H
#define FEATURES_H

#include <string>
#include "DatabaseConnection.h"

class Features {
public:
    // Constructor
    Features();
    DatabaseConnection db;

    // Function to mask text input
    std::string getPassword();
    bool isPasswordComplex(const std::string& password);
    std::string trim(const std::string& str);
    bool checkUsernameExists(const std::string& username);
    bool checkUserIDExists(int userID);
    int getTherapistIDByUserID(int userID);
    int getPatientIDByUserID(int userID);
};

#endif // FEATURES_H
