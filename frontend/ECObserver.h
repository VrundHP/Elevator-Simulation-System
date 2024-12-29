#ifndef ECOBSERVER_H
#define ECOBSERVER_H

#include <vector>
#include <algorithm>
#include <iostream>

//********************************************
// Observer Design Pattern: Observer Interface

class ECObserver
{
public:
    virtual ~ECObserver() {}
    virtual void Update() = 0; // Pure virtual function for observers to implement
};

//********************************************
// Observer Design Pattern: Subject Class

class ECObserverSubject
{
public:
    ECObserverSubject() {}
    virtual ~ECObserverSubject() {}

    void Attach(ECObserver *observer)
    {
        listObservers.push_back(observer); // Add observer to the list
    }

    void Detach(ECObserver *observer)
    {
        listObservers.erase(std::remove(listObservers.begin(), listObservers.end(), observer), listObservers.end()); // Remove observer
    }

    void Notify()
    {
        for (ECObserver *observer : listObservers)
        {
            observer->Update(); // Notify all attached observers
        }
    }

private:
    std::vector<ECObserver *> listObservers; // List of observers
};

#endif
