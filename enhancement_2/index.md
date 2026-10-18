---
default: home
title: Enhancement 2
---

# Enhancement 2: Algorithms and Data Structure

[View pipeline.c](https://github.com/marchsnhu37/ePortfolio/blob/main/ThermostatProject/pipeline.c)

[View pipeline.h](https://github.com/marchsnhu37/ePortfolio/blob/main/ThermostatProject/pipeline.h)

  The CS 350 prototypical thermostat comes about from Module 7 of that course. In this particular assignment and in the ones preceding it, we were tasked with setting up Raspberry Pi hardware and programming LEDs on the board to correspond with temperatures that could be changed via pressing buttons attached to the board. I plan to take the original file that made the thermostat functionality of the Raspberry Pi and replace its abstract serial string management with a data processing pipeline. This will be done first with the creation of a FIFO array and then a data-stream parsing algorithm. Such implementations will prevent memory corruption and will in fact optimize it by making it more resistant to errors. 

  I selected this artifact both due to its recency in my degree program, and because I struggled with that course, and by continuing to work on it, I inevitably will improve as a student and a working professional. By building a data processing pipeline in C within this program, I demonstrate the ability to use algorithmic and data principles to improve computing solutions. Instead of using automated abstractions, I'm applying learned practices in order to minimize runtime errors while also maximizing the efficient use of resources. This memory balancing is done in the service of developing programs that can be run on a diverse array of physical devices. By building this I've demonstrated a professional-quality ability to adapt programs for increased data integrity and performance on embedded systems with fewer resources. 

  During the planning phase, I figured that this enhancement would align with Course Outcomes 2, 3, 4, and 5. Even though neither enhancement has been completely finalized, seeing as the artifact currently possesses hardware mockups, I believe that in its final form it will meet my goals. This enhancement is meant to make the thermostat a more durable program by reducing the scope of errors it can suffer and increasing its efficiency by way of improving its resource management. Such resource allocation will mean it can run on more devices, serving more audiences. 

  As I was working on this enhancement, I recognized the downsides of programming an entire project in just one file. Even in just the first enhancement, I struggled to maintain the code and catch errors, seeing as this was all in one file. It was very frustrating to have to split the codebase into multiple files given the first enhancement was made as a monolith. 

[Back to Homepage](../)

