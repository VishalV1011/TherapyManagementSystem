#include "BookingRequest.h"

BookingRequest::BookingRequest(DatabaseConnection* database)
    : db(database), feat(std::make_unique<Features>()) {

}

void BookingRequest::setUserIDRole(int userID, const std::string& role) {
    this->userID = userID;
    this->role = role;
}

void BookingRequest::viewBookings() {
    std::string query;

    if (role == "admin") {
        // Admin can view all bookings, including therapist names
        query = R"(
            SELECT 
                b.bookingID, b.patientID, 
                GROUP_CONCAT(t.name ORDER BY t.name ASC) AS therapistNames, 
                b.slotID, s.available_date, s.available_time, s.available_duration, 
                b.session_status, b.isConfirmed, b.created_at 
            FROM 
                booking b 
            LEFT JOIN 
                therapist_availability_mapping tam ON b.slotID = tam.slotID 
            LEFT JOIN 
                therapist t ON tam.therapistID = t.therapistID 
            LEFT JOIN 
                available_slots s ON tam.slotID = s.slotID 
            GROUP BY 
                b.bookingID
        )";
    }
    else if (role == "patient") {
        // Patient can only view their own bookings
        int patientID = feat->getPatientIDByUserID(userID);
        query = R"(
            SELECT 
                b.bookingID, b.patientID, 
                GROUP_CONCAT(t.name ORDER BY t.name ASC) AS therapistNames, 
                b.slotID, s.available_date, s.available_time, s.available_duration, 
                b.session_status, b.isConfirmed, b.created_at 
            FROM 
                booking b 
            LEFT JOIN 
                therapist_availability_mapping tam ON b.slotID = tam.slotID 
            LEFT JOIN 
                therapist t ON tam.therapistID = t.therapistID 
            LEFT JOIN 
                available_slots s ON tam.slotID = s.slotID 
            WHERE 
                b.patientID = )" + std::to_string(patientID) + R"( 
            GROUP BY 
                b.bookingID
        )";
    }
    else {
        std::cerr << "Error: Unauthorized role.\n";
        return;
    }

    // Fetch the results from the database
    auto results = db->fetchQuery(query);

    if (results.empty()) {
        std::cout << "No bookings found.\n";
        return;
    }

    // Display the bookings in a more structured and organized format
    std::cout << "Booking Details:\n";
    std::cout << "------------------------------------------------------------------------------------------------------------------\n";
    std::cout << std::setw(10) << "BookingID" << " | "
        << std::setw(10) << "PatientID" << " | "
        << std::setw(25) << "Therapist Names" << " | "
        << std::setw(10) << "SlotID" << " | "
        << std::setw(12) << "Date" << " | "
        << std::setw(8) << "Time" << " | "
        << std::setw(10) << "Duration" << " | "
        << std::setw(12) << "Status" << " | "
        << std::setw(10) << "Confirmed" << " | "
        << "Created At\n";
    std::cout << "------------------------------------------------------------------------------------------------------------------\n";

    for (const auto& row : results) {
        std::cout << std::setw(10) << row[0] << " | "
            << std::setw(10) << row[1] << " | "
            << std::setw(25) << row[2] << " | "
            << std::setw(10) << row[3] << " | "
            << std::setw(12) << row[4] << " | "
            << std::setw(8) << row[5] << " | "
            << std::setw(10) << row[6] << " | "
            << std::setw(12) << row[7] << " | "
            << std::setw(10) << (row[8] == "1" ? "Yes" : "No") << " | "
            << row[9] << "\n";
    }

    std::cout << "------------------------------------------------------------------------------------------------------------------\n";

    // Wait for the user to press Enter to continue
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();  // Ignore any leftover input
    std::cin.get();  // Wait for the user to press Enter
}

void BookingRequest::getTherapistSlotDetails() {
    // Assuming therapistID is available (e.g., via userID)
    int therapistID = feat->getTherapistIDByUserID(userID);

    // Query to get therapist and slot details for the logged-in therapist only
    std::string query = R"(
        SELECT 
            aslot.slotID, aslot.available_date, aslot.available_time, aslot.available_duration, 
            aslot.max_booking, aslot.type, GROUP_CONCAT(t.name ORDER BY t.name ASC) AS therapists 
        FROM 
            therapist_availability_mapping tam 
        JOIN 
            available_slots aslot ON tam.slotID = aslot.slotID 
        JOIN 
            therapist t ON tam.therapistID = t.therapistID 
        WHERE 
            tam.therapistID = )" + std::to_string(therapistID) + R"( 
        GROUP BY 
            aslot.slotID, aslot.available_date, aslot.available_time, aslot.available_duration, 
            aslot.max_booking, aslot.type
    )";

    // Fetching the results
    auto results = db->fetchQuery(query);

    if (results.empty()) {
        std::cout << "No therapist slot details found.\n";
        return;
    }

    // Table headers
    std::cout << "\nTherapist Slot Details:\n";
    std::cout << std::left
        << std::setw(10) << "Slot ID"
        << std::setw(12) << "Date"
        << std::setw(10) << "Time"
        << std::setw(12) << "Dur (mins)"
        << std::setw(6) << "Max"
        << std::setw(15) << "Type"
        << std::setw(30) << "Therapists"
        << "\n";
    std::cout << std::string(85, '-') << "\n";

    // Table rows
    for (const auto& row : results) {
        std::cout << std::left
            << std::setw(10) << row[0]
            << std::setw(12) << row[1]
            << std::setw(10) << row[2]
            << std::setw(12) << row[3]
            << std::setw(6) << row[4]
            << std::setw(15) << row[5]
            << std::setw(30) << row[6]
            << "\n";
    }
}

void BookingRequest::addAvailableSlot() {
    if (role != "admin") {
        std::cout << "Access denied. Only admins can add available slots.\n";
        return;
    }

    std::string date, time, type;
    int duration, maxBooking;

    // Get the current date using localtime_s
    time_t now = std::time(nullptr);
    struct tm ltm;
    localtime_s(&ltm, &now);
    char currentDate[11];
    strftime(currentDate, sizeof(currentDate), "%Y-%m-%d", &ltm);

    // Input date and validate it
    while (true) {
        std::cout << "Enter date (YYYY-MM-DD) (must be after " << currentDate << "): ";
        std::cin >> date;

        // Basic date validation (assuming simple format validation is enough)
        if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
            std::cout << "Invalid date format. Please enter a valid date (YYYY-MM-DD).\n";
            continue;
        }

        if (date > currentDate) break;  // Validate that the date is in the future
        std::cout << "Invalid date. Please enter a date after " << currentDate << ".\n";
    }

    // Input time and validate
    std::cout << "Enter time (HH:MM:SS): ";
    std::cin >> time;
    if (time.length() != 8 || time[2] != ':' || time[5] != ':' || !isdigit(time[0]) || !isdigit(time[1]) || !isdigit(time[3]) || !isdigit(time[4]) || !isdigit(time[6]) || !isdigit(time[7])) {
        std::cout << "Invalid time format. Please enter a valid time (HH:MM:SS).\n";
        return;
    }

    // Input duration and validate
    std::cout << "Enter duration (minutes): ";
    if (!(std::cin >> duration) || duration <= 0) {
        std::cout << "Invalid duration. Please enter a positive number for duration.\n";
        return;
    }

    // Input max booking and validate
    std::cout << "Enter max booking: ";
    if (!(std::cin >> maxBooking) || maxBooking <= 0) {
        std::cout << "Invalid max booking. Please enter a positive number for max booking.\n";
        return;
    }

    // Determine type based on max booking
    type = (maxBooking == 1) ? "private" : "group";

    // Prepare SQL query for insertion
    std::string insertSlotQuery = "INSERT INTO available_slots (available_date, available_time, available_duration, max_booking, type) "
        "VALUES ('" + date + "', '" + time + "', " + std::to_string(duration) + ", " +
        std::to_string(maxBooking) + ", '" + type + "')";

    // Try inserting into the database
    try {
        bool success = db->executeQuery(insertSlotQuery);
        if (success) {
            std::cout << "Slot added successfully.\n";
        }
        else {
            std::cout << "Error: Could not add the available slot.\n";
        }
    }
    catch (const std::exception& e) {
        std::cout << "Database error: " << e.what() << "\n";
    }
}

void BookingRequest::updateSlot() {
    if (role != "admin") {
        std::cout << "Access denied. Only admins can update slots.\n";
        return;
    }

    // Display available slots using a DatabaseConnection method
    viewAvailableSlots();

    int slotID;
    int selection;
    std::string newValue;
    int newIntValue;

    // Get the Slot ID to update
    std::cout << "Enter Slot ID to update: ";
    std::cin >> slotID;

    // Present the options for which attribute to update
    std::cout << "\nSelect attribute to update:\n";
    std::cout << "1. Date (YYYY-MM-DD)\n";
    std::cout << "2. Time (HH:MM:SS)\n";
    std::cout << "3. Duration (minutes)\n";
    std::cout << "4. Max Booking\n";
    std::cout << "Enter your selection: ";
    std::cin >> selection;

    std::string query;

    // Process the selection and update the corresponding attribute
    switch (selection) {
    case 1:
        std::cout << "Enter new date (YYYY-MM-DD): ";
        std::cin >> newValue;
        query = "UPDATE available_slots SET available_date = '" + newValue + "' WHERE slotID = " + std::to_string(slotID);
        break;
    case 2:
        std::cout << "Enter new time (HH:MM:SS): ";
        std::cin >> newValue;
        query = "UPDATE available_slots SET available_time = '" + newValue + "' WHERE slotID = " + std::to_string(slotID);
        break;
    case 3:
        std::cout << "Enter new duration (minutes): ";
        std::cin >> newIntValue;
        query = "UPDATE available_slots SET available_duration = " + std::to_string(newIntValue) + " WHERE slotID = " + std::to_string(slotID);
        break;
    case 4:
        std::cout << "Enter new max booking: ";
        std::cin >> newIntValue;
        query = "UPDATE available_slots SET max_booking = " + std::to_string(newIntValue) +
            ", type = '" + ((newIntValue == 1) ? "private" : "group") + "' WHERE slotID = " + std::to_string(slotID);
        break;
    default:
        std::cout << "Invalid selection. Operation canceled.\n";
        return;
    }

    // Execute the query using the DatabaseConnection instance (db)
    if (db->executeQuery(query)) {
        std::cout << "Slot updated successfully!\n";
    }
    else {
        std::cout << "Error updating slot. Please check your input and try again.\n";
    }
}

void BookingRequest::deleteSlot() {
    if (role != "admin") {
        std::cout << "Access denied. Only admins can delete slots.\n";
        return;
    }

    viewAvailableSlots();

    int slotID;
    char confirmation;

    // Prompt admin for Slot ID to delete
    std::cout << "Enter Slot ID to delete: ";
    std::cin >> slotID;

    // Check if there are any bookings associated with this slot
    std::string checkBookingsQuery = "SELECT COUNT(*) FROM booking WHERE slotID = " + std::to_string(slotID);
    auto result = db->fetchQuery(checkBookingsQuery);

    if (!result.empty() && !result[0].empty() && std::stoi(result[0][0]) > 0) {
        std::cout << "This slot is already booked. You cannot delete it.\n";
        return;
    }

    // Confirmation step
    std::cout << "Are you sure you want to delete Slot ID " << slotID << "? (y/n): ";
    std::cin >> confirmation;

    if (confirmation == 'y' || confirmation == 'Y') {
        // Delete from therapist mapping first to avoid foreign key constraint violation
        std::string deleteMappingQuery = "DELETE FROM therapist_availability_mapping WHERE slotID = " + std::to_string(slotID);
        db->executeQuery(deleteMappingQuery);

        // Delete slot from the `available_slots` table
        std::string deleteSlotQuery = "DELETE FROM available_slots WHERE slotID = " + std::to_string(slotID);
        if (db->executeQuery(deleteSlotQuery)) {
            std::cout << "Slot deleted successfully.\n";
        }
        else {
            std::cout << "Error deleting slot. Please check the Slot ID and try again.\n";
        }
    }
    else {
        std::cout << "Delete operation canceled.\n";
    }
}

// Assuming the results from db->fetchQuery(query) is a vector of vectors of strings
void BookingRequest::viewAvailableSlots() {
    std::string query =
        "SELECT available_slots.slotID, available_date, available_time, available_duration, type, "
        "GROUP_CONCAT(DISTINCT therapist.name ORDER BY therapist.name SEPARATOR ', ') AS therapists, "
        "available_slots.max_booking, "
        "COUNT(booking.bookingID) AS current_bookings, "
        "SUM(booking.isConfirmed) AS confirmed_bookings "
        "FROM available_slots "
        "LEFT JOIN therapist_availability_mapping "
        "    ON available_slots.slotID = therapist_availability_mapping.slotID "
        "LEFT JOIN therapist "
        "    ON therapist_availability_mapping.therapistID = therapist.therapistID "
        "LEFT JOIN booking "
        "    ON available_slots.slotID = booking.slotID "
        "WHERE available_slots.max_booking > 0 "
        "GROUP BY available_slots.slotID, available_date, available_time, available_duration, "
        "         type, available_slots.max_booking";

    auto results = db->fetchQuery(query);

    if (results.empty()) {
        std::cout << "No available slots found.\n";
        return;
    }

    // Print header row
    std::cout << "\n" << std::setw(10) << "Slot ID"
        << std::setw(15) << "Date"
        << std::setw(10) << "Time"
        << std::setw(15) << "Duration"
        << std::setw(15) << "Type"
        << std::setw(25) << "Therapists"
        << std::setw(15) << "Max Booking"
        << std::setw(20) << "Current Bookings"
        << std::setw(20) << "Confirmed Bookings" << "\n";

    std::cout << "--------------------------------------------------------\n";

    // Loop through the results and print each slot
    for (const auto& row : results) {
        std::cout << std::setw(10) << row[0]          // Slot ID
            << std::setw(15) << row[1]          // Date
            << std::setw(10) << row[2]          // Time
            << std::setw(15) << row[3]          // Duration
            << std::setw(15) << row[4]          // Type
            << std::setw(25) << row[5]          // Therapists
            << std::setw(15) << row[6]          // Max Booking
            << std::setw(20) << row[7]          // Current Bookings
            << std::setw(20) << row[8]          // Confirmed Bookings
            << "\n";
    }

    // Wait for the user to press Enter before continuing
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void BookingRequest::therapistSelectSlot() {
    if (role != "therapist") {
        std::cout << "Access denied. Only therapists can select slots.\n";
        return;
    }

    viewAvailableSlots(); // Display available slots for therapists to choose from

    int slotID;
    std::cout << "Enter Slot ID to select: ";
    std::cin >> slotID;

    // Get the therapist's ID using their userID
    int therapistID = feat->getTherapistIDByUserID(userID);
    if (therapistID == -1) {
        std::cerr << "Error: Invalid therapist ID.\n";
        return;
    }

    // Verify if the slot exists and is valid
    std::string validateSlotQuery =
        "SELECT max_booking FROM available_slots WHERE slotID = " + std::to_string(slotID);
    auto slotResults = db->fetchQuery(validateSlotQuery);

    if (slotResults.empty()) {
        std::cerr << "Error: Slot does not exist.\n";
        return;
    }

    int maxBooking = std::stoi(slotResults[0][0]);

    if (maxBooking <= 0) {
        std::cerr << "Error: Slot is not available for selection.\n";
        return;
    }

    // Check if the therapist has already selected this slot
    std::string checkQuery =
        "SELECT COUNT(*) FROM therapist_availability_mapping WHERE therapistID = " +
        std::to_string(therapistID) + " AND slotID = " + std::to_string(slotID);
    auto results = db->fetchQuery(checkQuery);

    if (!results.empty() && !results[0].empty() && std::stoi(results[0][0]) > 0) {
        std::cerr << "Error: You have already selected this slot.\n";
        return;
    }

    // Insert the therapist-slot mapping
    std::string insertQuery =
        "INSERT INTO therapist_availability_mapping (therapistID, slotID) VALUES (" +
        std::to_string(therapistID) + ", " + std::to_string(slotID) + ")";

    if (db->executeQuery(insertQuery)) {
        std::cout << "Slot successfully selected.\n";

        // Decrement max_booking for the slot
        std::string updateSlotQuery =
            "UPDATE available_slots SET max_booking = max_booking - 1 WHERE slotID = " + std::to_string(slotID);
        db->executeQuery(updateSlotQuery);
    }
    else {
        std::cerr << "Error: Database operation failed.\n";
    }
}

void BookingRequest::viewPatientDetails() {
    if (role != "therapist") {
        std::cout << "Access denied. Only therapists can view patient details.\n";
        return;
    }

    if (!db->checkConnection()) {
        std::cerr << "Database connection failed. Cannot retrieve patient details.\n";
        return;
    }

    // Fetch only relevant patient details
    std::string query = R"(
        SELECT patientID, userID, name, rating 
        FROM patient
        WHERE rating IS NOT NULL
    )";
    auto results = db->fetchQuery(query);

    if (results.empty()) {
        std::cout << "No patient records found.\n";
        return;
    }

    std::cout << "Patient Details:\n";
    for (const auto& row : results) {
        std::cout << "PatientID: " << (row[0] == "NULL" ? "N/A" : row[0])
            << ", UserID: " << (row[1] == "NULL" ? "N/A" : row[1])
            << ", Name: " << (row[2] == "NULL" ? "N/A" : row[2])
            << ", Rating: " << (row[3] == "NULL" ? "N/A" : row[3]) << "\n";
    }
}

void BookingRequest::viewTherapistDetails() {
    if (role != "patient") {
        std::cout << "Access denied. Only patients can view therapist details.\n";
        return;
    }

    if (!db->checkConnection()) {
        std::cerr << "Database connection failed. Cannot retrieve therapist details.\n";
        return;
    }

    // Fetch therapists with specialization
    std::string query = R"(
        SELECT therapistID, name, seniority, specialization 
        FROM therapist
        WHERE specialization IS NOT NULL
    )";
    auto results = db->fetchQuery(query);

    if (results.empty()) {
        std::cout << "No therapist records found.\n";
        return;
    }

    std::cout << "Therapist Details:\n";
    for (const auto& row : results) {
        std::cout << "TherapistID: " << (row[0] == "NULL" ? "N/A" : row[0])
            << ", Name: " << (row[1] == "NULL" ? "N/A" : row[1])
            << ", Seniority: " << (row[2] == "NULL" ? "N/A" : row[2])
            << ", Specialization: " << (row[3] == "NULL" ? "N/A" : row[3]) << "\n";
    }

}

void BookingRequest::bookSlot() {
    try {
        // Ensure the user has the correct role to book a slot
        if (role != "patient") {
            std::cout << "Access denied. Only patients can book slots.\n";
            return;
        }

        viewCatalogue();

        // Display available slots to the user
        viewAvailableSlots();

        // Prompt the user for a Slot ID
        int slotID;
        std::cout << "Enter Slot ID: ";
        std::cin >> slotID;

        if (std::cin.fail() || slotID <= 0) {
            std::cin.clear();
            std::cin.ignore();
            throw std::runtime_error("Invalid Slot ID. Please enter a valid number.");
        }

        // Verify the database connection
        if (!db->checkConnection()) {
            throw std::runtime_error("Database connection failed.");
        }

        // Retrieve the patient ID for the current user
        int patientID = feat->getPatientIDByUserID(this->userID);
        if (patientID <= 0) {
            throw std::runtime_error("Failed to retrieve patient ID.");
        }

        // Check if the patient has already booked this slot
        std::string bookingCheckQuery = "SELECT COUNT(*) FROM booking WHERE patientID = " + std::to_string(patientID) +
            " AND slotID = " + std::to_string(slotID);
        auto bookingCheckResult = db->fetchQuery(bookingCheckQuery);
        if (!bookingCheckResult.empty() && std::stoi(bookingCheckResult[0][0]) > 0) {
            throw std::runtime_error("You have already booked this slot.");
        }

        // Check if the selected slot exists and is not fully booked
        std::string slotCheckQuery = "SELECT max_booking FROM available_slots WHERE slotID = " + std::to_string(slotID);
        auto slotCheckResult = db->fetchQuery(slotCheckQuery);

        if (slotCheckResult.empty()) {
            throw std::runtime_error("Invalid Slot ID. The slot does not exist.");
        }

        int maxBooking = std::stoi(slotCheckResult[0][0]);
        if (maxBooking <= 0) {
            throw std::runtime_error("This slot is fully booked.");
        }

        // Check if a therapist is assigned to the selected slot
        std::string therapistCheckQuery =
            "SELECT tm.therapistID FROM therapist_availability_mapping tm "
            "JOIN available_slots s ON tm.slotID = s.slotID "
            "WHERE s.slotID = " + std::to_string(slotID);
        auto therapistCheckResult = db->fetchQuery(therapistCheckQuery);

        if (therapistCheckResult.empty()) {
            throw std::runtime_error("No therapist is available for this slot.");
        }

        // Insert a new booking record
        std::string insertQuery =
            "INSERT INTO booking (patientID, slotID, session_status, isConfirmed) VALUES ("
            + std::to_string(patientID) + ", " + std::to_string(slotID) + ", 'oncoming', 0)";
        if (!db->executeQuery(insertQuery)) {
            throw std::runtime_error("Failed to book the slot.");
        }

        // Decrement the max_booking counter for the selected slot
        std::string updateSlotQuery = "UPDATE available_slots SET max_booking = max_booking - 1 WHERE slotID = " + std::to_string(slotID);
        if (!db->executeQuery(updateSlotQuery)) {
            throw std::runtime_error("Failed to update slot availability.");
        }

        // Retrieve the booking ID of the newly created booking
        std::string bookingIDQuery = "SELECT LAST_INSERT_ID()";
        auto bookingIDResult = db->fetchQuery(bookingIDQuery);
        if (bookingIDResult.empty() || bookingIDResult[0].empty()) {
            throw std::runtime_error("Failed to retrieve booking ID.");
        }

        int bookingID = std::stoi(bookingIDResult[0][0]);
        std::cout << "Booking successful! Your Booking ID is: " << bookingID << "\n";

        // Trigger payment calculation for the booking
        payment.calculatePayment(bookingID);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void BookingRequest::withdrawBooking() {
    viewBookings();
    if (role == "patient") {
        int bookingID;
        std::cout << "Enter Booking ID to withdraw: ";
        std::cin >> bookingID;

        // Confirm withdrawal
        char confirm;
        std::cout << "Are you sure you want to withdraw booking ID " << bookingID << "? (y/n): ";
        std::cin >> confirm;
        if (confirm != 'y' && confirm != 'Y') {
            std::cout << "Withdrawal canceled.\n";
            return;
        }

        // Check if booking exists and belongs to the patient
        std::string query = "SELECT slotID FROM booking WHERE bookingID = " +
            std::to_string(bookingID) +
            " AND patientID = " + std::to_string(feat->getPatientIDByUserID(userID));
        auto result = db->fetchQuery(query);

        if (result.empty()) {
            std::cerr << "Error: No such booking found for this patient.\n";
            return;
        }

        int slotID = std::stoi(result[0][0]);

        // Delete the booking entry from the booking table
        query = "DELETE FROM booking WHERE bookingID = " + std::to_string(bookingID);
        if (!db->executeQuery(query)) {
            std::cerr << "Error: Unable to delete the booking.\n";
            return;
        }

        // Decrease the `isBooked` counter in `available_slots`
        query = "UPDATE available_slots SET max_booking = max_booking + 1 WHERE slotID = " + std::to_string(slotID);
        if (db->executeQuery(query)) {
            std::cout << "Booking successfully withdrawn.\n";
        }
        else {
            std::cerr << "Error: Unable to update slot availability.\n";
        }

    }
    else if (role == "therapist") {
        int slotID;
        std::cout << "Enter Slot ID to withdraw from: ";
        std::cin >> slotID;

        // Confirm withdrawal
        char confirm;
        std::cout << "Are you sure you want to withdraw slot ID " << slotID << "? (y/n): ";
        std::cin >> confirm;
        if (confirm != 'y' && confirm != 'Y') {
            std::cout << "Withdrawal canceled.\n";
            return;
        }

        // Check if the therapist is assigned to this slot
        std::string query = "SELECT therapistID FROM therapist_availability_mapping WHERE slotID = " +
            std::to_string(slotID) +
            " AND therapistID = " + std::to_string(feat->getTherapistIDByUserID(userID));
        auto result = db->fetchQuery(query);

        if (result.empty()) {
            std::cerr << "Error: This slot is not assigned to the therapist.\n";
            return;
        }

        // Delete therapist's mapping for this slot
        query = "DELETE FROM therapist_availability_mapping WHERE slotID = " + std::to_string(slotID) +
            " AND therapistID = " + std::to_string(feat->getTherapistIDByUserID(userID));
        if (db->executeQuery(query)) {
            std::cout << "Slot successfully withdrawn for the therapist.\n";
        }
        else {
            std::cerr << "Error: Unable to withdraw slot.\n";
        }
    }
    else {
        std::cerr << "Error: Only patients or therapists can withdraw from bookings or slots.\n";
    }
}

void BookingRequest::updateBookings() {
    int bookingID;
    std::string sessionStatus;

    // Display current bookings (if necessary)
    std::cout << "Your current bookings:\n";
    viewBookings();

    // Get the booking ID from the user
    std::cout << "Enter Booking ID to update: ";
    std::cin >> bookingID;

    // Show selection menu for session_status
    std::cout << "Select the new session_status from the following options:\n";
    std::cout << "1. Ongoing\n";
    std::cout << "2. Absent\n";
    std::cout << "3. Completed\n";
    std::cout << "4. Postponed\n";
    std::cout << "5. Canceled\n";
    std::cout << "6. Oncoming\n";
    std::cout << "7. Advanced\n";

    int selection;
    std::cout << "Enter your selection (1-7): ";
    std::cin >> selection;

    // Validate the selection and map it to session status
    switch (selection) {
    case 1: sessionStatus = "ongoing"; break;
    case 2: sessionStatus = "absent"; break;
    case 3: sessionStatus = "completed"; break;
    case 4: sessionStatus = "postponed"; break;
    case 5: sessionStatus = "canceled"; break;
    case 6: sessionStatus = "oncoming"; break;
    case 7: sessionStatus = "advanced"; break;
    default:
        std::cerr << "Invalid selection!" << std::endl;
        return;
    }

    // Construct the query to update the session_status in the booking table
    std::string query = "UPDATE booking SET session_status = '" + sessionStatus +
        "' WHERE bookingID = " + std::to_string(bookingID);

    // Execute the query
    if (db->executeQuery(query)) {
        std::cout << "Booking ID " << bookingID << " updated successfully to '" << sessionStatus << "'!" << std::endl;
    }
    else {
        std::cerr << "Failed to update booking ID " << bookingID << "!" << std::endl;
    }
}

void BookingRequest::viewCatalogue() {
    std::cout << "\nPrices\n\nSenior ~ RM400.00/hr\nIntermediate ~ RM250.00/hr\nRookie ~ RM125.00/hr" << std::endl;
}