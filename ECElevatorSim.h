
//
//  ECElevatorSim.h
//  
//
//  Created by Yufeng Wu on 6/27/23.
//  Elevator simulation


#ifndef ECELEVATORSIM_H
#define ECELEVATORSIM_H

#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <string>

//*****************************************************************************
// DON'T CHANGE THIS CLASS
// 
// Elevator simulation request: 
// (i) time: when the request is made
// (ii) floorSrc: which floor the user is at at present
// (iii) floorDest floor: where the user wants to go; we assume floorDest != floorSrc
// 
// Note: a request is in three stages:
// (i) floor request: the passenger is waiting at floorSrc; once the elevator arrived 
// at the floor (and in the right direction), move to the next stage
// (ii) inside request: passenger now requests to go to a specific floor once inside the elevator
// (iii) Once the passenger arrives at the floor, this request is considered to be "serviced"
//
// two special requests:
// (a) maintenance start: floorSrc=floorDest=-1; put elevator into maintenance 
// starting at the specified time; elevator starts at the current floor
// (b) maintenance end: floorSrc=floorDest=0; put elevator back to operation (from the current floor)

class ECElevatorSimRequest
{
public:
    ECElevatorSimRequest(int timeIn, int floorSrcIn, int floorDestIn) : time(timeIn), floorSrc(floorSrcIn), floorDest(floorDestIn), fFloorReqDone(false), fServiced(false), timeArrive(-1) {}
    ECElevatorSimRequest(const ECElevatorSimRequest &rhs) : time(rhs.time), floorSrc(rhs.floorSrc), floorDest(rhs.floorDest), fFloorReqDone(rhs.fFloorReqDone), fServiced(rhs.fServiced), timeArrive(rhs.timeArrive) {}
    int GetTime() const { return time; }
    int GetFloorSrc() const { return floorSrc; }
    int GetFloorDest() const { return floorDest; }
    bool IsGoingUp() const { return floorDest >= floorSrc; }

    // Is this passenger in the elevator or not
    bool IsFloorRequestDone() const { return fFloorReqDone; }
    void SetFloorRequestDone(bool f) { fFloorReqDone = f; }

    // Is this event serviced (i.e., the passenger has arrived at the destination)?
    bool IsServiced() const { return fServiced; }
    void SetServiced(bool f) { fServiced = f; }

    // Get the floor to service
    // If this is in stage (i): waiting at a floor, return that floor waiting at
    // If this is in stage (ii): inside an elevator, return the floor going to
    // Otherwise, return -1
    int GetRequestedFloor() const {
        if (IsServiced()) {
            return -1;
        }
        else if (IsFloorRequestDone()) {
            return GetFloorDest();
        }
        else {
            return GetFloorSrc();
        }
    }

    // Wait time: get/set. Note: you need to maintain the wait time yourself!
    int GetArriveTime() const { return timeArrive; }
    void SetArriveTime(int t) { timeArrive = t; }

    // Check if this is the special maintenance start request
    bool IsMaintenanceStart() const { return floorSrc == -1 && floorDest == -1; }
    bool IsMaintenanceEnd() const { return floorSrc == 0 && floorDest == 0; }

    void ValidateRequest(); 

private:
    int time;           // time of request made
    int floorSrc;       // which floor the request is made
    int floorDest;      // which floor is going
    bool fFloorReqDone;   // is this passenger passing stage one (no longer waiting at the floor) or not
    bool fServiced;     // is this request serviced already?
    int timeArrive;     // when the user gets to the destination floor
};

//*****************************************************************************
// Elevator moving direction
typedef enum
{
    EC_ELEVATOR_STOPPED = 0,    // not moving
    EC_ELEVATOR_UP,             // moving up
    EC_ELEVATOR_DOWN            // moving down
} EC_ELEVATOR_DIR;

//*****************************************************************************
// Add your own classes here...

// Base Elevator class to demonstrate inheritance and virtual functions
class ElevatorBase
{
public:
    virtual ~ElevatorBase() {}
    virtual void ExecuteMove(int currentTime) = 0; // Abstract function for elevator movement
};

//*****************************************************************************
// Simulation of elevator

class ECElevatorSim : public ElevatorBase
{
public:
    // numFloors: number of floors serviced (floors numbers from 1 to numFloors)
    ECElevatorSim(int totalFloors, std::vector<ECElevatorSimRequest> &requestsList);
    ~ECElevatorSim();

    void Simulate(int simulationDuration);

    // Query elevator state
    int GetNumFloors() const { return floorCount; }
    int GetCurrFloor() const { return currentFloor; }
    void SetCurrFloor(int floor) { currentFloor = floor; }
    EC_ELEVATOR_DIR GetCurrDir() const { return currentDirection; }
    void SetCurrDir(EC_ELEVATOR_DIR direction) { currentDirection = direction; }

    void ExecuteMove(int currentTime) override;


private:
    int floorCount;                      // Total number of floors serviced
    int currentFloor;                    // Current elevator floor
    EC_ELEVATOR_DIR currentDirection;    // Current movement direction
    std::vector<ECElevatorSimRequest> &pendingRequests; // List of requests
    std::set<int> activeFloorRequests;   // Pending floor service requests

    bool ProcessIncomingRequests(int time);
    void HandlePassengers(int &time);
    int SelectNextFloor();
    void UpdateDirection();
    bool HasFloorsAbove() const;
    bool HasFloorsBelow() const;
};

#endif /* ECELEVATORSIM_H */