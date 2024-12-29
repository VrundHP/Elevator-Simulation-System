#ifndef ELEVATOR_OBSERVER_H
#define ELEVATOR_OBSERVER_H

#include "ECObserver.h"
#include "ECGraphicViewImp.h"
#include "ECElevatorSim1.h"
#include <vector>
#include <memory>
#include <queue>
#include <string>

// Enum to represent elevator direction
enum Direction {
    UP = 1,
    DOWN = -1,
    STOPPED = 0
};

// Class to manage the behavior of the elevator
class ElevatorHandler : public ECObserver {
public:
    ElevatorHandler(ECGraphicViewImp &viewInstance, std::shared_ptr<ECElevatorSim> simInstance, const std::string &dataFile);  // Constructor to initialize the elevator handler
    virtual void Update();  // Main function to process elevator events


    void MoveElevatorToFloor(int targetFloor);  // Move elevator to a target floor
    void DrawScene();  // Draw the scene
    void DrawProgressBar(int currentTime, int maxTime);  // Function to draw a progress bar indicating the current time
    bool IsSimulationComplete() const;  // Getter to check if the simulation is complete
    bool HasPrintedCompletionMessage() const;  // Getter to check if the completion message has been printed
    void InitializePassengersFromBackend();

    void IncrementallyMoveElevator(int targetFloor);  // Function to incrementally move the elevator to a target floor
    void IncrementTime();  // Increment the simulation time
    int GetCurrentTime() const;  // Get the current time
    int GetMaxTime() const;  // Get the maximum simulation time
    int GetTargetFloor() const;  // Get the current target floor
    void TogglePause();  // Toggle the simulation pause state
    void HandlePassengers();  // Handle passengers when the elevator reaches a floor
    void UpdateDirectionAndTarget();



private:


    // Track disappearing passengers with their remaining display time
    std::unordered_map<int, std::chrono::steady_clock::time_point> disappearingPassengers;

    const int disappearDelayMs = 1000; // 1000 milliseconds (1 second)

    void UpdateDisappearingPassengers();


    void DrawElevator();  // Function to draw the elevator and passengers inside it
    void DrawWaitingPassengers();  // Function to draw waiting passengers at each floor
    void ProcessPassengers();  // Handles passengers boarding or leaving the elevator
    bool ShouldStopAtCurrentFloor() const;  // Determines whether the elevator should stop at a floor
    void StopAndHandle();  // Stops the elevator to manage passengers boarding or leaving
    std::string GenerateWaitingPassengerString(int floor, int count);
    void ValidateState();
    void HandleSimulationCompletion();  




    void LoadSimulationData(const std::string &filename);  // Function to load simulation data from a file
    void ParseAndUpdateFromStep(const std::string& stepData);  // Parse and update elevator state from a given step of simulation data

    ECGraphicViewImp& graphicView;  // Graphic view reference
    std::shared_ptr<ECElevatorSim> elevatorSim;  // Pointer to backend elevator simulation

    Direction direction;  // Direction of the elevator
    bool isPaused;  // Pause flag for simulation
    bool isSimulationComplete;  // Flag indicating if the simulation is complete
    bool hasPrintedCompletionMessage;  // Tracks if the completion message has been printed
    
    std::vector<std::vector<int>> waitingDestinations; // List of destinations for each waiting passenger at each floor

    int currentFloor;  // Tracks the current floor of the elevator
    int targetFloor;  // The target floor the elevator is currently moving to
    int elevatorYPos;  // Current y-coordinate of the elevator for smooth movement
    int numFloors;  // Total number of floors in the building
    int totalTicks;  // Total number of time ticks from input file
    int currentTick;  // Current time tick for progress bar 
    int moveEndTime;  // The target time when the elevator must reach its destination
    int ticksPerFloor;  // Number of ticks required to move between floors
    int currentStepIndex;  // Keeps track of the current simulation step index
    int CalculateYPosForFloor(int floor) const;  // Calculates Y position for a given floor
    int CalculateMoveDuration(int startFloor, int targetFloor) const;  // Calculates the duration in ticks to move between floors

    double moveSpeed;  // Pixels per tick to ensure timely arrival at the target

    bool isElevatorMoving;  // Tracks whether the elevator is currently moving
    bool stopNext;  // Indicates if elevator should stop at the next floor

    std::queue<int> floorQueue;  // Queue to track floor requests
    std::vector<int> waitingPassengers;  // Tracks passengers waiting at each floor
    std::vector<int> elevatorPassengers;  // Tracks passengers inside the elevator and their target floors
    std::vector<int> waitingDirections;  // Direction of waiting passengers (-1 for down, 1 for up)
    std::vector<std::string> simulationSteps;  // Stores all the simulation steps loaded from the output file
};

#endif /* ELEVATOR_HANDLER_H */
