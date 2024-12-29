#include "ElevatorObserver.h"
#include <cstdlib>  // For random number generation
#include <thread>   // For std::this_thread::sleep_for
#include <chrono>   // For std::chrono::milliseconds
#include <fstream>  // For file input
#include <sstream>  // For string streams
#include <iostream>

// Constructor for ElevatorHandler
ElevatorHandler::ElevatorHandler(ECGraphicViewImp &viewInstance, std::shared_ptr<ECElevatorSim> simInstance, const std::string &dataFile)
    : graphicView(viewInstance), elevatorSim(simInstance), isPaused(false), isSimulationComplete(false), currentTick(0),
      currentFloor(1), targetFloor(1), numFloors(elevatorSim->GetTotalFloors()), 
      totalTicks(0), ticksPerFloor(10), moveEndTime(0), moveSpeed(0),isElevatorMoving(false), hasPrintedCompletionMessage(false) {

    int floorHeight = graphicView.GetHeight() / numFloors;
    elevatorYPos = graphicView.GetHeight() - (currentFloor * floorHeight);

    waitingPassengers.resize(numFloors, 0);
    waitingDirections.resize(numFloors, 0);
    elevatorPassengers.clear();

    std::cout << "ElevatorHandler initialized. NumFloors: " << numFloors << ", ElevatorYPos: " << elevatorYPos << std::endl;

    LoadSimulationData(dataFile);

    if (numFloors == 0) {
        isPaused = true;
        std::cerr << "Simulation paused due to initialization error." << std::endl;
    } else {
        std::cout << "ElevatorHandler initialized with " << numFloors << " floors." << std::endl;
    }
}

// Function to load the simulation data from a file
void ElevatorHandler::LoadSimulationData(const std::string &filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(inFile, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::cout << "Processing line: [" << line << "]" << std::endl;

        if (firstLine) {
            std::istringstream iss(line);
            if (!(iss >> numFloors >> totalTicks)) {
                std::cerr << "Error reading number of floors and ticks from input file. Line content: " << line << std::endl;
                return;
            }

            std::cout << "Parsed numFloors: " << numFloors << ", totalTicks: " << totalTicks << std::endl;

            if (numFloors <= 0 || totalTicks <= 0) {
                std::cerr << "Error: Number of floors or total ticks must be greater than zero." << std::endl;
                return;
            }

            waitingPassengers.resize(numFloors, 0);
            waitingDirections.resize(numFloors, 0);
            firstLine = false;
        } else {
            simulationSteps.push_back(line);
        }
    }

    inFile.close();
    currentStepIndex = 0;
    std::cout << "Loaded " << simulationSteps.size() << " simulation steps from file: " << filename << std::endl;
}

void ElevatorHandler::ParseAndUpdateFromStep(const std::string &stepData) {
    if (stepData.find("Waiting Passengers:") != std::string::npos) {
        std::istringstream iss(stepData.substr(stepData.find(":") + 1));
        bool validData = true;

        std::vector<int> updatedWaiting(numFloors, 0);
        for (int i = 0; i < numFloors; ++i) {
            int passengerCount;
            if (!(iss >> passengerCount)) {
                std::cerr << "[ParseAndUpdateFromStep] Error parsing waiting passengers for floor " << (i + 1) << std::endl;
                validData = false;
                break;
            }
            updatedWaiting[i] = passengerCount;
        }

        if (validData) {
            waitingPassengers = updatedWaiting; // Update state only if valid
            std::cout << "[ParseAndUpdateFromStep] Updated waiting passengers: ";
            for (int count : waitingPassengers) std::cout << count << " ";
            std::cout << std::endl;
        } else {
            std::cerr << "[ParseAndUpdateFromStep] Invalid data, retaining previous state." << std::endl;
        }
    } else {
        std::cerr << "[ParseAndUpdateFromStep] Missing 'Waiting Passengers:', retaining previous state." << std::endl;
    }
}




void ElevatorHandler::ProcessPassengers() {
    // Synchronize boarding and alighting logic with backend
    auto currentPassengers = elevatorSim->GetElevatorPassengers();
    elevatorPassengers = currentPassengers;

    // Update waiting passengers after boarding
    auto updatedWaiting = elevatorSim->GetWaitingPassengers();
    waitingPassengers = updatedWaiting;

    std::cout << "Processed passengers at floor: " << currentFloor << std::endl;
}

int ElevatorHandler::CalculateMoveDuration(int startFloor, int targetFloor) const {
    if (startFloor < 1 || startFloor > numFloors || targetFloor < 1 || targetFloor > numFloors) {
        std::cerr << "Error: Invalid floor(s) for move duration calculation. "
                  << "Start: " << startFloor << ", Target: " << targetFloor << std::endl;
        return -1; // Return an error code
    }

    int numFloorsToMove = abs(targetFloor - startFloor);
    int duration = numFloorsToMove * ticksPerFloor;

    std::cout << "Calculated move duration from floor " << startFloor 
              << " to floor " << targetFloor << ": " << duration << " ticks." << std::endl;
    return duration;
}


bool isElevatorMoving = false;

// Main event update function for the elevator
void ElevatorHandler::Update() {
    // Synchronize state from backend
    waitingPassengers = elevatorSim->GetWaitingPassengers();
    elevatorPassengers = elevatorSim->GetElevatorPassengers();

    std::cout << "[Update] Current waiting passengers: ";
    for (int count : waitingPassengers) std::cout << count << " ";
    std::cout << std::endl;

    std::cout << "[Update] Current elevator passengers: ";
    for (int dest : elevatorPassengers) std::cout << dest << " ";
    std::cout << std::endl;

    static auto lastTime = std::chrono::steady_clock::now();
    static auto endSimulationTime = std::chrono::steady_clock::time_point();

    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = currentTime - lastTime;
    lastTime = currentTime;

    if (isSimulationComplete) {
        if (endSimulationTime == std::chrono::steady_clock::time_point()) {
            endSimulationTime = std::chrono::steady_clock::now();
        }

        auto delayElapsed = std::chrono::steady_clock::now() - endSimulationTime;
        if (delayElapsed >= std::chrono::seconds(3)) {
            if (!hasPrintedCompletionMessage) {
                std::cout << "Simulation over." << std::endl;
                hasPrintedCompletionMessage = true;
            }
            DrawScene();
            return;
        } else {
            DrawScene();
            return;
        }
    }

    // Handle pause/resume functionality
    ECGVEventType event = graphicView.GetCurrEvent();
    if (event == ECGV_EV_KEY_DOWN_SPACE) {
        isPaused = !isPaused;
        std::cout << "Pause state toggled. Current state: " << (isPaused ? "Paused" : "Running") << std::endl;
    }

    if (isPaused) {
        DrawScene();
        return;
    }

    currentTick++;

    // Parse and update waiting passengers from step data
    if (currentStepIndex < simulationSteps.size()) {
        std::cout << "Parsing step index: " << currentStepIndex << std::endl;
        ParseAndUpdateFromStep(simulationSteps[currentStepIndex]);
        currentStepIndex++;
    }

    // Handle elevator movement
    if (isElevatorMoving) {
        IncrementallyMoveElevator(targetFloor);
        return; // Skip the rest of this cycle
    }

    // Handle passenger boarding and alighting
    if (!isElevatorMoving && currentFloor == targetFloor) {
        StopAndHandle();

        if (!elevatorPassengers.empty() || 
            std::any_of(waitingPassengers.begin(), waitingPassengers.end(), [](int count) { return count > 0; })) {
            UpdateDirectionAndTarget();
            isElevatorMoving = true;
        } else {
            // Check if simulation is complete
            bool allRequestsHandled = currentStepIndex >= simulationSteps.size();
            bool elevatorEmpty = elevatorPassengers.empty();
            bool noWaitingPassengers = std::all_of(waitingPassengers.begin(), waitingPassengers.end(), [](int count) { return count == 0; });

            if (allRequestsHandled && elevatorEmpty && noWaitingPassengers) {
                std::cout << "All requests have been handled. Marking simulation as complete." << std::endl;
                isSimulationComplete = true;
                endSimulationTime = std::chrono::steady_clock::now();
            }
        }

        DrawScene();
        return;
    }

    DrawScene(); // Redraw the scene
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // Adjust the duration to slow down the simulation

}


// Function to initialize waiting passengers from backend
void ElevatorHandler::InitializePassengersFromBackend() {
    // Load initial passenger states from the backend once during initialization
    elevatorPassengers = elevatorSim->GetElevatorPassengers();
    waitingPassengers = elevatorSim->GetWaitingPassengers();
    std::cout << "Initialized passengers from backend." << std::endl;

    std::cout << "[Update] Waiting passengers: ";
for (int count : waitingPassengers) std::cout << count << " ";
std::cout << "\n[Update] Elevator passengers: ";
for (int dest : elevatorPassengers) std::cout << dest << " ";
std::cout << std::endl;

}

bool ElevatorHandler::IsSimulationComplete() const {
    return isSimulationComplete;
}

bool ElevatorHandler::HasPrintedCompletionMessage() const {
    return hasPrintedCompletionMessage;
}

// Function to move the elevator incrementally to a target floor
void ElevatorHandler::IncrementallyMoveElevator(int targetFloor) {
    int targetYPos = CalculateYPosForFloor(targetFloor);

    const int moveSpeed = 20; // Pixels per update cycle

    if (elevatorYPos < targetYPos) {
        elevatorYPos += moveSpeed;
        if (elevatorYPos >= targetYPos) {
            elevatorYPos = targetYPos;
            currentFloor = targetFloor;
            isElevatorMoving = false;
            StopAndHandle();
            std::cout << "Elevator reached target floor: " << currentFloor << std::endl;
        }
    } else if (elevatorYPos > targetYPos) {
        elevatorYPos -= moveSpeed;
        if (elevatorYPos <= targetYPos) {
            elevatorYPos = targetYPos;
            currentFloor = targetFloor;
            isElevatorMoving = false;
            StopAndHandle();
            std::cout << "Elevator reached target floor: " << currentFloor << std::endl;
        }
    }

    DrawScene();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int ElevatorHandler::CalculateYPosForFloor(int floor) const {
    // Assuming floors are indexed starting at 1 (i.e., floor 1 is at the bottom)
    int floorHeight = 200;  // Height in pixels per floor (this can be adjusted as needed)
    int totalBuildingHeight = floorHeight * numFloors;

    // Calculate Y position
    // Assuming Y=0 is at the top, and each floor is evenly spaced by floorHeight.
    int yPos = totalBuildingHeight - (floor * floorHeight);

    return yPos;
}

// Function to move the elevator smoothly to a target floor
void ElevatorHandler::MoveElevatorToFloor(int targetFloor) {
    std::cout << "Moving elevator to floor: " << targetFloor << std::endl;

    // Calculate the level height for each floor
    int levelHeight = graphicView.GetHeight() / numFloors;
    int targetYPos = graphicView.GetHeight() - (targetFloor * levelHeight);

    // Set up the speed for elevator movement (pixels per millisecond)
    const double speed = 0.5;  // Adjust this value to make the movement slower or faster

    // Track the time using a steady clock
    auto startTime = std::chrono::steady_clock::now();

    // Determine the direction of movement
    int direction = (targetYPos > elevatorYPos) ? 1 : -1;

    // Loop to move the elevator smoothly
    while ((direction > 0 && elevatorYPos < targetYPos) || (direction < 0 && elevatorYPos > targetYPos)) {
        // Calculate elapsed time
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = currentTime - startTime;

        // Move elevator based on elapsed time and speed
        elevatorYPos += direction * speed * elapsed.count();
        startTime = currentTime;

        // Prevent overshooting the target position
        if ((direction > 0 && elevatorYPos > targetYPos) || (direction < 0 && elevatorYPos < targetYPos)) {
            elevatorYPos = targetYPos;
        }

        // Redraw scene
        DrawScene();

        // Sleep for a short time to control the update frequency
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 frames per second
    }

    // Ensure elevator is at the exact position of the target floor
    elevatorYPos = targetYPos;
    currentFloor = targetFloor;  // Update current floor
    std::cout << "Elevator reached floor: " << currentFloor << std::endl;

    // Final draw to make sure the elevator is rendered properly in its final position
    DrawScene();
}

// Function to handle passengers at a floor (boarding and alighting)
void ElevatorHandler::StopAndHandle() {
    std::cout << "Handling passengers at floor: " << currentFloor << std::endl;

    // Passengers alighting
    for (auto it = elevatorPassengers.begin(); it != elevatorPassengers.end();) {
        if (*it == currentFloor) {
            std::cout << "Passenger alighted at floor: " << currentFloor << std::endl;
            it = elevatorPassengers.erase(it); // Remove passenger from elevator
        } else {
            ++it;
        }
    }

    // Passengers boarding
    if (waitingPassengers[currentFloor - 1] > 0) {
    std::cout << "Passengers boarding at floor: " << currentFloor << std::endl;

    for (int i = 0; i < waitingPassengers[currentFloor - 1]; ++i) {
        int destination;

        // Ensure a valid destination is selected
        do {
            destination = (rand() % numFloors) + 1;
        } while (destination == currentFloor);

        elevatorPassengers.push_back(destination); // Add passenger to elevator
        std::cout << "New passenger boarded with destination: " << destination << std::endl;
    }

    // Clear the waiting passengers only after they are fully added to the elevator
    waitingPassengers[currentFloor - 1] = 0;
}

    std::cout << "Waiting passengers state after handling: ";
    for (int count : waitingPassengers) {
        std::cout << count << " ";
    }
    std::cout << std::endl;
}



// Draw the entire graphical scene
void ElevatorHandler::DrawScene() {
    if (numFloors == 0) {
        std::cerr << "DrawScene: numFloors is zero, cannot draw." << std::endl;
        return;
    }

    std::cout << "Drawing entire scene. Current tick: " << currentTick << std::endl;

    graphicView.DrawFilledRectangle(0, 0, graphicView.GetWidth(), graphicView.GetHeight(), ECGV_YELLOW);

    DrawElevator();
    DrawWaitingPassengers();

    if (simulationSteps.size() > 0) {
        DrawProgressBar(currentStepIndex, simulationSteps.size());
    }

    graphicView.DrawText(500, 3, ("Time: " + std::to_string(currentStepIndex)).c_str(), ECGV_BLACK);
    graphicView.SetRedraw(true);
}

void ElevatorHandler::UpdateDirectionAndTarget() {
    bool foundTarget = false;

    // Prioritize passengers in the elevator
    if (!elevatorPassengers.empty()) {
        targetFloor = elevatorPassengers.front();
        direction = (targetFloor > currentFloor) ? Direction::UP : Direction::DOWN;
        foundTarget = true;
    }

    // Check waiting passengers
    if (!foundTarget) {
        for (int i = 0; i < numFloors; ++i) {
            if (waitingPassengers[i] > 0) {
                targetFloor = i + 1;
                direction = (targetFloor > currentFloor) ? Direction::UP : Direction::DOWN;
                foundTarget = true;
                break;
            }
        }
    }

    if (!foundTarget) {
        isElevatorMoving = false;
        targetFloor = currentFloor; // Stay at current floor
    }

    std::cout << "Updated target floor: " << targetFloor << ", Direction: "
              << ((direction == Direction::UP) ? "UP" : "DOWN") << std::endl;
}


// Create the elevator with its passengers
void ElevatorHandler::DrawElevator() {
    std::cout << "Drawing elevator at floor: " << currentFloor << ", ElevatorYPos: " << elevatorYPos << std::endl;

    int elevatorHeight = 200;
    int elevatorWidth = graphicView.GetWidth() - 130;

    graphicView.DrawFilledRectangle(30, elevatorYPos, 30 + elevatorWidth, elevatorYPos + elevatorHeight, ECGV_PURPLE);

    int headSize = 25;
    int offset = 60;

    std::cout << "Number of passengers in elevator: " << elevatorPassengers.size() << std::endl;

    for (size_t i = 0; i < elevatorPassengers.size(); ++i) {
        int headX = 30 + 20 + i * offset;

        if (headX + headSize > 30 + elevatorWidth) {
            std::cout << "Not enough space to draw passenger " << i << " at headX: " << headX << std::endl;
            break;
        }

        graphicView.DrawCircle(headX, elevatorYPos + headSize, headSize, ECGV_RED);
        graphicView.DrawLine(headX, elevatorYPos + headSize * 2, headX, elevatorYPos + elevatorHeight - 10, ECGV_RED);
        graphicView.DrawText(headX - 5, elevatorYPos + headSize - 10, std::to_string(elevatorPassengers[i]).c_str(), ECGV_BLACK);

        std::cout << "Drawing passenger " << i << " in elevator with destination floor: " << elevatorPassengers[i]
                  << ", Position: (" << headX << ", " << (elevatorYPos + headSize) << ")" << std::endl;
    }

    graphicView.SetRedraw(true);
}

// Draw waiting passengers on each floor
void ElevatorHandler::DrawWaitingPassengers() {
    if (numFloors == 0) {
        std::cerr << "[DrawWaitingPassengers] Error: numFloors is zero!" << std::endl;
        return;
    }

    std::cout << "[DrawWaitingPassengers] Drawing passengers on all floors." << std::endl;

    int levelHeight = graphicView.GetHeight() / numFloors;

    for (int i = 0; i < numFloors; ++i) {
        int levelY = graphicView.GetHeight() - (i + 1) * levelHeight;

        // Draw the floor line
        graphicView.DrawLine(0, levelY, graphicView.GetWidth(), levelY, ECGV_BLACK);

        // Check if there are passengers waiting on this floor
        if (waitingPassengers[i] > 0) {
            int passengerX = graphicView.GetWidth() - (40 * waitingPassengers[i]);
            int passengerY = levelY + levelHeight / 2 - 20;

            // Divide passengers into "up" and "down" groups
            int upCount = waitingPassengers[i] / 2; // Half go up
            int downCount = waitingPassengers[i] - upCount; // Remaining go down

            int passengerIndex = 0;
            for (int j = 0; j < waitingPassengers[i]; ++j) {
                int offset = passengerIndex * 40;

                // Draw passenger circle and body
                graphicView.DrawCircle(passengerX + offset, passengerY, 15, ECGV_BLACK);
                graphicView.DrawLine(passengerX + offset, passengerY + 15, 
                                     passengerX + offset, passengerY + 45, ECGV_BLACK);

                // Draw direction indicator
                if (j < upCount) {
                    // Draw upward arrow for passengers going up
                    graphicView.DrawTriangle(passengerX + offset, passengerY - 20, 
                                             passengerX + offset - 10, passengerY - 10, 
                                             passengerX + offset + 10, passengerY - 10, ECGV_BLACK);
                } else {
                    // Draw downward arrow for passengers going down
                    graphicView.DrawTriangle(passengerX + offset, passengerY + 50, 
                                             passengerX + offset - 10, passengerY + 40, 
                                             passengerX + offset + 10, passengerY + 40, ECGV_BLACK);
                }

                std::cout << "[DrawWaitingPassengers] Passenger " << (j + 1) 
                          << " at Floor " << (i + 1) 
                          << " drawn at (" << (passengerX + offset) << ", " << passengerY << ")."
                          << (j < upCount ? " (Up)" : " (Down)") << std::endl;

                ++passengerIndex;
            }
        }
    }
}

// Draw progress bar for simulation time
void ElevatorHandler::DrawProgressBar(int currentTime, int maxTime) {
    if (maxTime == 0) {
        std::cerr << "Error: maxTime is zero. Cannot draw progress bar." << std::endl;
        return;
    }

    int barWidth = graphicView.GetWidth() - 20;

    // Adjust filledWidth calculation to make the progress bar movement smoother
    double progressRatio = static_cast<double>(currentTime) / maxTime;
    int filledWidth = static_cast<int>(barWidth * progressRatio);

    // Draw the filled portion of the progress bar
    graphicView.DrawFilledRectangle(10, 40, 10 + filledWidth, 60, ECGV_BLUE);

    // Draw the border for the progress bar
    graphicView.DrawRectangle(10, 40, 10 + barWidth, 60, 2, ECGV_BLACK);
}

// Getter for the current time tick
int ElevatorHandler::GetCurrentTime() const {
    return currentTick;
}

// Getter for the max time ticks
int ElevatorHandler::GetMaxTime() const {
    return totalTicks;
}

// Increment simulation time
void ElevatorHandler::IncrementTime() {
    currentTick++;
}

// Getter for the target floor
int ElevatorHandler::GetTargetFloor() const {
    return targetFloor;
}

// Toggle the pause state
void ElevatorHandler::TogglePause() {
    isPaused = !isPaused;
}

bool ElevatorHandler::ShouldStopAtCurrentFloor() const {
    return elevatorSim->IsSimulationComplete();
}