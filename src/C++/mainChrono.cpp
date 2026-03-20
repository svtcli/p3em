#include <chrono>
#include <iostream>
#include <thread>

#include "p3emChrono.hpp"

class Timer {
private:
  std::chrono::system_clock::time_point start_time;

public:
  Timer() {
    p3emChrono::system_clock::init();
  } // ADD THIS LINE to constructor

  void start() {
    start_time = p3emChrono::system_clock::now();
  } // Change std::chrono to p3emChrono:: everywhere

  double elapsed() {
    auto end_time = p3emChrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    );
    return duration.count() / 1000.0;  // Return seconds as double
  }
};

int main() {
    Timer timer;

    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2300));

    std::cout << "Elapsed: " << timer.elapsed() << " seconds\n";
    return 0;
}
