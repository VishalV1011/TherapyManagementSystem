#include "Features.h"
#include <iostream>
#include <conio.h>

Features::Features() {
    // Constructor body (can be empty if no initialization is needed)
}

// to hide password input
std::string Features::getPassword() {
    std::string input;
    char ch;

    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!input.empty()) {
                std::cout << "\b \b";
                input.pop_back();
            }
        }
        else {
            input.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return input;
}

// Ensuring Complex Password
bool Features::isPasswordComplex(const std::string& password) {
    if (password.length() < 8) {
        std::cout << "Password must be at least 8 characters long.\n";
        return false;
    }

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    // Check for character types
    for (char ch : password) {
        if (std::isupper(ch)) hasUpper = true;
        else if (std::islower(ch)) hasLower = true;
        else if (std::isdigit(ch)) hasDigit = true;
        else if (std::ispunct(ch)) hasSpecial = true;
    }

    if (!hasUpper) {
        std::cout << "Password must contain at least one uppercase letter.\n";
        return false;
    }
    if (!hasLower) {
        std::cout << "Password must contain at least one lowercase letter.\n";
        return false;
    }
    if (!hasDigit) {
        std::cout << "Password must contain at least one digit.\n";
        return false;
    }
    if (!hasSpecial) {
        std::cout << "Password must contain at least one special character (e.g., !@#$%^&*()).\n";
        return false;
    }

    return true;
}

// Define the trim function (To avoid blank space error)
std::string Features::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}

bool Features::checkUsernameExists(const std::string& username) {
    std::string query = "SELECT COUNT(*) FROM user WHERE username = '" + username + "'";
    MYSQL_RES* res = db.fetchResults(query);
    if (!res) {
        std::cerr << "Error: Failed to execute query.\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    int count = row ? std::stoi(row[0]) : 0;

    mysql_free_result(res);
    if (count > 0) {
        std::cout << "Username already exists! Please choose a different username.\n";
        return true;
    }

    return false;
}

bool Features::checkUserIDExists(int userID) {
    std::string query = "SELECT COUNT(*) FROM user WHERE userID = " + std::to_string(userID);
    MYSQL_RES* res = db.fetchResults(query);  // Assuming 'db' is your DatabaseConnection object

    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        int count = std::stoi(row[0]);
        mysql_free_result(res);
        return count > 0;  // Return true if user exists, false otherwise
    }
    std::cout << "\nUser with userID: " << userID << " doesn't exist!" << std::endl;
    return false;
}

int Features::getTherapistIDByUserID(int userID) {
    try {
        //if (db.checkConnection() == nullptr) {
        //    std::cerr << "Error: Database connection is null.\n";
        //    return -1; // Safeguard against a null database pointer
        //}

        if (userID == -1) {
            std::cerr << "Error: Invalid userID. Ensure userID is set correctly.\n";
            return -1;
        }

        // Construct the query to fetch therapistID
        std::string query = "SELECT therapistID FROM therapist WHERE userID = " + std::to_string(userID);

        // Fetch query results
        auto results = db.fetchQuery(query);

        // Check if the result is not empty
        if (!results.empty() && !results[0][0].empty()) {
            // Attempt to convert the first result to an integer
            std::cout << results[0][0];
            return std::stoi(results[0][0]);
        }
        else {
            std::cerr << "Error: No therapist found for userID " << userID << ".\n";
            return -1; // Indicate failure to find a therapist
        }
    }
    catch (const std::exception& e) {
        // Catch and log any exceptions that occur
        std::cerr << "Exception occurred while fetching therapistID: " << e.what() << std::endl;
        return -1; // Indicate failure
    }
}

int Features::getPatientIDByUserID(int userID) {
    std::string query = "SELECT patientID FROM patient WHERE userID = " + std::to_string(userID);
    auto results = db.fetchQuery(query);

    if (!results.empty()) {
        return std::stoi(results[0][0]); // Assuming the query returns a valid patientID
    }
    else {
        std::cerr << "Error: No patient found for userID " << userID << std::endl;
        return -1; // Return -1 if no patientID is found
    }
}
