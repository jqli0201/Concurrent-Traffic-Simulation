#include <iostream>
#include <random>
#include <chrono>

#include "TrafficLight.h"


/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and 
    // _condition.wait() to wait for and receive new messages and pull them 
    // from the queue using move semantics. The received object should then be 
    // returned by the receive function. 
  
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
  
    // pass unique lock to condition variable
    _cond.wait(uLock, [this] { return !_queue.empty(); });

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    // will not be copied due to return value optimization (RVO) in c++
    return msg; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "  Message " << msg 
        << " has been sent to the queue" << std::endl;
    _queue.push_front(std::move(msg));
    // notify client after pushing new Vehicle into vector
    _cond.notify_one(); 
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, 
    // in which an infinite while-loop runs and repeatedly 
    // calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (_message.receive() != green);
  return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be 
    // started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the 
    // time between two loop cycles and toggles the current phase of the traffic
    // light between red and green and sends an update method to the message 
    // queue using move semantics. The cycle duration should be a random value 
    // between 4 and 6 seconds. Also, the while-loop should use 
    // std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto start = std::chrono::high_resolution_clock::now();
    int randNum = rand() % (6001 - 4000) + 4000;
  	auto random = std::chrono::duration<int>(randNum*1000000);
    while (true) {
      auto now = std::chrono::high_resolution_clock::now();
      auto duration = now - start;
      if (duration >= random) {
        randNum = rand() % (6001 - 4000) + 4000;
        random = std::chrono::duration<int>(randNum*1000000);
        start = std::chrono::high_resolution_clock::now();
        switch(_currentPhase) {          
          case red:
            _currentPhase = green;
            break;
          case green:
            _currentPhase = red;
            break;
        }
        // sends an update method to the message queue using move semantics 
        _message.send(TrafficLightPhase(_currentPhase));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
}
