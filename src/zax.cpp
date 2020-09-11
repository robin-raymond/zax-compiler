
#include "pch.h"
#include "zax.h"

int main(int argc, const char* const argv[]) {

    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] " << argv[i] << "\n";
    }
    std::cout << "Hello World!\n";
}
