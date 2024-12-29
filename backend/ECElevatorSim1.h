#ifndef ECELEVATORSIM1_H
#define ECELEVATORSIM1_H

#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <string>

//*****************************************************************************
// Elevator simulation request: 
class ECElevatorSimRequest
{
public:
    ECElevatorSimRequest(int timeIn, int floorSrcIn, int floorDestIn) 
        : time(timeIn), floorSrc(floorSrcIn), floorDest(floorDestIn), 
          fFloorReqDone(false), fServiced(false), timeArrive(-1) {}

    int GetTime() const { return time; }
    int GetFloorSrc() const { return floorSrc; }
    int GetFloorDest() const { return floorDest; }
    bool IsServiced() const { return fServiced; }
    void SetServiced(bool f) { fServiced = f; }
    bool IsFloorRequestDone() const { return fFloorReqDone; }
    void SetFloorRequestDone(bool f) { fFloorReqDone = f; }
    int GetArriveTime() const { return timeArrive; }
    void SetArriveTime(int t) { timeArrive = t; }
    

private:
    int time;
    int floorSrc;
    int floorDest;
    bool fFloorReqDone;
    bool fServiced;
    int timeArrive;
};

//*****************************************************************************
// Elevator moving direction
typedef enum {
    EC_ELEVATOR_STOPPED = 0,
    EC_ELEVATOR_UP,
    EC_ELEVATOR_DOWN
} EC_ELEVATOR_DIR;

//*****************************************************************************
// Simulation of elevator
class ECElevatorSim
{
public:
    ECElevatorSim(int totalFloors, std::vector<ECElevatorSimRequest> &requestsList);
    ~ECElevatorSim();

    void Simulate(int simulationDuration, const std::string& outputFilename);   
    void SimulateStep();  // Stepwise simulation
    bool IsSimulationComplete() const;
        const std::set<int>& GetActiveFloorRequests() const;

    

    // Getters to interact with the frontend
    int GetTotalFloors() const { return floorCount; }
    int GetCurrentFloor() const { return currentFloor; }
    int GetFloorCount() const { return floorCount; }
    int GetSimulationDuration() const { return simulationDuration; }
    bool IsGoingUp() const { return currentDirection == EC_ELEVATOR_UP; }
    bool IsGoingDown() const { return currentDirection == EC_ELEVATOR_DOWN; }
    std::vector<int> GetWaitingPassengers() const;
    int GetCurrentTime() const { return currentTime; }
    int GetNumberOfRiders() const { return numberOfRiders; }
    const std::vector<int> GetElevatorPassengers() const;
    std::vector<int> GetWaitingDirections() const;

    void ExecuteMove() { ExecuteMove(currentTime); }

    // New methods to allow `ElevatorHandler` access
    std::vector<ECElevatorSimRequest>& GetPendingRequests() { return pendingRequests; }
    void AddActiveFloorRequest(int floor) { activeFloorRequests.insert(floor); }
    void RemoveActiveFloorRequest(int floor) { activeFloorRequests.erase(floor); }
    void AddPassengerToElevator(int destination) { elevatorPassengers.push_back(destination); }
    void RemovePassengerFromElevator(int floor) {
        auto passengerIt = std::find(elevatorPassengers.begin(), elevatorPassengers.end(), floor);
        if (passengerIt != elevatorPassengers.end()) {
            elevatorPassengers.erase(passengerIt);
        }
    }
    void IncrementRiders() { numberOfRiders++; }
    void DecrementRiders() { if (numberOfRiders > 0) numberOfRiders--; }
    void DecrementWaitingPassengers(int index) { 
        if (index >= 0 && index < waitingPassengers.size() && waitingPassengers[index] > 0) 
            waitingPassengers[index]--; 
    }
    
    void UpdateElevatorPassengers(const std::vector<int>& updatedPassengers);
    void UpdateWaitingPassengers(const std::vector<int>& updatedWaiting);



private:
    int floorCount;                    
    int simulationDuration;            
    int currentFloor;                  
    EC_ELEVATOR_DIR currentDirection;  
    int currentTime;                   
    int numberOfRiders;                

    std::vector<ECElevatorSimRequest> pendingRequests; 
    std::set<int> activeFloorRequests; 
    std::vector<int> elevatorPassengers; 
    std::vector<int> waitingPassengers; 
    std::vector<int> waitingDirections; 

    void ProcessIncomingRequests(int time);
    void HandlePassengers(int currentTime); 
    void UpdateDirection();
    void ExecuteMove(int timeStep);
    int SelectNextFloor();
    std::string GetDirectionString(EC_ELEVATOR_DIR direction) const;
};

#endif /* ECELEVATORSIM1_H */
