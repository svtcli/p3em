#include <iostream>
#include <thread>
#include "p3em.hpp"

int main() {

    p3em myem("../p3em.sh");
    // Simulate usage
    for (int i = 0; i < 10; ++i) {
        std::cout << "Latest value: " << myem.getLatestValue() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    return 0;
}
