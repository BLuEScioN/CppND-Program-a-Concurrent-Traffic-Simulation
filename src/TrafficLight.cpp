#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mtx);
    _cond.wait(uLock, [this] { return !_q.empty(); });
    T msg = std::move(_q.front());
    _q.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T&& msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mtx);
    _q.emplace_back(msg);
    _cond.notify_one(); 
}

/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

      	TrafficLightPhase nextPhase = _q.receive();
        if (nextPhase == TrafficLightPhase::green) { return; };
    }
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // init stop watch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();
    
    // Generate a random value between 4 and 6 (ms)
    std::random_device rd;
    std::mt19937 eng(rd());
  	std::uniform_real_distribution<> distr(4000.0, 6000.0);
   	double phaseDuration = distr(eng);
    
    while (true) {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if (timeSinceLastUpdate >= phaseDuration) {
            if (_currentPhase == TrafficLightPhase::green) {
                _currentPhase = TrafficLightPhase::red;
            } else {
                _currentPhase = TrafficLightPhase::green;
            }

          	// send update message with new phase
            _q.send(std::move(_currentPhase));

            // reset stop watch
            lastUpdate = std::chrono::system_clock::now();
        }
    }
}
