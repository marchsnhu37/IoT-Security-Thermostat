---
layout: default
title: Enhancement 3
---

# Enhancement 3: Databases

[View database.c](https://github.com/marchsnhu37/ePortfolio/blob/main/ThermostatProject/database.c)

[View database.h](https://github.com/marchsnhu37/ePortfolio/blob/main/ThermostatProject/database.h)

  The CS 350 prototypical thermostat comes about from Module 7 of that course. In this particular assignment and in the ones preceding it, we were tasked with setting up Raspberry Pi hardware and programming LEDs on the board to correspond with temperatures that could be changed via pressing buttons attached to the board. In its previous iteration, the code had no persistence and kept no logs, meaning all history would be lost as soon as the Pi is cut from power or the program stops. I took the original file that made the thermostat functionality of the Raspberry Pi and implemented a local data logging engine using SQLite in C to ensure it has a trackable history. 

  I selected this artifact both due to its recency in my degree program, and because I struggled with that course, and by continuing to work on it, I inevitably will improve as a student and a working professional. By integrating an SQLite database layer into my C application, I show a clear ability to work with databases to manage and maintain an app's data in a structured way, as well as doing so safely with data integrity in mind. Transitioning from temporary logs by way of print statements to something with continuing permanence allows me to display the professional capability to build apps with a concentration on data integrity and robustness. This enhancement demonstrates that I can make apps that can span years by creating them with a focus on longevity. Through using SQLite, I also demonstrate the skills needed to manage I/O sans automation. Through this, I work to prevent resource leaking and data corruption.

  While planning in Module One, I predicted this enhancement would align with Course Outcomes 1, 3, 4, and 5, and I believe that even in its current state, with hardware mockups, it does that and will continue to due that once it is finalized. The implementation of a history allows developers and people using the thermostat to deduce what's working and what's not and how things in the given environment and with the given objectives can be improved upon. The enhancement works to also maintain resource efficiency through cleanups and by upkeeping data integrity. 

  While working on this enhancement, I noticed the usefulness of preventing monolithic programming when developing a project. I could easily take chunks of code from previous parts of the project, where applicable, and tailor them to make them relevant to the logging engine and to ensure they were secure. 

[Back to Homepage](../)
