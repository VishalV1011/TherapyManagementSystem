#ifndef STATISTICMANAGEMENT_H
#define STATISTICMANAGEMENT_H

#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include "DatabaseConnection.h"
#include "PaymentManager.h"
#include <string>

class StatisticManagement {
private:
    DatabaseConnection dbConn; // Database connection object for querying
    PaymentManager payment;
    // Helper methods to get the current year and month
    int getCurrentYear();
    int getCurrentMonth();

    // Tax rate for clinic earnings
    double taxRate = 0.1;
    int userID;
    std::string role;

public:

    // Constructor
    StatisticManagement();

    void setUserIDRole(int userID, const std::string& role);

    // Function to calculate clinic earnings
    void calculateEarnings();

    // Function to count bookings per month
    void countBookingsPerMonth();

    // Function to print payment receipt
    void printReceipt();
};

#endif
