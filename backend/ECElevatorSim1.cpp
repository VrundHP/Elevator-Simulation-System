#include "ECElevatorSim1.h"
#include <map>
#include <cmath>
#include <fstream>


ECElevatorSim::ECElevatorSim(int totalFloors, std::vector<ECElevatorSimRequest> &requestsList)
    : floorCount(totalFloors), currentFloor(1), currentDirection(EC_ELEVATOR_STOPPED), pendingRequests(requestsList),
      currentTime(0), numberOfRiders(0) {
    waitingPassengers.resize(floorCount, 0);
    waitingDirections.resize(floorCount, 0);
}

ECElevatorSim::~ECElevatorSim() {}

void ECElevatorSim::Simulate(int simulationDuration, const std::string& outputFilename) {
    std::ofstream outFile(outputFilename);  // Open the output file to write simulation results
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << outputFilename << " for writing." << std::endl;
        return;
    }

    // Write the number of floors and total simulation time as the first line in the file
    outFile << floorCount << " " << simulationDuration << "\n";  

    for (currentTime = 0; currentTime < simulationDuration; ++currentTime) {
        std::cout << "---- Time Step: " << currentTime << " ----" << std::endl;

        // Display current elevator state to console
        std::cout << "Current Floor: " << currentFloor 
                  << ", Direction: " << GetDirectionString(currentDirection) 
                  << ", Active Requests: ";
        for (const auto &floor : activeFloorRequests) std::cout << floor << " ";
        std::cout << std::endl;

        // Write the current state to the output file
        outFile << "Time Step: " << currentTime << "\n";
        outFile << "Floor: " << currentFloor << ", Direction: " << GetDirectionString(currentDirection) << "\n";
        outFile << "Passengers In Elevator: ";
        for (int dest : elevatorPassengers) {
            outFile << dest << " ";
        }
        outFile << "\n";
        outFile << "Waiting Passengers: ";
        for (int numWaiting : waitingPassengers) {
            outFile << numWaiting << " ";
        }
        outFile << "\n";
        outFile << "--\n";  // Use "--" to indicate the end of a time step

        // Process new requests and handle passengers
        ProcessIncomingRequests(currentTime);
        HandlePassengers(currentTime);

        // Update elevator's movement direction and move
        UpdateDirection();
        ExecuteMove(currentTime);

        // Early exit check - If all requests have been handled, end the simulation early
        if (pendingRequests.empty() && activeFloorRequests.empty()) {
            std::cout << "All requests have been processed. Ending simulation early at Time Step: " << currentTime << std::endl;
            outFile << "All requests have been processed. Ending simulation early at Time Step: " << currentTime << "\n";
            break;
        }

        std::cout << "End of Time Step: " << currentTime << std::endl;
        outFile << "End of Time Step: " << currentTime << "\n";
    }

    outFile.close();  // Close the output file
}


void ECElevatorSim::ProcessIncomingRequests(int time) {
    for (auto &request : pendingRequests) {
        if (request.GetTime() == time && !request.IsServiced()) {
            activeFloorRequests.insert(request.GetFloorSrc());
            waitingPassengers[request.GetFloorSrc() - 1]++;
            waitingDirections[request.GetFloorSrc() - 1] = (request.GetFloorDest() > request.GetFloorSrc()) ? 1 : -1;
            std::cout << "New Request: Floor " << request.GetFloorSrc() << " at Time " << time << std::endl;
        }
    }
}

void ECElevatorSim::HandlePassengers(int currentTime) {
    std::cout << "Handling Passengers at Floor: " << currentFloor << std::endl;

    // Debugging: Print out current state of passengers in the elevator before handling
    std::cout << "Current passengers in elevator before handling: ";
    for (const auto& dest : elevatorPassengers) {
        std::cout << dest << " ";
    }
    std::cout << std::endl;

    auto it = pendingRequests.begin();
    while (it != pendingRequests.end()) {
        if (it->GetFloorSrc() == currentFloor && !it->IsFloorRequestDone()) {
            // Passenger boards, request is handled
            it->SetFloorRequestDone(true);
            activeFloorRequests.insert(it->GetFloorDest());
            waitingPassengers[currentFloor - 1]--;

            // Debugging: Log before adding to elevatorPassengers
            std::cout << "Adding passenger to elevator with destination: " << it->GetFloorDest() << std::endl;

            elevatorPassengers.push_back(it->GetFloorDest());
            numberOfRiders++;

            // Debugging: Verify passenger was added
            std::cout << "Passenger Boarded at Floor: " << currentFloor
                      << ", Destination: " << it->GetFloorDest() << std::endl;
            std::cout << "Number of riders now: " << numberOfRiders << std::endl;

        } else if (it->GetFloorDest() == currentFloor && it->IsFloorRequestDone()) {
            // Passenger arrives at destination
            it->SetServiced(true);
            it->SetArriveTime(currentTime);
            std::cout << "Passenger Dropped at Floor: " << currentFloor
                      << ", Arrival Time: " << currentTime << std::endl;

            // Debugging: Log before removing from pendingRequests
            std::cout << "Removing serviced request for destination floor: " << it->GetFloorDest() << std::endl;

            it = pendingRequests.erase(it); // Remove completed requests
            numberOfRiders--;

            // Debugging: Verify the request was removed
            std::cout << "Request for floor " << currentFloor << " completed. Number of riders now: " << numberOfRiders << std::endl;

            continue;  
        }

        ++it;
    }

    // Debugging: Print out current active floor requests after handling
    std::cout << "Active floor requests after handling: ";
    for (const auto& req : activeFloorRequests) {
        std::cout << req << " ";
    }
    std::cout << std::endl;

    activeFloorRequests.erase(currentFloor); // Remove serviced floor

    // Debugging: Print out updated state of passengers in the elevator after handling
    std::cout << "Current passengers in elevator after handling: ";
    for (const auto& dest : elevatorPassengers) {
        std::cout << dest << " ";
    }
    std::cout << std::endl;

    // Debugging: Check waiting passengers at each floor
    std::cout << "Waiting passengers state: ";
    for (int i = 0; i < waitingPassengers.size(); ++i) {
        std::cout << "Floor " << (i + 1) << ": " << waitingPassengers[i] << " ";
    }
    std::cout << std::endl;
}

void ECElevatorSim::UpdateDirection() {
    std::cout << "Updating Direction. Current Floor: " << currentFloor
              << ", Active Requests: ";
    for (auto floor : activeFloorRequests) std::cout << floor << " ";
    std::cout << std::endl;

    if (activeFloorRequests.empty()) {
        currentDirection = EC_ELEVATOR_STOPPED;
        return;
    }

    int nextFloor = SelectNextFloor();
    if (nextFloor > currentFloor) {
        currentDirection = EC_ELEVATOR_UP;
    } else if (nextFloor < currentFloor) {
        currentDirection = EC_ELEVATOR_DOWN;
    } else {
        currentDirection = EC_ELEVATOR_STOPPED;
    }

    std::cout << "New Direction: " << GetDirectionString(currentDirection) << std::endl;
}

void ECElevatorSim::ExecuteMove(int timeStep) {
    if (currentDirection == EC_ELEVATOR_UP) {
        // Move directly to the next target floor
        int nextFloor = SelectNextFloor();
        currentFloor = nextFloor;
    } else if (currentDirection == EC_ELEVATOR_DOWN) {
        // Move directly to the next target floor
        int nextFloor = SelectNextFloor();
        currentFloor = nextFloor;
    }
    std::cout << "Moving to Floor: " << currentFloor
              << ", Direction: " << GetDirectionString(currentDirection)
              << ", Time: " << timeStep << std::endl;
}

int ECElevatorSim::SelectNextFloor() {
    // Find the closest request in the current direction
    int closestFloor = -1;
    int minDistance = INT_MAX;

    for (const auto &floor : activeFloorRequests) {
        int distance = floor - currentFloor;

        if ((currentDirection == EC_ELEVATOR_UP && distance > 0 && distance < minDistance) ||
            (currentDirection == EC_ELEVATOR_DOWN && distance < 0 && -distance < minDistance)) {
            closestFloor = floor;
            minDistance = std::abs(distance);
        }
    }

    // If no requests are found in the current direction, switch direction and find the closest
    if (closestFloor == -1) {
        currentDirection = (currentDirection == EC_ELEVATOR_UP) ? EC_ELEVATOR_DOWN : EC_ELEVATOR_UP;

        for (const auto &floor : activeFloorRequests) {
            int distance = std::abs(floor - currentFloor);
            if (distance < minDistance) {
                closestFloor = floor;
                minDistance = distance;
            }
        }
    }

    return closestFloor;
}

std::string ECElevatorSim::GetDirectionString(EC_ELEVATOR_DIR direction) const {
    switch (direction) {
        case EC_ELEVATOR_STOPPED: return "STOPPED";
        case EC_ELEVATOR_UP: return "UP";
        case EC_ELEVATOR_DOWN: return "DOWN";
        default: return "UNKNOWN";
    }
}


void ECElevatorSim::SimulateStep() {
    static int currentTimeStep = 0;

    std::cout << "---- Time Step: " << currentTimeStep << " ----" << std::endl;

    // Display current elevator state
    std::cout << "Current Floor: " << currentFloor 
              << ", Direction: " << currentDirection 
              << ", Active Requests: ";
    for (const auto &floor : activeFloorRequests) std::cout << floor << " ";
    std::cout << std::endl;

    // Process new requests and handle passengers
    ProcessIncomingRequests(currentTimeStep);
    HandlePassengers(currentTimeStep);

    // **Early Exit Check** - If all requests have been handled, end the simulation early
    if (pendingRequests.empty() && activeFloorRequests.empty()) {
        std::cout << "All requests have been processed. Ending simulation early at Time Step: " << currentTimeStep << std::endl;
        return;
    }

    // Update elevator's movement direction and move
    UpdateDirection();
    ExecuteMove(currentTimeStep);

    std::cout << "End of Time Step: " << currentTimeStep << std::endl;

    // Increment the time step for the next simulation
    ++currentTimeStep;
}

bool ECElevatorSim::IsSimulationComplete() const {
    bool allRequestsProcessed = pendingRequests.empty();
    bool noActiveRequests = activeFloorRequests.empty();
    bool elevatorStopped = currentDirection == EC_ELEVATOR_STOPPED;

    std::cout << "Check completion: " 
              << "Pending Requests Empty: " << allRequestsProcessed
              << ", Active Requests Empty: " << noActiveRequests
              << ", Elevator Stopped: " << elevatorStopped
              << std::endl;

    return allRequestsProcessed && noActiveRequests && elevatorStopped;
}




//*****************************************************************************
// New Getter Methods Implementation

std::vector<int> ECElevatorSim::GetWaitingPassengers() const {
    return waitingPassengers;
}

const std::vector<int> ECElevatorSim::GetElevatorPassengers() const {
    return elevatorPassengers;
}

std::vector<int> ECElevatorSim::GetWaitingDirections() const {
    return waitingDirections;
}

void ECElevatorSim::UpdateElevatorPassengers(const std::vector<int>& updatedPassengers) {
    elevatorPassengers = updatedPassengers;
}

void ECElevatorSim::UpdateWaitingPassengers(const std::vector<int>& updatedWaiting) {
    waitingPassengers = updatedWaiting;
}
