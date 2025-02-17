#ifndef PAYMENTMANAGER_H
#define PAYMENTMANAGER_H

#include "DatabaseConnection.h"
#include "Features.h"
#include <string>
#include <thread>
#include <chrono>
#include <ctime>

class PaymentManager {
private:
    DatabaseConnection db; // Instance of the database connection class
    std::unique_ptr<Features> feat;
    double calculatePayment(); // Calculate payment based on therapist's seniority and session duration
    int userID;
    std::string role;
public:
    int getTherapistIDFromUserID();
    int getPatientIDFromUserID();
    void viewAvailableTherapists();
    void setUserIDRole(int userID, const std::string& role);  // Retrieve the current logged-in user's ID
    bool isAdmin();               // Check if the current user has admin privileges
    bool viewPendingPayments();   // View pending payments (admin sees all, patients see their own)
    bool viewAllPayments(const std::string& role);       // View all payments (admin only)
    bool deletePayment();         // Delete payment (admin only)
    bool updatePayment();         // Update payment details (admin only)
    bool continuePaymentProcess(); // Continue payment process (patient only)
    double calculatePayment(int bookingID);
};

#endif // PAYMENTMANAGER_H
