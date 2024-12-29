

//
//  ECElevatorSim.cpp
//  
//
//  Created by Yufeng Wu on 6/27/23.
//  Elevator simulation


#include "ECElevatorSim.h"
#include <map>

ECElevatorSim::ECElevatorSim(int totalFloors, std::vector<ECElevatorSimRequest> &requestsList)
    : floorCount(totalFloors), currentFloor(1), currentDirection(EC_ELEVATOR_STOPPED), pendingRequests(requestsList) {}

ECElevatorSim::~ECElevatorSim() {}

void ECElevatorSim::Simulate(int simulationDuration) {
    for (int timeStep = 0; timeStep < simulationDuration; ++timeStep) {
        std::cout << "---- Time Step: " << timeStep << " ----" << std::endl;

        // Display current elevator state
        std::cout << "Current Floor: " << currentFloor 
                  << ", Direction: " << currentDirection 
                  << ", Active Requests: ";
        for (const auto &floor : activeFloorRequests) std::cout << floor << " ";
        std::cout << std::endl;

        // Process new requests and handle passengers
        ProcessIncomingRequests(timeStep);
        HandlePassengers(timeStep);

        // Update elevator's movement direction and move
        UpdateDirection();
        ExecuteMove(timeStep);

        std::cout << "End of Time Step: " << timeStep << std::endl;
    }
}

bool ECElevatorSim::ProcessIncomingRequests(int time) {
    bool newRequestAdded = false;
    for (auto &request : pendingRequests) {
        if (request.GetTime() == time && !request.IsServiced()) {
            activeFloorRequests.insert(request.GetRequestedFloor());
            newRequestAdded = true;
            std::cout << "New Request: Floor " << request.GetRequestedFloor() 
                      << " at Time " << time << std::endl;
        }
    }
    return newRequestAdded;
}

void ECElevatorSim::HandlePassengers(int &time) {
    std::cout << "Handling Passengers at Floor: " << currentFloor << std::endl;

    auto it = pendingRequests.begin();
    while (it != pendingRequests.end()) {
        if (it->GetRequestedFloor() == currentFloor && !it->IsFloorRequestDone()) {
            // Passenger boards, request is handled
            it->SetFloorRequestDone(true);
            activeFloorRequests.insert(it->GetFloorDest());
            std::cout << "Passenger Boarded at Floor: " << currentFloor 
                      << ", Destination: " << it->GetFloorDest() << std::endl;
        } else if (it->GetFloorDest() == currentFloor && it->IsFloorRequestDone()) {
            // Passenger arrives at destination
            it->SetServiced(true);
            it->SetArriveTime(time);
            std::cout << "Passenger Dropped at Floor: " << currentFloor 
                      << ", Arrival Time: " << time << std::endl;
            it = pendingRequests.erase(it); // Remove completed requests
            continue;
        }
        ++it;
    }
    activeFloorRequests.erase(currentFloor); // Remove serviced floor
}

void ECElevatorSim::UpdateDirection() {
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
}

void ECElevatorSim::ExecuteMove(int time) {
    if (currentDirection == EC_ELEVATOR_UP) {
        ++currentFloor;
    } else if (currentDirection == EC_ELEVATOR_DOWN) {
        --currentFloor;
    }
    std::cout << "Moving to Floor: " << currentFloor 
              << ", Direction: " << currentDirection 
              << ", Time: " << time << std::endl;
}

int ECElevatorSim::SelectNextFloor() {
    int closestFloor = *activeFloorRequests.begin();
    int minDistance = abs(currentFloor - closestFloor);

    for (const auto &floor : activeFloorRequests) {
        int distance = abs(currentFloor - floor);
        if (distance < minDistance) {
            closestFloor = floor;
            minDistance = distance;
        }
    }
    return closestFloor;
}

bool ECElevatorSim::HasFloorsAbove() const {
    for (const auto &request : pendingRequests) {
        if (request.GetRequestedFloor() > currentFloor && !request.IsServiced()) {
            return true;
        }
    }
    return false;
}

bool ECElevatorSim::HasFloorsBelow() const {
    for (const auto &request : pendingRequests) {
        if (request.GetRequestedFloor() < currentFloor && !request.IsServiced()) {
            return true;
        }
    }
    return false;
}