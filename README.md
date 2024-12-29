# Elevator-Simulation-System
An Object-Oriented Approach in C++

To run:

Prerequisites:
        Allegro libraries (allegro, allegro_font, allegro_ttf, allegro_primitives, allegro_image, allegro_main).

Build Instructions: 

Place all source files and headers in the same directory including: 
main.cpp
ECElevatorSim.cpp and ECElevatorSim.h 
ECGraphicViewImp.cpp and ECGraphicViewImp.h
ElevatorObserver.cpp and ElevatorObserver.h 


Run the following command in the terminal to compile the code and create the executable:

g++ -std=c++11 main.cpp ECElevatorSim1.cpp ECGraphicViewImp.cpp ElevatorObserver.cpp -o elevator_sim -I. -L/opt/homebrew/lib -lallegro -lallegro_font -lallegro_ttf -lallegro_primitives -lallegro_image -lallegro_main


Run Instructions: 
To run the program, use the following command:

./elevator_sim test-file-1.txt output.txt


P.S: Replace /opt/homebrew/lib with the correct library path for your system if necessary. 
