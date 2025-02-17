#include "DatabaseConnection.h"

// open connection to database
DatabaseConnection::DatabaseConnection() {
    conn = mysql_init(0);
    conn = mysql_real_connect(conn, "localhost", "root", "", "therapy_mgmt", 3306, NULL, 0);  
}

// close connection to database
DatabaseConnection::~DatabaseConnection() {
    if (conn)
        mysql_close(conn);
}

bool DatabaseConnection::checkConnection() {
    if (conn) {
        std::cout << "Initializing...." << std::endl;
        return true;
    }
    else {
        std::cerr << "Connection Failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
}

// to execute query to database
bool DatabaseConnection::executeQuery(const std::string& query) {
    if (mysql_query(conn, query.c_str()) == 0) {
        std::cout << "Query Executed Successfully!" << std::endl;
        return true;
    }
    else {
        std::cerr << "Query Failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
}

// to fetch result from database
MYSQL_RES* DatabaseConnection::fetchResults(const std::string& query) {
    if (mysql_query(conn, query.c_str()) == 0) {
        return mysql_store_result(conn);
    }
    else {
        std::cerr << "Query Failed: " << mysql_error(conn) << std::endl;
        return nullptr;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
}

// to fetch query and return data as a 2D vector
std::vector<std::vector<std::string>> DatabaseConnection::fetchQuery(const std::string& query) {
    MYSQL_RES* result = fetchResults(query);
    std::vector<std::vector<std::string>> rows;

    if (result) {
        MYSQL_ROW row;
        unsigned int num_fields = mysql_num_fields(result);

        while ((row = mysql_fetch_row(result))) {
            std::vector<std::string> currentRow;
            for (unsigned int i = 0; i < num_fields; ++i) {
                currentRow.push_back(row[i] ? row[i] : "NULL"); // Handle NULL values
            }
            rows.push_back(currentRow);
        }
        mysql_free_result(result);
    }
    else {
        std::cerr << "Error: Unable to fetch results!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 2 seconds pause
    }

    return rows;
}

MYSQL* DatabaseConnection::getConnection() {
    return conn;
}
