#include "StatisticManagement.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>

// Constructor
StatisticManagement::StatisticManagement() : payment() {
    if (!dbConn.checkConnection()) {
        throw std::runtime_error("Database connection failed.");
    }
}

void StatisticManagement::setUserIDRole(int userID, const std::string& role) {
    this->userID = userID;
    this->role = role;
}

// Function to calculate clinic earnings
void StatisticManagement::calculateEarnings() {
    bool continueRunning = true;
    while (continueRunning) {
        // Query to calculate total payments per month based on booking created_at
        std::string query = R"(
            SELECT MONTH(b.created_at) AS month, 
                    YEAR(b.created_at) AS year, 
                    SUM(p.totalAmount) AS totalPayments
            FROM payments p
            JOIN booking b ON p.bookingID = b.bookingID
            WHERE p.paymentStatus = 'Completed'
            GROUP BY year, month;
        )";

        auto results = dbConn.fetchQuery(query);
        if (results.empty()) {
            std::cout << "No payment data available to calculate earnings.\n";
            return;
        }

        // Fetch therapist salaries based on seniority
        std::map<std::string, int> therapistSalaries = { {"senior", 10000}, {"intermediate", 6500}, {"rookie", 3500} };
        std::string salaryQuery = "SELECT seniority, COUNT(*) AS count FROM therapist GROUP BY seniority;";
        auto salaryResults = dbConn.fetchQuery(salaryQuery);

        int totalMonthlySalary = 0;
        for (const auto& row : salaryResults) {
            std::string seniority = row[0];
            int count = std::stoi(row[1]);
            totalMonthlySalary += count * therapistSalaries[seniority];
        }

        // Calculate monthly earnings
        std::cout << "Month-Year | Total Payments | Total Salary | Net Earnings\n";
        std::cout << "---------------------------------------------------------\n";
        for (const auto& row : results) {
            int month = std::stoi(row[0]);
            int year = std::stoi(row[1]);
            double totalPayments = std::stod(row[2]);
            double netEarnings = totalPayments - totalMonthlySalary;

            std::cout << std::setw(10) << month << "-" << year << " | "
                << std::setw(14) << totalPayments << " | "
                << std::setw(12) << totalMonthlySalary << " | "
                << std::setw(12) << netEarnings << "\n";
        }

        std::cout << "Press Enter to exit or any other key to calculate again.\n";
        std::cin.ignore(); // Consume the newline left in the buffer after user input
        std::string userInput;
        std::getline(std::cin, userInput);
        if (userInput.empty()) {
            continueRunning = false;
        }
    }
}

// Function to count bookings per month
void StatisticManagement::countBookingsPerMonth() {
    bool continueRunning = true;
    while (continueRunning) {
        // Query to count bookings per month based on session status
        std::string query = R"(
            SELECT MONTH(created_at) AS month, 
                   YEAR(created_at) AS year, 
                   COUNT(*) AS totalBookings
            FROM booking
            GROUP BY year, month
            ORDER BY year, month;
        )";

        auto results = dbConn.fetchQuery(query);
        if (results.empty()) {
            std::cout << "No booking data available.\n";
            return;
        }

        std::cout << "Month-Year | Total Bookings | Graph\n";
        std::cout << "---------------------------------------\n";

        // Print each month's bookings with a simple line graph
        for (const auto& row : results) {
            int month = std::stoi(row[0]);
            int year = std::stoi(row[1]);
            int totalBookings = std::stoi(row[2]);

            // Convert month number to month name
            std::string monthName;
            switch (month) {
            case 1: monthName = "Jan"; break;
            case 2: monthName = "Feb"; break;
            case 3: monthName = "Mar"; break;
            case 4: monthName = "Apr"; break;
            case 5: monthName = "May"; break;
            case 6: monthName = "Jun"; break;
            case 7: monthName = "Jul"; break;
            case 8: monthName = "Aug"; break;
            case 9: monthName = "Sep"; break;
            case 10: monthName = "Oct"; break;
            case 11: monthName = "Nov"; break;
            case 12: monthName = "Dec"; break;
            default: monthName = "Unknown"; break;
            }

            // Print the month-year and total bookings
            std::cout << std::setw(9) << monthName << "-" << year << " | "
                << std::setw(14) << totalBookings;

            // Generate the simple graph (line of asterisks)
            int numAsterisks = totalBookings; // Scale down the number of asterisks for visibility
            std::cout << " | ";
            for (int i = 0; i < numAsterisks; ++i) {
                std::cout << "*";
            }

            std::cout << "\n";
        }

        std::cout << "Press Enter to exit or any other key to count bookings again.\n";
        std::cin.ignore(); // Consume the newline left in the buffer after user input
        std::string userInput;
        std::getline(std::cin, userInput);
        if (userInput.empty()) {
            continueRunning = false;
        }
    }
}

// Function to print payment receipt
void StatisticManagement::printReceipt() {
    bool continueRunning = true;
    while (continueRunning) {
        int paymentID;

        try {
            payment.viewAllPayments(role);
            // Query to fetch payment details
            std::cout << "Enter paymentID of selected payment: ";
            std::cin >> paymentID;

            std::ostringstream query;
            query << "SELECT p.paymentID, p.totalAmount, p.paymentDateTime, b.patientID, p.paymentDateTime "
                << "FROM payments p "
                << "JOIN booking b ON p.bookingID = b.bookingID "
                << "WHERE p.paymentID = " << paymentID << " AND p.paymentStatus = 'Completed';";

            auto results = dbConn.fetchQuery(query.str());
            if (results.empty()) {
                std::cout << "No receipt found for payment ID: " << paymentID << "\n";
                return;
            }

            // Print receipt details in a medical receipt format
            const auto& receipt = results[0];

            // Header with clinic information
            std::cout << "------------------------------------------------------------\n";
            std::cout << "                HEALTHCARE CLINIC NAME                     \n";
            std::cout << "                 Address: 123 Health St, City, Zip         \n";
            std::cout << "                 Phone: (123) 456-7890                     \n";
            std::cout << "------------------------------------------------------------\n";

            // Receipt title
            std::cout << "                     PAYMENT RECEIPT                        \n";
            std::cout << "------------------------------------------------------------\n";

            // Payment Information
            std::cout << "Receipt ID: " << receipt[0] << "\n";
            std::cout << "Patient ID: " << receipt[3] << "\n";
            std::cout << "Date of Payment: " << receipt[4] << "\n";
            std::cout << "------------------------------------------------------------\n";

            // Payment summary
            std::cout << "Total Amount: $" << receipt[1] << "\n";
            std::cout << "------------------------------------------------------------\n";

            // Footer with additional information
            std::cout << "Thank you for choosing our clinic for your healthcare needs. \n";
            std::cout << "Please keep this receipt for your records. \n";
            std::cout << "------------------------------------------------------------\n";
            std::cout << "                   * Therapy Clinic *               \n";
            std::cout << "------------------------------------------------------------\n";

        }
        catch (const std::exception& e) {
            std::cerr << "Error viewing payment details: " << e.what() << std::endl;
            return;
        }

        std::cout << "Press Enter to exit or any other key to print another receipt.\n";
        std::cin.ignore(); // Consume the newline left in the buffer after user input
        std::string userInput;
        std::getline(std::cin, userInput);
        if (userInput.empty()) {
            continueRunning = false;
        }
    }
}
