#ifndef BOOKINGREQUEST_H
#define BOOKINGREQUEST_H

#include "DatabaseConnection.h"
#include "Features.h"
#include "PaymentManager.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <ctime>

class BookingRequest {
private:
    DatabaseConnection* db;
    PaymentManager payment;
    std::unique_ptr<Features> feat;
    int userID;
    int therapistID;
    int patientID;
    int bookingID;
    std::string bookingDate;
    std::string bookingTime;
    std::string role;
    int duration;
    std::string therapistName;
    std::string patientName;


public:
    BookingRequest(DatabaseConnection* database);
    void setUserIDRole(int userID, const std::string& role);
    void viewBookings();
    void updateBookings();

    // Admin functions
    void addAvailableSlot();
    void updateSlot();
    void deleteSlot();
    void viewAvailableSlots();

    // Therapist functions
    void therapistSelectSlot();
    void viewPatientDetails();
    void withdrawBooking();
    void getTherapistSlotDetails();

    // Patient functions
    void viewTherapistDetails();
    void bookSlot();
    void viewCatalogue();
};

#endif // BOOKINGREQUEST_H
