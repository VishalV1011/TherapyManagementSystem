-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Jan 28, 2025 at 05:10 PM
-- Server version: 10.4.32-MariaDB
-- PHP Version: 8.2.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `therapy_mgmt`
--

-- --------------------------------------------------------

--
-- Table structure for table `available_slots`
--

CREATE TABLE `available_slots` (
  `slotID` int(11) NOT NULL,
  `available_date` date DEFAULT NULL,
  `available_time` time DEFAULT NULL,
  `available_duration` int(11) DEFAULT NULL,
  `max_booking` int(11) DEFAULT NULL,
  `type` enum('private','group') DEFAULT 'group'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `available_slots`
--

INSERT INTO `available_slots` (`slotID`, `available_date`, `available_time`, `available_duration`, `max_booking`, `type`) VALUES
(1, '2025-01-29', '10:00:00', 120, 0, 'group'),
(2, '2026-01-01', '07:00:00', 600, 5, 'group'),
(3, '2025-01-28', '12:00:00', 120, 0, 'private'),
(4, '2025-10-23', '10:00:40', 120, 0, 'private'),
(5, '2025-06-10', '09:00:00', 60, 8, 'group'),
(6, '2025-02-01', '16:00:00', 240, 0, 'private');

--
-- Triggers `available_slots`
--
DELIMITER $$
CREATE TRIGGER `set_type_based_on_max_booking` BEFORE INSERT ON `available_slots` FOR EACH ROW BEGIN
  IF NEW.`max_booking` = 1 THEN
    SET NEW.`type` = 'private';
  ELSE
    SET NEW.`type` = 'group';
  END IF;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `booking`
--

CREATE TABLE `booking` (
  `bookingID` int(11) NOT NULL,
  `patientID` int(11) NOT NULL,
  `session_status` enum('ongoing','absent','completed','postponed','canceled','oncoming','advanced') DEFAULT 'oncoming',
  `slotID` int(11) NOT NULL,
  `isConfirmed` tinyint(1) DEFAULT 0,
  `created_at` datetime DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `booking`
--

INSERT INTO `booking` (`bookingID`, `patientID`, `session_status`, `slotID`, `isConfirmed`, `created_at`) VALUES
(6, 1, 'completed', 2, 0, '2025-01-28 23:27:31'),
(7, 2, 'oncoming', 5, 0, '2025-01-28 23:40:09'),
(8, 2, 'oncoming', 2, 0, '2025-01-28 23:40:18');

--
-- Triggers `booking`
--
DELIMITER $$
CREATE TRIGGER `before_booking_delete` BEFORE DELETE ON `booking` FOR EACH ROW BEGIN
    -- Insert corresponding payment row when booking is deleted
    INSERT INTO payments (patientID, totalAmount, paymentStatus, bookingID, paymentDateTime)
    VALUES (OLD.patientID, 0.00, 'Pending', OLD.bookingID, NOW());
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `patient`
--

CREATE TABLE `patient` (
  `patientID` int(11) NOT NULL,
  `rating` tinyint(1) DEFAULT NULL,
  `userID` int(11) NOT NULL,
  `name` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `patient`
--

INSERT INTO `patient` (`patientID`, `rating`, `userID`, `name`) VALUES
(1, 1, 2, 'Vishal'),
(2, 4, 5, 'Lacen');

--
-- Triggers `patient`
--
DELIMITER $$
CREATE TRIGGER `after_patient_delete` AFTER DELETE ON `patient` FOR EACH ROW BEGIN
  UPDATE booking 
  SET notes = CONCAT(notes, ' patient has been removed from the system') 
  WHERE patientID = OLD.patientID;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `payments`
--

CREATE TABLE `payments` (
  `paymentID` int(11) NOT NULL,
  `patientID` int(11) NOT NULL,
  `totalAmount` decimal(10,2) NOT NULL DEFAULT 0.00,
  `paymentStatus` enum('Pending','Completed') NOT NULL DEFAULT 'Pending',
  `bookingID` int(11) NOT NULL,
  `paymentDateTime` datetime DEFAULT NULL COMMENT 'Timestamp when payment is completed'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `payments`
--

INSERT INTO `payments` (`paymentID`, `patientID`, `totalAmount`, `paymentStatus`, `bookingID`, `paymentDateTime`) VALUES
(17, 2, 187.50, 'Completed', 7, '2025-01-28 23:40:09'),
(18, 2, 1875.00, 'Completed', 8, '2025-01-28 23:40:18');

-- --------------------------------------------------------

--
-- Table structure for table `payments_archive`
--

CREATE TABLE `payments_archive` (
  `paymentID` int(11) NOT NULL,
  `patientID` int(11) NOT NULL,
  `paidAmount` decimal(10,2) NOT NULL DEFAULT 0.00,
  `paymentStatus` enum('Pending','Completed') NOT NULL DEFAULT 'Pending',
  `bookingID` int(11) DEFAULT NULL,
  `archived_at` datetime NOT NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- Table structure for table `therapist`
--

CREATE TABLE `therapist` (
  `therapistID` int(11) NOT NULL,
  `seniority` enum('senior','intermediate','rookie') DEFAULT NULL,
  `userID` int(11) NOT NULL,
  `name` varchar(100) NOT NULL,
  `specialization` enum('psychology','physiology') DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `therapist`
--

INSERT INTO `therapist` (`therapistID`, `seniority`, `userID`, `name`, `specialization`) VALUES
(1, 'intermediate', 3, 'Aishu', 'psychology'),
(2, 'senior', 4, 'Papa', 'physiology');

--
-- Triggers `therapist`
--
DELIMITER $$
CREATE TRIGGER `after_therapist_delete` AFTER DELETE ON `therapist` FOR EACH ROW BEGIN
  UPDATE booking 
  SET notes = CONCAT(notes, ' therapist has been removed from the system') 
  WHERE therapistID = OLD.therapistID;
END
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `therapist_availability_mapping`
--

CREATE TABLE `therapist_availability_mapping` (
  `therapistID` int(11) NOT NULL,
  `slotID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `therapist_availability_mapping`
--

INSERT INTO `therapist_availability_mapping` (`therapistID`, `slotID`) VALUES
(1, 1),
(1, 3),
(2, 1),
(1, 4),
(1, 2),
(2, 6),
(1, 5),
(2, 2);

-- --------------------------------------------------------

--
-- Table structure for table `user`
--

CREATE TABLE `user` (
  `userID` int(11) NOT NULL,
  `name` varchar(100) NOT NULL,
  `username` varchar(50) NOT NULL,
  `password` varchar(255) NOT NULL,
  `role` enum('therapist','patient','admin') NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `user`
--

INSERT INTO `user` (`userID`, `name`, `username`, `password`, `role`) VALUES
(1, 'Administrator', 'root', 'NextAge!0', 'admin'),
(2, 'Vishal', 'p1', 'NextAge!0', 'patient'),
(3, 'Aishu', 't1', 'NextAge!0', 'therapist'),
(4, 'Papa', 't2', 'NextAge!0', 'therapist'),
(5, 'Lacen', 'p2', 'NextAge!0', 'patient');

--
-- Triggers `user`
--
DELIMITER $$
CREATE TRIGGER `after_user_insert_patient` AFTER INSERT ON `user` FOR EACH ROW BEGIN
  IF NEW.role = 'patient' THEN
    -- Insert into the patient table without specifying patientID, allowing auto-increment to take over
    INSERT INTO `patient` (`userID`, `name`)
    VALUES (NEW.userID, NEW.name);
  END IF;
END
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `after_user_insert_therapist` AFTER INSERT ON `user` FOR EACH ROW BEGIN
  IF NEW.role = 'therapist' THEN
    -- Insert into the therapist table without specifying therapistID, allowing auto-increment to take over
    INSERT INTO `therapist` (`userID`, `name`)
    VALUES (NEW.userID, NEW.name);
  END IF;
END
$$
DELIMITER ;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `available_slots`
--
ALTER TABLE `available_slots`
  ADD PRIMARY KEY (`slotID`);

--
-- Indexes for table `booking`
--
ALTER TABLE `booking`
  ADD PRIMARY KEY (`bookingID`),
  ADD KEY `patientID` (`patientID`),
  ADD KEY `booking_fk_slot` (`slotID`);

--
-- Indexes for table `patient`
--
ALTER TABLE `patient`
  ADD PRIMARY KEY (`patientID`),
  ADD KEY `patient_fk_userID` (`userID`);

--
-- Indexes for table `payments`
--
ALTER TABLE `payments`
  ADD PRIMARY KEY (`paymentID`),
  ADD KEY `patientID` (`patientID`),
  ADD KEY `payments_fk_booking` (`bookingID`);

--
-- Indexes for table `payments_archive`
--
ALTER TABLE `payments_archive`
  ADD PRIMARY KEY (`paymentID`);

--
-- Indexes for table `therapist`
--
ALTER TABLE `therapist`
  ADD PRIMARY KEY (`therapistID`),
  ADD KEY `therapist_fk_userID` (`userID`);

--
-- Indexes for table `therapist_availability_mapping`
--
ALTER TABLE `therapist_availability_mapping`
  ADD KEY `therapistID` (`therapistID`),
  ADD KEY `slotID` (`slotID`);

--
-- Indexes for table `user`
--
ALTER TABLE `user`
  ADD PRIMARY KEY (`userID`),
  ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `available_slots`
--
ALTER TABLE `available_slots`
  MODIFY `slotID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- AUTO_INCREMENT for table `booking`
--
ALTER TABLE `booking`
  MODIFY `bookingID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- AUTO_INCREMENT for table `patient`
--
ALTER TABLE `patient`
  MODIFY `patientID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT for table `payments`
--
ALTER TABLE `payments`
  MODIFY `paymentID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=19;

--
-- AUTO_INCREMENT for table `therapist`
--
ALTER TABLE `therapist`
  MODIFY `therapistID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT for table `user`
--
ALTER TABLE `user`
  MODIFY `userID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `booking`
--
ALTER TABLE `booking`
  ADD CONSTRAINT `booking_fk_patient` FOREIGN KEY (`patientID`) REFERENCES `patient` (`patientID`) ON DELETE CASCADE,
  ADD CONSTRAINT `booking_fk_slot` FOREIGN KEY (`slotID`) REFERENCES `available_slots` (`slotID`) ON DELETE CASCADE;

--
-- Constraints for table `patient`
--
ALTER TABLE `patient`
  ADD CONSTRAINT `patient_fk_userID` FOREIGN KEY (`userID`) REFERENCES `user` (`userID`) ON DELETE CASCADE;

--
-- Constraints for table `payments`
--
ALTER TABLE `payments`
  ADD CONSTRAINT `payments_fk_booking` FOREIGN KEY (`bookingID`) REFERENCES `booking` (`bookingID`) ON DELETE CASCADE,
  ADD CONSTRAINT `payments_fk_patient` FOREIGN KEY (`patientID`) REFERENCES `patient` (`patientID`);

--
-- Constraints for table `therapist`
--
ALTER TABLE `therapist`
  ADD CONSTRAINT `therapist_fk_userID` FOREIGN KEY (`userID`) REFERENCES `user` (`userID`) ON DELETE CASCADE;

--
-- Constraints for table `therapist_availability_mapping`
--
ALTER TABLE `therapist_availability_mapping`
  ADD CONSTRAINT `therapist_availability_mapping_ibfk_1` FOREIGN KEY (`therapistID`) REFERENCES `therapist` (`therapistID`),
  ADD CONSTRAINT `therapist_availability_mapping_ibfk_2` FOREIGN KEY (`slotID`) REFERENCES `available_slots` (`slotID`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
