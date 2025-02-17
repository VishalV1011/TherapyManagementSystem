#include "PaymentManager.h"
#include <iostream>
#include <unordered_map>
#include <mysql.h>
#include <chrono>
#include <thread>

// Mock implementation for session-based methods
void PaymentManager::setUserIDRole(int userID, const std::string& role) {
    this->userID = userID;
    this->role = role;
}

int PaymentManager::getTherapistIDFromUserID() {
    try {
        std::string query = "SELECT therapistID FROM therapist WHERE userID = " + std::to_string(userID);
        MYSQL_RES* res = db.fetchResults(query);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row && row[0] != nullptr) {
                int therapistID = std::stoi(row[0]);
                mysql_free_result(res);
                return therapistID;
            }
            mysql_free_result(res);
        }
        return -1; // Return -1 if therapistID is not found
    }
    catch (const std::exception& e) {
        std::cerr << "Error retrieving therapist ID: " << e.what() << std::endl;
        return -1;
    }
}

int PaymentManager::getPatientIDFromUserID() {
    try {
        std::string query = "SELECT patientID FROM patient WHERE userID = " + std::to_string(userID);
        MYSQL_RES* res = db.fetchResults(query);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row && row[0] != nullptr) {
                int patientID = std::stoi(row[0]);
                mysql_free_result(res);
                return patientID;
            }
            mysql_free_result(res);
        }
        return -1; // Return -1 if patientID is not found
    }
    catch (const std::exception& e) {
        std::cerr << "Error retrieving patient ID: " << e.what() << std::endl;
        return -1;
    }
}

void PaymentManager::viewAvailableTherapists() {
    try {
        std::string query = "SELECT therapistID, name, seniority, available_date, available_time FROM therapist WHERE available_date >= CURDATE() AND available_time >= CURTIME()";

        MYSQL_RES* res = db.fetchResults(query);
        if (res) {
            MYSQL_ROW row;
            std::cout << "Available Therapists:\n";
            std::cout << "--------------------------------------------------------\n";
            std::cout << "TherapistID | Name            | Seniority | Available Date | Available Time\n";
            std::cout << "--------------------------------------------------------\n";

            while ((row = mysql_fetch_row(res))) {
                if (row[0] != nullptr && row[1] != nullptr && row[2] != nullptr && row[3] != nullptr && row[4] != nullptr) {
                    std::cout << row[0] << "        | " << row[1] << " | " << row[2] << "    | "
                        << row[3] << "   | " << row[4] << "\n";
                }
                else {
                    std::cerr << "Warning: Missing data in row.\n";
                }
            }

            mysql_free_result(res);
        }
        else {
            std::cout << "No available therapists found.\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error viewing available therapists: " << e.what() << std::endl;
    }
}

// View pending payments (admin sees all, patients see their own)
bool PaymentManager::viewPendingPayments() {
    try {
        int patientID = getPatientIDFromUserID();
        std::string query;

        // Query for admin to see all pending payments
        if (role == "admin") {
            query = "SELECT p.paymentID, p.patientID, p.totalAmount, p.paymentStatus, p.bookingID, p.paymentDateTime "
                "FROM payments p WHERE p.paymentStatus = 'Pending'";
        }
        // Query for patient to see their own pending payments
        else {
            query = "SELECT p.paymentID, p.patientID, p.totalAmount, p.paymentStatus, p.bookingID, p.paymentDateTime "
                "FROM payments p WHERE p.patientID = " + std::to_string(patientID) + " AND p.paymentStatus = 'Pending'";
        }

        MYSQL_RES* res = db.fetchResults(query);
        if (res) {
            MYSQL_ROW row;
            std::cout << "+------------+------------+-------------+-----------------+------------+---------------------+" << std::endl;
            std::cout << "| PaymentID | PatientID  | TotalAmount |   Status        | BookingID  | PaymentDateTime      |" << std::endl;
            std::cout << "+------------+------------+-------------+-----------------+------------+---------------------+" << std::endl;

            while ((row = mysql_fetch_row(res))) {
                if (row[0] != nullptr && row[1] != nullptr && row[2] != nullptr && row[3] != nullptr &&
                    row[4] != nullptr && row[5] != nullptr) {
                    std::cout << "| " << std::setw(10) << row[0]
                        << " | " << std::setw(10) << row[1]
                        << " | " << std::setw(11) << row[2]
                        << " | " << std::setw(15) << row[3]
                        << " | " << std::setw(10) << row[4]
                        << " | " << std::setw(19) << row[5]
                        << " |" << std::endl;
                }
                else {
                    std::cerr << "Warning: Missing data in payment record.\n";
                }
            }

            std::cout << "+------------+------------+-------------+-----------------+------------+---------------------+" << std::endl;
            mysql_free_result(res);
            return true;
        }

        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error viewing pending payments: " << e.what() << std::endl;
        return false;
    }
}

// View all payments 
bool PaymentManager::viewAllPayments(const std::string& role) {
    try {
        std::string query;
        // Admin can view all payments
        if (role == "admin") {
            query = "SELECT p.paymentID, p.patientID, p.totalAmount, p.paymentStatus, p.bookingID, p.paymentDateTime "
                "FROM payments p";
        }
        // Patient can only view their own payments
        else if (role == "patient") {
            int patientID = getPatientIDFromUserID();
            query = "SELECT p.paymentID, p.patientID, p.totalAmount, p.paymentStatus, p.bookingID, p.paymentDateTime "
                "FROM payments p WHERE p.patientID = " + std::to_string(patientID);
        }
        else {
            std::cerr << "Access denied. Invalid role." << std::endl;
            return false;
        }

        MYSQL_RES* res = db.fetchResults(query);
        if (res) {
            MYSQL_ROW row;
            std::cout << "+------------+------------+-------------+-----------------+------------+---------------------+" << std::endl;
            std::cout << "| PaymentID | PatientID  | TotalAmount |   Status        | BookingID  | PaymentDateTime      |" << std::endl;
            std::cout << "+------------+------------+-------------+-----------------+------------+---------------------+" << std::endl;

            while ((row = mysql_fetch_row(res))) {
                std::cout << "| " << std::setw(10) << (row[0] ? row[0] : "N/A")
                    << " | " << std::setw(10) << (row[1] ? row[1] : "N/A")
                    << " | " << std::setw(11) << (row[2] ? row[2] : "N/A")
                    << " | " << std::setw(15) << (row[3] ? row[3] : "N/A")
                    << " | " << std::setw(10) << (row[4] ? row[4] : "N/A")
                    << " | " << std::setw(19) << (row[5] ? row[5] : "N/A")
                    << " |" << std::endl;
            }

            std::cout << "+------------+------------+-------------+-----------------+------------+---------------------+" << std::endl;
            mysql_free_result(res);

            return true;
        }
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error viewing all payments: " << e.what() << std::endl;
        return false;
    }
}

// Delete payment (admin only)
bool PaymentManager::deletePayment() {
    try {
        if (role != "admin") {
            std::cerr << "Access denied. Only admins can delete payments." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        viewAllPayments(role);
        int paymentID;
        std::cout << "Select paymentID to delete: ";
        std::cin >> paymentID;
        std::string query = "DELETE FROM payments WHERE paymentID = " + std::to_string(paymentID);
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for 2 seconds
        return db.executeQuery(query);
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting payment: " << e.what() << std::endl;
        return false;
    }
}

// Update payment details (admin only)
bool PaymentManager::updatePayment() {
    try {
        if (role != "admin") {
            std::cerr << "Access denied. Only admins can update payments." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        int paymentID;
        viewAllPayments(role);
        std::cout << "Enter Payment ID to update: ";
        std::cin >> paymentID;

        std::string query = "SELECT * FROM payments WHERE paymentID = " + std::to_string(paymentID);
        MYSQL_RES* res = db.fetchResults(query);

        if (!res) {
            std::cerr << "Invalid Payment ID. Update aborted." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row || row[2] == nullptr || row[3] == nullptr) {
            std::cerr << "No valid payment found with ID " << paymentID << ". Update aborted." << std::endl;
            mysql_free_result(res);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        double currentTotalAmount = row[2] ? std::stod(row[2]) : 0.0;
        std::string currentStatus = row[3] ? row[3] : "Unknown";

        std::cout << "Current Payment Details:\n";
        std::cout << "PaymentID: " << row[0] << ", PatientID: " << row[1]
            << ", TotalAmount: " << currentTotalAmount
            << ", Status: " << currentStatus << "\n";

        mysql_free_result(res);

        // Variables for new amounts
        std::string newTotalAmountInput;
        double newTotalAmount = currentTotalAmount;

        std::cout << "Enter new total amount (leave empty to keep current): ";
        std::cin.ignore();  // Clear input buffer
        std::getline(std::cin, newTotalAmountInput);
        if (!newTotalAmountInput.empty()) {
            newTotalAmount = std::stod(newTotalAmountInput);
        }

        // Construct and execute the update query
        query = "UPDATE payments SET totalAmount = " + std::to_string(newTotalAmount) +
            ", paymentStatus = '" + currentStatus + "' WHERE paymentID = " + std::to_string(paymentID);

        if (db.executeQuery(query)) {
            std::cout << "Payment ID " << paymentID << " updated successfully.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return true;
        }
        else {
            std::cerr << "Failed to update payment ID " << paymentID << ". Please try again.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating payment: " << e.what() << std::endl;
        return false;
    }
}

bool PaymentManager::continuePaymentProcess() {
    try {
        if (role != "patient") {
            std::cerr << "Access denied. Only patients can continue payment processes." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        int patientID = getPatientIDFromUserID();

        std::cout << "Your current payment records:\n";
        std::string query = "SELECT * FROM payments WHERE patientID = " + std::to_string(patientID);
        MYSQL_RES* res = db.fetchResults(query);

        if (!res) {
            std::cerr << "No payments found for your account. Process aborted." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        MYSQL_ROW row;
        std::cout << "------------------------------------------------------------\n";
        std::cout << "| PaymentID | TotalAmount | Status | BookingID | PaymentDateTime |\n";
        std::cout << "------------------------------------------------------------\n";

        while ((row = mysql_fetch_row(res))) {
            if (row[0] != nullptr && row[2] != nullptr && row[3] != nullptr && row[4] != nullptr && row[5] != nullptr) {
                std::cout << "| " << std::setw(10) << row[0]
                    << " | " << std::setw(11) << row[2]
                    << " | " << std::setw(6) << row[3]
                    << " | " << std::setw(9) << row[4]
                    << " | " << std::setw(17) << row[5] << " |\n";
            }
            else {
                std::cerr << "Warning: Missing data in payment record.\n";
            }
        }

        std::cout << "------------------------------------------------------------\n";
        mysql_free_result(res);

        int paymentID;
        std::cout << "Enter Payment ID to continue payment: ";
        std::cin >> paymentID;

        query = "SELECT totalAmount, paymentStatus FROM payments WHERE paymentID = " + std::to_string(paymentID) + " AND patientID = " + std::to_string(patientID);
        res = db.fetchResults(query);

        if (!res || mysql_num_rows(res) == 0) {
            std::cerr << "Invalid Payment ID or you are not authorized to continue this payment process.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        row = mysql_fetch_row(res);
        if (row[1] != nullptr && std::string(row[1]) == "Completed") {
            std::cout << "Payment already completed.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            return false;
        }

        double totalAmount = 0.0;
        if (row[0] != nullptr) {
            totalAmount = std::stod(row[0]);
        }
        else {
            std::cerr << "Error: Missing totalAmount in payment record.\n";
            mysql_free_result(res);
            return false;
        }

        std::cout << "Total amount due: " << totalAmount << "\n";
        std::cout << "Proceeding to payment gateway...\n";

        std::this_thread::sleep_for(std::chrono::seconds(3)); // Simulate payment processing
        std::cout << "Payment successful!\n";

        query = "UPDATE payments SET paymentStatus = 'Completed' WHERE paymentID = " + std::to_string(paymentID);
        db.executeQuery(query);

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait before clearing the screen
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error continuing payment process: " << e.what() << std::endl;
        return false;
    }
}

double PaymentManager::calculatePayment(int bookingID) {
    // Mapping of seniority to hourly rate
    std::unordered_map<std::string, double> seniorityRates = {
        {"rookie", 125.0},
        {"intermediate", 250.0},
        {"senior", 400.0}
    };

    try {
        // Ensure the database connection is active
        if (!db.checkConnection()) {
            throw std::runtime_error("Database connection failed");
        }

        // Query to get slotID and patientID from booking
        std::string queryBooking = "SELECT slotID, patientID FROM booking WHERE bookingID = " + std::to_string(bookingID);
        auto bookingResult = db.fetchQuery(queryBooking);
        if (bookingResult.empty()) {
            throw std::runtime_error("Booking not found");
        }

        int slotID = std::stoi(bookingResult[0][0]);
        int patientID = std::stoi(bookingResult[0][1]);

        // Query to get available_duration and type from available_slots
        std::string querySlot = "SELECT available_duration, type FROM available_slots WHERE slotID = " + std::to_string(slotID);
        auto slotResult = db.fetchQuery(querySlot);
        if (slotResult.empty()) {
            throw std::runtime_error("Slot not found");
        }

        int duration = std::stoi(slotResult[0][0]);
        std::string slotType = slotResult[0][1];

        // Query to get therapistID from therapist_availability_mapping
        std::string queryTherapist = "SELECT therapistID FROM therapist_availability_mapping WHERE slotID = " + std::to_string(slotID);
        auto therapistResult = db.fetchQuery(queryTherapist);
        if (therapistResult.empty()) {
            throw std::runtime_error("Therapist not found");
        }

        int therapistID = std::stoi(therapistResult[0][0]);

        // Query to get seniority of the therapist
        std::string querySeniority = "SELECT seniority FROM therapist WHERE therapistID = " + std::to_string(therapistID);
        auto seniorityResult = db.fetchQuery(querySeniority);
        if (seniorityResult.empty()) {
            throw std::runtime_error("Therapist seniority not found");
        }

        std::string seniority = seniorityResult[0][0];

        // Calculate payment based on seniority and duration
        if (seniorityRates.find(seniority) == seniorityRates.end()) {
            throw std::runtime_error("Invalid therapist seniority");
        }

        double rate = seniorityRates[seniority];
        double payment = rate * duration / 60.0; // Convert duration to hours

        // Apply a 25% discount if the slot type is 'group'
        if (slotType == "group") {
            payment *= 0.75; // 25% discount
        }

        // Insert the payment into the payments table
        std::string insertPaymentQuery =
            "INSERT INTO payments (patientID, totalAmount, paymentStatus, bookingID, paymentDateTime) "
            "VALUES (" + std::to_string(patientID) + ", " +
            std::to_string(payment) + ", 'Pending', " +
            std::to_string(bookingID) + ", NOW())";

        if (!db.executeQuery(insertPaymentQuery)) {
            throw std::runtime_error("Failed to insert payment record");
        }

        return payment;
    }
    catch (const std::exception& e) {
        std::cerr << "Error calculating payment: " << e.what() << std::endl;
        return -1.0; // Indicate an error
    }
}
