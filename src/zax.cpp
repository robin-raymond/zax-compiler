
#include "pch.h"
#include "zax.h"

#include "Tokenizer.h"

using namespace zax;

int runAllTests() noexcept;

int main(int argc, const char* const argv[]) {

    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] " << argv[i] << "\n";
    }

#ifdef _DEBUG
    runAllTests();
#endif //_DEBUG

    //auto length = lut.longestSymbol('^');

    std::cout << "Hello World!\n";
}
