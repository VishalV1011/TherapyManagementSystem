#include "Interface.h"

// Instantiate class objects
Interface::Interface()
	: user(&database), booking(&database), feat(), req(database), stats(), payment() {  // Pass references to the constructor of UserManagement and BookingRequest
	// No dynamic allocation, objects are now stack-allocated

    database.checkConnection();

    opt = '0';
}

void Interface::logInInterface() {
    switch (selectedRole) {
    case 1:
        role = "admin";
        break;
    case 2:
        role = "therapist";
        break;
    case 3:
        role = "patient";
        break;
    default:
        std::cout << "\n[Error] Role not selected properly! Please restart." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "~~~ Access your Account ~~~" << std::endl;
        std::cout << "\nRole: " << role << "\n\n";

        if (role != "admin") {
            std::cout << "1. Login\n";
            std::cout << "2. Register (Go to the counter)\n";
            std::cout << "3. Cancel\n";
        }
        else {
            std::cout << "1. Login\n";
            std::cout << "2. Cancel\n";
        }

        std::cout << "\nSelect an option: ";
        std::cin >> opt;

        system("cls");

        switch (opt) {
        case '1': {
            if (user.login(role)) {
                this->userID = user.getUserID();
                payment.setUserIDRole(this->userID, this->role);
                booking.setUserIDRole(this->userID, this->role);
                stats.setUserIDRole(this->userID, this->role);
                if (role == "admin") {
                    loggedInInterfaceAdmin();
                }
                else if (role == "therapist") {
                    loggedInInterfaceTherapist();
                }
                else if (role == "patient") {
                    loggedInInterfacePatient();
                }
                else {
                    std::cerr << "[Error] Something went wrong!" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
            else {
                std::cout << "\n[Error] Invalid login credentials, please try again." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            break;
        }
        case '2': {
            if (role != "admin") {
                std::cout << "\nPlease go to the counter for account creation!" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            else {
                role = "";
                introInterface();
            }
            break;
        }
        case '3':
            role = "";
            introInterface();
            break;
        default:
            std::cout << "\n[Error] Invalid selection! Please enter a valid option.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            break;
        }

    } while (opt != '3');
}

// Intro menu
void Interface::introInterface() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    system("cls");
    std::cout << "~~~ Welcome to the Therapy Management System! ~~~\n" << std::endl;
    std::cout << "Please select your role:\n\n";
    std::cout << "1. Admin\n";
    std::cout << "2. Therapist\n";
    std::cout << "3. Patient\n";
    std::cout << "0. Exit the program\n\n";

    std::cout << "Select your role: ";
    std::cin >> selectedRole;

    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        if (selectedRole == 1 || selectedRole == 2 || selectedRole == 3) {
            logInInterface();
        }
        else if (selectedRole == 0) {
            std::cout << "\nThank you for using the Therapy Management System!" << std::endl;
            exit(0);
        }
        else {
            std::cout << "\n[Error] Invalid role selection! Please select a valid role.\n";
        }
    } while (selectedRole != 0);
}

// booking interface
void Interface::manageBookingRequests() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    system("cls");
    if (this->role != "admin") {
        std::cout << "\nError: Access denied. Admin role required.\n";
        return; // Exit the function if the user is not an admin
    }


    int choice = 0;
    booking.setUserIDRole(userID, role);

    std::cout << "\n==== Manage Booking Panel ====\n";
    std::cout << "Logged in as: " << this->role << "\n";
    std::cout << "---------------------------------\n";

    
    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\nAdmin Options:\n";
        std::cout << "1. Add Available Slot\n";
        std::cout << "2. View Available Slots\n";
        std::cout << "3. Update Slot\n";
        std::cout << "4. Delete Slot\n";
        std::cout << "5. View Bookings\n";
        std::cout << "6. Update Bookings\n";
        std::cout << "7. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            booking.addAvailableSlot();
            break;
        case 2:
            booking.viewAvailableSlots();
            break;
        case 3:
            booking.updateSlot();
            break;
        case 4:
            booking.deleteSlot();
            break;
        case 5:
            booking.viewBookings();
            break;
        case 6:
            booking.updateBookings();
            break;
        case 7:
            std::cout << "\nExiting Admin Panel.\n";
            break;
        default:
            std::cout << "\nInvalid choice. Please try again.\n";
        }
    } while (choice != 7);
}

void Interface::manageUsers(UserManagement& user) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    system("cls");  
    if (this->role != "admin") {
        std::cout << "\nError: Access denied. Admin role required.\n";
        return; // Exit the function if the user is not an admin
    }


    int adminSelectAction;
    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\n================= User Management Menu =================\n";
        std::cout << "1. Register New User\n"
            << "2. Update User Details\n"
            << "3. View User Details\n"
            << "4. Reset User Passwords\n"
            << "5. Delete User\n"
            << "6. Return to Main Menu\n";
        std::cout << "\n========================================================\n";
        std::cout << "Enter your choice (1-6): ";
        std::cin >> adminSelectAction;

        switch (adminSelectAction) {
        case 1:
            std::cout << "\nYou have selected 'Register New User'.\n";
            user.registerUserWithDetails();
            break;
        case 2:
            std::cout << "\nYou have selected 'Update User Details'.\n";
            user.updateUserDetails(role);
            break;
        case 3:
            std::cout << "\nYou have selected 'View User Details'.\n";
            user.viewUsers(role);
            break;
        case 4:
            std::cout << "\nYou have selected 'Reset User Passwords'.\n";
            user.updateUserPassword();
            break;
        case 5:
            std::cout << "\nYou have selected 'Delete User'.\n";
            user.deleteUser();
            break;
        case 6:
            std::cout << "\nReturning to Main Menu...\n";
            break;
        default:
            std::cout << "\nInvalid selection! Please enter a valid option (1-6).\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Pause before refreshing the menu
        system("cls");
    } while (adminSelectAction != 6);
}

void Interface::manageStatistics(StatisticManagement& stats) {
    if (this->role != "admin") {
        std::cout << "\nError: Access denied. Admin role required.\n";
        return; // Exit the function if the user is not an admin
    }

    int statsChoice;

    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\n============== Statistics Management Menu ==============\n";
        std::cout << "1. View Clinic Earnings\n"
            << "2. Count Bookings Per Month (Line Graph)\n"
            << "3. Generate Receipt for Payment\n"
            << "4. Return to Main Menu\n";
        std::cout << "\n========================================================\n";
        std::cout << "Enter your choice (1-4): ";
        std::cin >> statsChoice;

        switch (statsChoice) {
        case 1: {
            // Calculate and display clinic earnings
            std::cout << "\n=== Clinic Earnings ===\n";
            stats.calculateEarnings();
            break;
        }
        case 2: {
            // Count and display bookings per month
            std::cout << "\n=== Bookings Per Month ===\n";
            stats.countBookingsPerMonth();
            break;
        }
        case 3: {
            stats.printReceipt();
            break;
        }
        case 4:
            // Exit to main menu
            std::cout << "\nReturning to Main Menu...\n";
            break;
        default:
            std::cout << "\nInvalid selection! Please enter a valid option (1-4).\n";
        }

        // Pause before refreshing the menu
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } while (statsChoice != 4);
}

void Interface::managePayments(PaymentManager& payment) {
    if (this->role != "admin") {
        std::cout << "\nError: Access denied. Admin role required.\n";
        return; // Exit the function if the user is not an admin
    }

    int paymentChoice;
    payment.setUserIDRole(userID, role); // Check if the user is an admin

    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\n============================\n";
        std::cout << "   Payment Management Menu   \n";
        std::cout << "============================\n";

        // Display options based on user role
        std::cout << "\n[1] View Pending Payments\n"
            << "[2] View All Payments\n"
            << "[3] Update Payment\n"
            << "[4] Delete Payment\n"
            << "[5] Return to Admin Menu\n";

        std::cout << "\nPlease enter your choice (1-5): ";
        std::cin >> paymentChoice;

        switch (paymentChoice) {
        case 1:
            // Handle view pending payments
            if (!payment.viewPendingPayments()) {
                std::cout << "\nNo pending payments found.\n";
            }
            break;

        case 2:
            // Handle view all payments
            if (!payment.viewAllPayments(role)) {
                std::cout << "\nNo payments found.\n";
            }
            break;

        case 3:
            // Handle update payment
            if (!payment.updatePayment()) {
                std::cout << "\nFailed to update payment.\n";
            }
            break;

        case 4:
            // Handle delete payment
            if (!payment.deletePayment()) {
                std::cout << "\nFailed to delete payment.\n";
            }
            break;

        case 5:
            // Admin only: return to admin menu
            std::cout << "\nReturning to Admin Menu...\n";
            break;

        default:
            std::cout << "\nInvalid selection. Please try again.\n";
            break;
        }

        // Pause and prompt for the next action
        if (paymentChoice != 5) {
            std::cout << "\nPress Enter to return to the menu...";
            std::cin.ignore();
            std::cin.get();
        }

    } while (paymentChoice != 5); // Loop back to show the menu again after performing an action
}

// admin interface (post-login)
void Interface::loggedInInterfaceAdmin() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    system("cls");
    std::cout << "\n============================\n";
    std::cout << "   Welcome, Admin!           \n";
    std::cout << "============================\n";

    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\nSelect an action:\n";
        std::cout << "[1] Manage Users\n"
            << "[2] Manage Bookings\n"
            << "[3] Manage Payments\n"
            << "[4] View Statistics and Analytics\n"
            << "[5] Log Out\n";

        std::cout << "\nEnter your selection (1-5): ";
        std::cin >> selection;

        switch (selection) {
        case 1:
            system("cls");
            std::cout << "\nYou have selected Manage Users.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            manageUsers(user);
            system("cls");
            break;
        case 2:
            system("cls");
            std::cout << "\nYou have selected Manage Bookings.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            manageBookingRequests();  // Call the new function to manage bookings
            break;
        case 3:
            system("cls");
            std::cout << "\nYou have selected Manage Payments.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            managePayments(payment);
            system("cls");
            break;
        case 4:
            system("cls");
            std::cout << "\nYou have selected View General Statistics and Analytics.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            manageStatistics(stats);
            system("cls");
            break;
        case 5:
            system("cls");
            std::cout << "\nLogging out...\n";
            role = "";
            std::cout << "\nYou have successfully logged out.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            system("cls");
            introInterface();
            break;
        default:
            system("cls");
            std::cout << "\nInvalid selection. Please choose a valid option.\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
            system("cls");
        }
    } while (selection != 5); // Loop until the user logs out
}

// therapist interface (post-login)
void Interface::loggedInInterfaceTherapist() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    system("cls");
    std::cout << "\n\nSuccessfully logged in as a Therapist!" << std::endl;

    int selection;
    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\nTherapist Options:\n";
        std::cout << "1. View Available Slots\n";
        std::cout << "2. Select Slot\n";
        std::cout << "3. View Selected Slots\n";
        std::cout << "4. Cancel Selected Slots\n";
        std::cout << "5. View Patient Details\n";
        std::cout << "6. View My Details\n";
        std::cout << "7. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> selection;

        switch (selection) {
        case 1:
            booking.viewAvailableSlots();
            break;
        case 2:
            booking.therapistSelectSlot();
            break;
        case 3:
            booking.getTherapistSlotDetails();
            break;
        case 4:
            booking.withdrawBooking();
            break;
        case 5:
            booking.viewPatientDetails();
            break;
        case 6:
            user.viewUsers(role);
            break;
        case 7:
            std::cout << "\nExiting Therapist Panel.\n";
            break;
        default:
            std::cout << "\nInvalid choice. Please try again.\n";
        }
    } while (selection != 7);
}

void Interface::loggedInInterfacePatient() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    system("cls");
    std::cout << "\n\nSuccessfully logged in as a Patient!" << std::endl;

    int selection;
    do {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");
        std::cout << "\nPatient Options:\n";
        std::cout << "1. View Available Slots\n";
        std::cout << "2. View Therapist Details\n";
        std::cout << "3. Book a Slot\n";
        std::cout << "4. View Booking\n";
        std::cout << "5. Withdraw Booking\n";
        std::cout << "6. Continue Payment\n";
        std::cout << "7. View Payments\n";
        std::cout << "8. View Pending Payments\n";
        std::cout << "9. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> selection;

        switch (selection) {
        case 1:
            booking.viewAvailableSlots();
            break;
        case 2:
            booking.viewTherapistDetails();
            break;
        case 3:
            booking.bookSlot();
            break;
        case 4:
            booking.viewBookings();
            break;
        case 5:
            booking.withdrawBooking();
            break;
        case 6:
            payment.continuePaymentProcess();
            break;
        case 7:
            payment.viewAllPayments(role);
            break;
        case 8:
            payment.viewPendingPayments();
            break;
        case 9:
            std::cout << "\nExiting Patient Panel.\n";
            break;
        default:
            std::cout << "\nInvalid choice. Please try again.\n";
        }
    } while (selection != 9);
}
