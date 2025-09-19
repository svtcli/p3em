#include "p3em.hpp"
#include <string>

extern "C" {

void *handle_to_p3em(const char *filename) {
    // Construct p3em on the heap using std::string(filename)
    p3em *handle = new p3em(std::string(filename));
    return static_cast<void*>(handle);
}

void free_handle(void *handle) {
    p3em *my_handle = static_cast<p3em *>(handle);
    delete my_handle;  // calls destructor and frees memory
}

int p3em_getLatestValue(void *handle) {
    return static_cast<p3em *>(handle)->getLatestValue();
}

}
