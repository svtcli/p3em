//   Copyright(C) 2025 Salvatore Cielo, Leibniz-Rechenzentrum
//   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
//   Copyright(C) 2025 Ivan Pribec, Leibniz-Rechenzentrum
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
//  License. You may obtain a copy of the License at    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.

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
