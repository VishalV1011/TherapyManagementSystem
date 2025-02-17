#ifndef INTERFACE_H
#define INTERFACE_H

#include <mysql.h>
#include <iostream>
#include <string>
#include "DatabaseConnection.h"
#include "UserManagement.h"
#include "BookingRequest.h"
#include "Features.h"
#include "RequestManagement.h"
#include "StatisticManagement.h"
#include "PaymentManager.h"

class Interface {
private:
	int selection, selectedRole, adminChoice, adminSelectAction, userID;
	char opt;
	bool loginSuccesful, contProg = true, contLogIn = true;
	std::string username, role, password, newPassword;
    DatabaseConnection database;  // Stack allocated DatabaseConnection object
    UserManagement user;  // Stack allocated UserManagement object
    BookingRequest booking;  // Stack allocated BookingRequest object
	Features feat;
	RequestManagement req;
	StatisticManagement stats;
	PaymentManager payment;
public:
	Interface();
	void introInterface();
	void logInInterface();
	void manageBookingRequests();
	void manageRequests(RequestManagement& requestManager, std::string role);
	void manageUsers(UserManagement& user);
	void manageStatistics(StatisticManagement& stats);
	void managePayments(PaymentManager& payment);
	void loggedInInterfaceAdmin();
	void loggedInInterfaceTherapist();
	void loggedInInterfacePatient();
};
#endif
