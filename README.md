# marchsnhu37.github.io

## What is this project?
This project takes a prototype thermostat previously built in Python and not only translates it to C, but also rearranges its original monolithic structure to implement various data-saving processes, including implementing a logging history and a data processing pipeline for data integrity and reducing leaks.

## What does the project do?
The thermostat is built to respond to user input through one of three button presses: depending on the combination of environmental variables, corresponding LEDs will light up and stay solid, fade, or remain off. The result is determined through various factors such as the current temperature versus the set temperature and the state of the thermostat.

## How does it differ from the original? Why did you make these changes?
The original thermostat had issues that were unique to Python, including lacking resource optimization. Rebuilding it in C allowed me to remediate some of its problems, which included delayed or missing inputs and memory issues. Beyond this, I built and replaced capabilities, like changing the original's abstract serial string management for a data processing pipeline to reduce memory corruption and improve optimization by making it more resistant to errors. I also gave the project persistence, meaning history is preserved even after the Pi or the program is cut from power, by implementing a local data logging engine. This, through SQLite in C, ensures a trackable history of events. These security-oriented changes make the thermostat more adaptable, including for future projects.
