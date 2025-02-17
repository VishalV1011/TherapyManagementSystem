#include "BookingRequest.h"

BookingRequest::BookingRequest(DatabaseConnection* database)
    : db(database), therapistID(-1), patientID(-1), duration(0) {
}

int BookingRequest::selectTherapist() {
    std::string query = "SELECT therapistID, name FROM therapist";
    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (res) {
        std::vector<int> therapistIDs;
        int index = 1;
        std::cout << "Available Therapists:\n";

        while ((row = mysql_fetch_row(res))) {
            therapistIDs.push_back(std::stoi(row[0]));
            std::cout << index++ << ". " << row[1] << " (ID: " << row[0] << ")\n";
        }

        int choice;
        std::cout << "Select Therapist (Enter number): ";
        std::cin >> choice;

        if (choice < 1 || choice > therapistIDs.size()) {
            std::cerr << "Invalid choice!\n";
            mysql_free_result(res);
            return -1;
        }

        mysql_free_result(res);
        return therapistIDs[choice - 1];
    }
    else {
        std::cerr << "No therapists available.\n";
        return -1;
    }
}

int BookingRequest::selectPatient() {
    std::string query = "SELECT patientID, name FROM patient";
    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (res) {
        std::vector<int> patientIDs;
        int index = 1;
        std::cout << "Available Patients:\n";

        while ((row = mysql_fetch_row(res))) {
            patientIDs.push_back(std::stoi(row[0]));
            std::cout << index++ << ". " << row[1] << " (ID: " << row[0] << ")\n";
        }

        int choice;
        std::cout << "Select Patient (Enter number): ";
        std::cin >> choice;

        if (choice < 1 || choice > patientIDs.size()) {
            std::cerr << "Invalid choice!\n";
            mysql_free_result(res);
            return -1;
        }

        mysql_free_result(res);
        return patientIDs[choice - 1];
    }
    else {
        std::cerr << "No patients available.\n";
        return -1;
    }
}

void BookingRequest::setTherapistAvailability() {
    therapistID = selectTherapist();
    if (therapistID == -1) return;

    std::cout << "Enter Available Date (YYYY-MM-DD): ";
    std::cin >> bookingDate;
    std::cout << "Enter Available Time (HH:MM): ";
    std::cin >> bookingTime;
    std::cout << "Enter Duration (in minutes): ";
    std::cin >> duration;

    std::string query = "UPDATE therapist SET available_date = '" + bookingDate +
        "', available_time = '" + bookingTime +
        "', available_duration = " + std::to_string(duration) +
        " WHERE therapistID = " + std::to_string(therapistID);
    if (db->executeQuery(query)) {
        std::cout << "Therapist availability set successfully.\n";
    }
    else {
        std::cerr << "Error setting therapist availability.\n";
    }
}

void BookingRequest::informPatientAboutAvailability() {
    patientID = selectPatient();
    if (patientID == -1) return;

    std::string query = "SELECT therapistID, available_date, available_time, available_duration FROM therapist WHERE therapistID = " + std::to_string(therapistID);
    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (res && (row = mysql_fetch_row(res))) {
        std::cout << "Therapist availability:\n"
            << "Therapist ID: " << row[0] << "\n"
            << "Date: " << row[1] << "\n"
            << "Time: " << row[2] << "\n"
            << "Duration: " << row[3] << " minutes\n";
    }
    else {
        std::cerr << "Error fetching availability.\n";
    }

    if (res) mysql_free_result(res);
}

void BookingRequest::patientConfirmsBooking() {
    therapistID = selectTherapist();
    if (therapistID == -1) return;

    patientID = selectPatient();
    if (patientID == -1) return;

    std::cout << "Enter Booking Date (YYYY-MM-DD): ";
    std::cin >> bookingDate;
    std::cout << "Enter Booking Time (HH:MM): ";
    std::cin >> bookingTime;
    std::cout << "Enter Duration (in minutes): ";
    std::cin >> duration;

    std::string query = "INSERT INTO booking (therapistID, patientID, booking_date, booking_time, duration, status) "
        "VALUES (" + std::to_string(therapistID) + ", " + std::to_string(patientID) + ", '" + bookingDate +
        "', '" + bookingTime + "', " + std::to_string(duration) + ", 'confirmed')";
    if (db->executeQuery(query)) {
        std::cout << "Booking confirmed by patient.\n";
    }
    else {
        std::cerr << "Error confirming booking.\n";
    }
}

void BookingRequest::adminConfirmsBooking() {
    std::cout << "Admin: Confirming a Booking\n";

    // First, view the existing bookings to get a list of booking IDs
    viewBookingDetails();

    // Prompt the admin to select a booking ID to confirm
    int bookingID;
    std::cout << "Enter Booking ID to Confirm: ";
    std::cin >> bookingID;

    // Construct the SQL query to update the status of the selected booking
    std::string query = "UPDATE booking SET status = 'confirmed' WHERE bookingID = " + std::to_string(bookingID);

    // Execute the query and provide feedback to the admin
    if (db->executeQuery(query)) {
        std::cout << "Booking ID " << bookingID << " confirmed successfully.\n";
    }
    else {
        std::cerr << "Error confirming booking ID " << bookingID << ". Please try again.\n";
    }
}

void BookingRequest::viewBookingDetails() {
    std::string query = "SELECT booking.bookingID, user1.name AS therapist_name, user2.name AS patient_name, booking.booking_date, booking.booking_time, booking.duration, booking.status "
        "FROM booking_request "
        "JOIN user AS user1 ON booking_request.therapistID = user1.userID "
        "JOIN user AS user2 ON booking_request.patientID = user2.userID";
    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (res) {
        std::cout << "Booking Details:\n";
        while ((row = mysql_fetch_row(res))) {
            std::cout << "Booking ID: " << row[0] << "\n"
                << "Therapist: " << row[1] << "\n"
                << "Patient: " << row[2] << "\n"
                << "Date: " << row[3] << "\n"
                << "Time: " << row[4] << "\n"
                << "Duration: " << row[5] << " minutes\n"
                << "Status: " << row[6] << "\n\n";
        }
        mysql_free_result(res);
    }
    else {
        std::cerr << "No booking details found!" << std::endl;
    }
}

void BookingRequest::updateBookingDetails() {
    viewBookingDetails();

    int bookingID;
    std::cout << "Enter Booking ID to Update: ";
    std::cin >> bookingID;

    std::cout << "Enter New Booking Date (YYYY-MM-DD): ";
    std::cin >> bookingDate;
    std::cout << "Enter New Booking Time (HH:MM): ";
    std::cin >> bookingTime;
    std::cout << "Enter New Duration (in minutes): ";
    std::cin >> duration;

    std::string query = "UPDATE booking SET booking_date = '" + bookingDate +
        "', booking_time = '" + bookingTime +
        "', duration = " + std::to_string(duration) +
        " WHERE bookingID = " + std::to_string(bookingID);
    if (db->executeQuery(query)) {
        std::cout << "Booking details updated successfully!" << std::endl;
    }
    else {
        std::cerr << "Error updating booking details!" << std::endl;
    }
}

void BookingRequest::deleteBooking() {
    viewBookingDetails();

    int bookingID;
    std::cout << "Enter Booking ID to Delete: ";
    std::cin >> bookingID;

    // Confirm deletion
    char confirmation;
    std::cout << "Are you sure you want to delete Booking ID " << bookingID << "? (y/n): ";
    std::cin >> confirmation;

    // Proceed only if the user confirms
    if (confirmation == 'y' || confirmation == 'Y') {
        std::string query = "DELETE FROM booking WHERE bookingID = " + std::to_string(bookingID);
        if (db->executeQuery(query)) {
            std::cout << "Booking deleted successfully!" << std::endl;
        }
        else {
            std::cerr << "Error deleting booking!" << std::endl;
        }
    }
    else {
        std::cout << "Deletion cancelled." << std::endl;
    }
}

void BookingRequest::viewChangeRequests() {
    std::string query =
        "SELECT br.requestID, b.bookingID, u1.name AS therapist_name, u2.name AS patient_name, "
        "br.requested_date, br.requested_time, br.requested_duration, br.status "
        "FROM booking_requests br "
        "JOIN booking b ON br.bookingID = b.bookingID "
        "JOIN user u1 ON b.therapistID = u1.userID "
        "JOIN user u2 ON b.patientID = u2.userID";

    MYSQL_RES* res = db->fetchResults(query);
    MYSQL_ROW row;

    if (res) {
        std::cout << "Change Requests:\n";
        while ((row = mysql_fetch_row(res))) {
            std::cout << "Request ID: " << row[0] << "\n"
                << "Booking ID: " << row[1] << "\n"
                << "Therapist: " << row[2] << "\n"
                << "Patient: " << row[3] << "\n"
                << "Requested Date: " << row[4] << "\n"
                << "Requested Time: " << row[5] << "\n"
                << "Requested Duration: " << row[6] << " minutes\n"
                << "Status: " << row[7] << "\n\n";
        }
        mysql_free_result(res);
    }
    else {
        std::cerr << "No change requests found!" << std::endl;
    }
}

void BookingRequest::requestBookingChange() {
    viewBookingDetails();  // Ensure the user sees available bookings

    int bookingID;
    std::cout << "Enter Booking ID for Change Request: ";
    std::cin >> bookingID;

    std::cout << "Enter New Requested Date (YYYY-MM-DD): ";
    std::cin >> bookingDate;
    std::cout << "Enter New Requested Time (HH:MM): ";
    std::cin >> bookingTime;
    std::cout << "Enter New Requested Duration (in minutes): ";
    std::cin >> duration;

    std::string query = "INSERT INTO booking_requests (bookingID, requested_date, requested_time, requested_duration, status) "
        "VALUES (" + std::to_string(bookingID) + ", '" + bookingDate + "', '" + bookingTime + "', " + std::to_string(duration) + ", 'pending')";
    if (db->executeQuery(query)) {
        std::cout << "Change request submitted.\n";
    }
    else {
        std::cerr << "Error submitting change request.\n";
    }
}

void BookingRequest::updateSessionStatus() {
    while (true) {
        // Step 1: Fetch and display all available bookings
        std::cout << "Fetching available bookings...\n";

        std::string query = "SELECT bookingID, user1.name AS therapist_name, user2.name AS patient_name, "
            "booking.booking_date, booking.booking_time, booking.duration, booking.status, booking.session_status "
            "FROM booking "
            "JOIN user AS user1 ON booking.therapistID = user1.userID "
            "JOIN user AS user2 ON booking.patientID = user2.userID "
            "WHERE booking.status != 'completed'"; // Exclude completed bookings

        MYSQL_RES* res = db->fetchResults(query);
        MYSQL_ROW row;

        if (!res) {
            std::cerr << "Error fetching available bookings." << std::endl;
            return;
        }

        std::vector<int> availableBookingIDs;
        int index = 1;

        std::cout << "Available bookings:\n";

        if (!res) {
            std::cerr << "Error: MySQL result set is NULL." << std::endl;
            return;
        }

        while ((row = mysql_fetch_row(res))) {
            try {
                // Check for NULL values before accessing
                int bookingID = row[0] ? std::stoi(row[0]) : 0;
                availableBookingIDs.push_back(bookingID);

                std::cout << index++ << ". Booking ID: " << (row[0] ? row[0] : "NULL")
                    << ", Therapist: " << (row[1] ? row[1] : "NULL")
                    << ", Patient: " << (row[2] ? row[2] : "NULL")
                    << ", Date: " << (row[3] ? row[3] : "NULL")
                    << ", Time: " << (row[4] ? row[4] : "NULL")
                    << ", Duration: " << (row[5] ? row[5] : "NULL") << " minutes"
                    << ", Status: " << (row[6] ? row[6] : "NULL")
                    << ", Session Status: " << (row[7] ? row[7] : "NULL")
                    << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing row: " << e.what() << std::endl;
            }
        }


        mysql_free_result(res);

        if (availableBookingIDs.empty()) {
            std::cerr << "No available bookings to update." << std::endl;
            return;
        }

        // Step 2: Let the user select a booking ID to update
        int selectedBookingID;
        std::cout << "Enter the Booking ID you want to update or 0 to exit: ";
        std::cin >> selectedBookingID;
        if (selectedBookingID == 0) {
            return;
        }

        // Check if the selected booking ID exists in the available bookings
        if (std::find(availableBookingIDs.begin(), availableBookingIDs.end(), selectedBookingID) == availableBookingIDs.end()) {
            std::cerr << "Error: Selected Booking ID " << selectedBookingID << " is not available for update." << std::endl;
            return;
        }

        // Step 3: Display session status options for the user to choose from
        std::cout << "Please select the new session status from the following options:\n";
        std::cout << "1. Ongoing\n";
        std::cout << "2. Absent\n";
        std::cout << "3. Completed\n";
        std::cout << "4. Postponed\n";
        std::cout << "5. Canceled\n";
        std::cout << "6. Oncoming\n";
        std::cout << "7. Advanced\n";
        std::cout << "8. Cancel\n";

        int statusChoice = 0;
        std::cout << "Enter the number corresponding to the new session status: ";
        std::cin >> statusChoice;

        // Validate the status choice
        if (statusChoice < 1 || statusChoice > 7) {
            std::cerr << "Invalid selection! Please choose a number between 1 and 7, 8 to cancel.\n";
            return;
        }

        // Map the choice to a status
        std::string newStatus;
        switch (statusChoice) {
        case 1: newStatus = "ongoing"; break;
        case 2: newStatus = "absent"; break;
        case 3: newStatus = "completed"; break;
        case 4: newStatus = "postponed"; break;
        case 5: newStatus = "canceled"; break;
        case 6: newStatus = "oncoming"; break;
        case 7: newStatus = "advanced"; break;
        case 8: break;
        }

        // Step 4: Update the session status for the selected booking
        query = "UPDATE booking_request SET session_status = '" + newStatus + "' WHERE bookingID = " + std::to_string(selectedBookingID);

        if (db->executeQuery(query)) {
            std::cout << "Booking ID " << selectedBookingID << " session status successfully updated to '" << newStatus << "'.\n";
        }
        else {
            std::cerr << "Error updating session status for Booking ID " << selectedBookingID << ". Please try again.\n";
        }
    }
}
