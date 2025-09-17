#include <iostream>
#include "p3em.hpp"

int main() {
    std::thread monitorThread(launchScriptAndMonitor);
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait a bit for init

    // Simulate usage
    for (int i = 0; i < 10; ++i) {
        std::cout << "Latest value: " << getLatestValue() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    // Cleanup
    if (scriptPid > 0) killpg(scriptPid, SIGTERM); // Kill entire process group
    monitorThread.join();
    return 0;
}
