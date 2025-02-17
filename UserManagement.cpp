#include "UserManagement.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

// Initialize link to database
UserManagement::UserManagement(DatabaseConnection* database) :
    feat(std::make_unique<Features>()),  // Initialize feat using std::make_unique
    db(database)  // Initialize db with the provided database connection
{
    // Ensure the root user exists during initialization
    ensureRootUser();
}

int UserManagement::getUserID() {
    return this->userID;
}

// View all users
void UserManagement::viewUsers(const std::string& role) {
    // Base query to join tables
    std::string query;

    // If the user is an admin, show all users including guests, therapists, and patients
    if (role == "admin") {
        query = R"(
            SELECT 
                u.userID, 
                u.name, 
                u.username, 
                u.role,
                p.patientID, 
                t.therapistID,
                p.rating AS patient_rating, 
                t.seniority AS therapist_seniority, 
                t.specialization
            FROM user u
            LEFT JOIN patient p ON u.userID = p.userID
            LEFT JOIN therapist t ON u.userID = t.userID
            ORDER BY u.userID ASC
        )"; // Sorting by userID in ascending order
    }
    else if (role == "patient") {
        query = R"(
            SELECT 
                u.userID, 
                u.name, 
                u.username, 
                u.role, 
                p.patientID, 
                p.rating AS patient_rating
            FROM user u
            JOIN patient p ON u.userID = p.userID
            WHERE u.userID = )" + std::to_string(getUserID());
    }
    else if (role == "therapist") {
        query = R"(
            SELECT 
                u.userID, 
                u.name, 
                u.username, 
                u.role, 
                t.therapistID, 
                t.seniority AS therapist_seniority, 
                t.specialization
            FROM user u
            JOIN therapist t ON u.userID = t.userID
            WHERE u.userID = )" + std::to_string(getUserID());
    }
    else {
        std::cerr << "Invalid role specified.\n";
        return;
    }

    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (!res) {
        std::cerr << "No data found or an error occurred.\n";
        return;
    }

    // Print header
    std::cout << "\nUser Details:\n";
    std::cout << std::string(120, '-') << '\n';

    // Print the table headers
    std::cout << std::left
        << std::setw(12) << "User ID"
        << std::setw(30) << "Name"
        << std::setw(25) << "Username"
        << std::setw(15) << "Role"
        << std::setw(15) << "Patient ID"
        << std::setw(15) << "Therapist ID"
        << std::setw(20) << "Rating/Seniority"
        << std::setw(20) << "Specialization"
        << "\n";
    std::cout << std::string(120, '-') << '\n';

    // Print rows
    while ((row = mysql_fetch_row(res))) {
        // General information
        std::string userID = row[0] ? row[0] : "N/A";
        std::string name = row[1] ? row[1] : "N/A";
        std::string username = row[2] ? row[2] : "N/A";
        std::string currentRowRole = row[3] ? row[3] : "N/A";

        std::string patientID, therapistID, ratingOrSeniority, specialization;

        if (role == "admin") {
            // Admin view: all columns are present, check each row's role to display correct data
            patientID = row[4] ? row[4] : "N/A";
            therapistID = row[5] ? row[5] : "N/A";
            std::string patient_rating = row[6] ? row[6] : "N/A";
            std::string therapist_seniority = row[7] ? row[7] : "N/A";
            specialization = row[8] ? row[8] : "N/A";

            // Determine ratingOrSeniority based on the row's role
            if (currentRowRole == "patient") {
                ratingOrSeniority = patient_rating;
            }
            else if (currentRowRole == "therapist") {
                ratingOrSeniority = therapist_seniority;
            }
            else {
                ratingOrSeniority = "N/A";
            }
        }
        else if (role == "patient") {
            // Patient view: only their patientID and rating are present
            patientID = row[4] ? row[4] : "N/A";
            ratingOrSeniority = row[5] ? row[5] : "N/A";
            therapistID = "N/A";
            specialization = "N/A";
        }
        else if (role == "therapist") {
            // Therapist view: only their therapistID, seniority, and specialization are present
            therapistID = row[4] ? row[4] : "N/A";
            ratingOrSeniority = row[5] ? row[5] : "N/A";
            specialization = row[6] ? row[6] : "N/A";
            patientID = "N/A";
        }

        // Display the information in a table format
        std::cout << std::left
            << std::setw(12) << userID
            << std::setw(30) << name
            << std::setw(25) << username
            << std::setw(15) << currentRowRole
            << std::setw(15) << patientID
            << std::setw(15) << therapistID
            << std::setw(20) << ratingOrSeniority
            << std::setw(20) << specialization
            << '\n';
    }

    std::cout << std::string(120, '-') << '\n';

    mysql_free_result(res);

    // Wait for user input to continue
    while (true) {
        std::cout << "\nPress Enter to continue...";

        // Wait for user to press Enter
        std::cin.ignore();
        std::string input;
        std::getline(std::cin, input); // Read an entire line of input

        // Check if the input is empty (i.e., Enter was pressed without any other characters)
        if (input.empty()) {
            std::cout << "Continuing...\n";
            break;
        }
        else {
            std::cout << "Invalid input, please press Enter to continue.\n";
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
// User login
bool UserManagement::login(std::string& role) {
    std::string username;
    std::string password;

    // Ask for username and password directly inside the login function
    std::cout << "\nEnter username: ";
    std::cin >> username;
    feat->trim(username);
    std::cin.ignore();

    std::cout << "\nEnter password: ";
    password = feat->getPassword();  // Get password from Features class
    feat->trim(password);

    // SQL Query to retrieve both the role and userID
    std::string query = "SELECT userID, role FROM user WHERE username = '" + username + "' AND password = '" + password + "' AND role = '" + role + "'";
    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (res && (row = mysql_fetch_row(res))) {
        // Retrieve the userID and role
        this->userID = std::stoi(row[0]);  // Convert the first column (userID) to an integer
        role = row[1];  // Retrieve the role from the second column

        system("cls");
        std::cout << "\nLogin Successful! Role: " << role << ", UserID: " << userID << std::endl;

        loggedInUsername = username;

        mysql_free_result(res);
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
        return true;
    }
    else {
        system("cls");
        std::cout << "\nInvalid Username or Password!" << std::endl;
        if (res) mysql_free_result(res);
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
        return false;
    }
}

// Ensure root user exists
void UserManagement::ensureRootUser() {
    const char* userQuery = "SELECT COUNT(*) FROM user WHERE userID = 1 AND username = 'root';";
    MYSQL_RES* result = db->fetchResults(userQuery);
    MYSQL_ROW row;

    bool exists = false;
    if (result && (row = mysql_fetch_row(result))) {
        exists = std::stoi(row[0]) > 0;
    }
    if (result) mysql_free_result(result);

    if (!exists) {
        createRootUser();
    }
}

// Create root user if not yet created
void UserManagement::createRootUser() {
    const char* userQuery = "INSERT INTO user (userID, name, username, password, role) "
        "VALUES (1, 'Administrator', 'root', 'NextAge!0', 'admin');";
    if (!db->executeQuery(userQuery)) {
        std::cerr << "Failed to create root user!" << std::endl;
    }
    else {
        std::cout << "Root user created successfully." << std::endl;
    }
}

// Register new user
void UserManagement::registerUserWithDetails() {
    system("cls");
    std::string name, username, password, seniority, rating,specialization;
    int senioritySelection, specializationSelection;

    std::cout << "\n--- User Registration ---\n";
    std::cout << "Enter Name: ";
    std::cin.get();
    std::getline(std::cin, name);
    feat->trim(name);

    // Check for unique username
    bool usernameExists;
    do {
        std::cout << "\nEnter Username: ";
        std::getline(std::cin, username);
        feat->trim(username);

        // Check if username already exists in the database
        std::string checkUsernameQuery = "SELECT COUNT(*) FROM user WHERE username = '" + username + "'";
        MYSQL_RES* res = db->fetchResults(checkUsernameQuery);
        MYSQL_ROW row;
        if (res && (row = mysql_fetch_row(res))) {
            int count = std::stoi(row[0]);
            if (count > 0) {
                std::cout << "Username already exists! Please choose a different username.\n";
                usernameExists = true;
            }
            else {
                usernameExists = false;
            }
        }
        if (res) mysql_free_result(res);
    } while (usernameExists); // Continue prompting if username exists

    do {
        std::cout << "\nPassword must be:\n - At least 8 characters\n - At least one uppercase letter\n - At least one lowercase letter\n - At least one digit\n - At least one special character (e.g., !@#$%^&*())\n\nEnter Password: ";
        password = feat->getPassword();
        password = feat->trim(password);
    } while (!feat->isPasswordComplex(password));

    std::string role = "";
    int selection = -1;

    do {
        std::cout << "\nSelect role:\n1. Therapist\n2. Patient\n\nEnter selection:";
        std::cin >> selection;

        if (selection == 1) {
            role = "therapist";
            break;
        }
        else if (selection == 2) {
            role = "patient";
            break;
        }
        else {
            std::cout << "\n\nInvalid role selection!" << std::endl;
        }
    } while (selection == -1);

    // If role is therapist, ask for specialization and experience
    if (role == "therapist") {
        std::cout << "\nSeniority:\n1. Senior\n2. Intermediate\n3. Rookie\n\nSelect Seniority:";
        std::cin >> senioritySelection;

        switch (senioritySelection) {
        case 1: seniority = "senior"; break;
        case 2: seniority = "intermediate"; break;
        case 3: seniority = "rookie"; break;
        default:
            std::cout << "Invalid selection. Defaulting to 'Rookie'.\n";
            seniority = "rookie";
            break;
        }

        std::cout << "\nSpecialization:\n1. Psychology\n2. Physiology\n\nSelect Specilization:";
        std::cin >> specializationSelection;

        switch (specializationSelection) {
        case 1: specialization = "psychology"; break;
        case 2: specialization = "physiology"; break;
        default:
            std::cout << "Invalid selection. Defaulting to 'Psychology'.\n";
            specialization = "psychology";
            break;
        }
    }

    // If role is patient, ask for medical history
    if (role == "patient") {
        std::cout << "\nEnter Medical Rating (0-5): ";
        std::cin.ignore();
        std::getline(std::cin, rating);
        if (rating.empty() || std::stoi(rating) < 0 || std::stoi(rating) > 5) {
            std::cout << "Invalid rating. Defaulting to 0.\n";
            rating = "0";
        }
    }

    // Insert user into the user table
    std::string userQuery = "INSERT INTO user (name, username, password, role) VALUES ('" + name + "', '" + username + "', '" + password + "', '" + role + "')";
    if (!db->executeQuery(userQuery)) {
        std::cerr << "Error: Failed to register user!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for 2 seconds before clearing the screen
        system("cls");
        return;
    }

    std::cout << "User registered successfully!" << std::endl;

    // Wait for a few seconds before clearing the screen
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
    system("cls");

    // Get the userID of the newly created user
    std::string userIDQuery = "SELECT LAST_INSERT_ID()";
    MYSQL_RES* res = db->fetchResults(userIDQuery);
    MYSQL_ROW row;
    int userID = -1;

    if (res && (row = mysql_fetch_row(res))) {
        userID = std::stoi(row[0]);
    }
    else {
        std::cerr << "Error retrieving new user ID!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait before clearing the screen
        system("cls");
        return;
    }
    if (res) mysql_free_result(res);

    // Insert into therapist or patient table if applicable
    if (role == "therapist") {
        std::string therapistQuery = "UPDATE therapist SET seniority = '" + seniority +
            "', specialization = '" + specialization +
            "' WHERE userID = " + std::to_string(userID);
        if (db->executeQuery(therapistQuery)) {
            std::cout << "Therapist details saved successfully!" << std::endl;
        }
        else {
            std::cerr << "Error saving therapist details!" << std::endl;
        }
    }

    if (role == "patient") {
        std::string patientQuery = "UPDATE patient SET rating = " + rating + " WHERE userID = " + std::to_string(userID);
        if (db->executeQuery(patientQuery)) {
            std::cout << "Patient details saved successfully!" << std::endl;
        }
        else {
            std::cerr << "Error saving patient details!" << std::endl;
        }
    }

    // Wait before clearing the screen
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
}

// Update user password
void UserManagement::updateUserPassword() {
    int selectedUserID = -1;
    do {
        std::cout << "\n\nYou have selected Update User Password!" << std::endl;
        viewUsers("admin");
        std::cout << "\nEnter userID of user to update password or press 0 to cancel: ";
        std::cin >> selectedUserID;
        if (selectedUserID == 0) {
            exit;
        }
    } while (!feat->checkUserIDExists(selectedUserID));

    do {
        std::cout << "\nPassword must be:\n - At least 8 characters\n - At least one uppercase letter\n - At least one lowercase letter\n - At least one digit\n - At least one special character (e.g., !@#$%^&*())\n\nEnter Password: ";
        newPassword = feat->getPassword();
        newPassword = feat->trim(newPassword);
    } while (!feat->isPasswordComplex(newPassword));

    std::string query = "UPDATE user SET password = '" + newPassword + "' WHERE username = '" + username + "'";
    if (db->executeQuery(query)) {
        std::cout << "Password Updated Successfully!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
    }
    else {
        std::cerr << "Error Updating Password!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
    }
}

// Update user details
void UserManagement::updateUserDetails(const std::string& role) {
    system("cls");
    DatabaseConnection db;
    if (!db.checkConnection()) {
        std::cerr << "Error: Database connection failed.\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
        return;
    }

    std::string tableName, fieldName, newValue;
    bool updateSuccessful = false;

    std::cout << "\n\nYou have selected Update User Data!" << std::endl;

    // Prompt for user ID
    int selectedUserID = -1;
    viewUsers("admin");
    do {
        std::cout << "\nEnter the user ID to update data or press 0 to cancel: ";
        std::cin >> selectedUserID;
        if (selectedUserID == 0) {
            return;
        }
        else if (selectedUserID == 1) {
            std::cout << "\nUnable to edit root user details" << std::endl;
            return;
        }
    } while (!feat->checkUserIDExists(selectedUserID));

    // Retrieve and store the role of the selected user
    std::string userRoleQuery = "SELECT role FROM user WHERE userID = " + std::to_string(selectedUserID);
    auto roleResults = db.fetchQuery(userRoleQuery);
    if (roleResults.empty()) {
        std::cerr << "Error: User not found.\n";
        return;
    }
    std::string userRole = roleResults[0][0]; // Assume the first row, first column contains the role

    // Display selection menu for fields based on the user's role
    do {
        std::cout << "\nSelect the field you want to update:\n";

        if (userRole == "therapist") {
            std::cout << "1. Name\n";
            std::cout << "2. Username\n";
            std::cout << "3. Specialization\n";
            std::cout << "4. Seniority\n";
        }
        else if (userRole == "patient") {
            std::cout << "1. Name\n";
            std::cout << "2. Username\n";
            std::cout << "3. Rating\n";
        }

        std::cout << "Enter the number of the field to update: ";
        int choice;
        std::cin >> choice;

        // Map the choice to a field name
        if (userRole == "therapist") {
            switch (choice) {
            case 1: fieldName = "name"; break;
            case 2: fieldName = "username"; break;
            case 3: fieldName = "specialization"; break;
            case 4: fieldName = "seniority"; break;
            default: fieldName.clear();
            }
        }
        else if (userRole == "patient") {
            switch (choice) {
            case 1: fieldName = "name"; break;
            case 2: fieldName = "username"; break;
            case 3: fieldName = "rating"; break;
            default: fieldName.clear();
            }
        }

        if (fieldName.empty()) {
            std::cerr << "Invalid choice. Please select a valid field number.\n";
            continue;
        }
    } while (fieldName.empty());

    std::cin.ignore();
    // Prompt for the new value
    int selectOpt;
    if (fieldName == "specialization") {
        std::cout << "\nSpecialization:\n\n1. Psychology\n2. Physiology\n\nSelect new value to update " << fieldName << ": ";
        std::cin >> selectOpt;

        switch (selectOpt) {
        case 1:
            newValue = "psychology";
            break;
        case 2:
            newValue = "physiology";
            break;
        default:
            std::cout << "\n\nError in selection!" << std::endl;
        }
    }
    else if (fieldName == "seniority") {
        std::cout << "\nSeniority:\n\n1. Senior\n2. Intermediate\n3. Rookie\n\nSelect new value to update " << fieldName << ": ";
        std::cin >> selectOpt;

        switch (selectOpt) {
        case 1:
            newValue = "senior";
            break;
        case 2:
            newValue = "intermediate";
            break;
        case 3:
            newValue = "rookie";
            break;
        default:
            std::cout << "\n\nError in selection!" << std::endl;
        }
    }
    else {
        std::cout << "\nEnter the new value for " << fieldName << ": ";
        std::getline(std::cin, newValue);
    }

    // Determine the table name
    if (fieldName == "username") {
        tableName = "user";
    }
    else if (userRole == "therapist") {
        tableName = "therapist";
    }
    else if (userRole == "patient") {
        tableName = "patient";
    }
    else {
        std::cerr << "Error: Unable to determine the table for the selected field.\n";
        return;
    }

    // Build and execute the query
    std::string query = "UPDATE " + tableName + " SET " + fieldName + " = '" + newValue + "' WHERE userID = " + std::to_string(selectedUserID);
    updateSuccessful = db.executeQuery(query);

    if (updateSuccessful) {
        std::cout << "Data updated successfully!" << std::endl;
    }
    else {
        std::cerr << "Error updating data. Please check the input and try again." << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 2 seconds pause
}

// Delete user, check (view all corresponding deletes)
void UserManagement::deleteUser() {
    int selectedUserID = -1;
    std::string currentUser = loggedInUsername;
    std::string selectedUsername;  

    do {
        std::cout << "\n\nYou have selected Delete User!" << std::endl;
        viewUsers("admin");
        std::cout << "\nEnter userID of user to delete or press 0 to cancel: ";
        std::cin >> selectedUserID;
   

        if (selectedUserID == 0) {
            std::cout << "\nDelete operation canceled." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            return;
        }
    } while (!feat->checkUserIDExists(selectedUserID));


    // Confirmation step
    std::string confirmation;
    do {
        std::cin.ignore();
        std::cout << "Are you sure? (y/n): ";
        std::getline(std::cin, confirmation);
        feat->trim(confirmation);
    } while (confirmation != "y" && confirmation != "n");

    if (confirmation == "n") {
        std::cout << "Delete operation canceled." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
        return;
    }

    // Now delete from the main user table
    std::string deleteUserQuery = "DELETE FROM user WHERE userID = '" + std::to_string(selectedUserID) + "'";
    if (db->executeQuery(deleteUserQuery)) {
        std::cout << "User deleted successfully!" << std::endl;
    }
    else {
        std::cerr << "Error deleting user!" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
}
