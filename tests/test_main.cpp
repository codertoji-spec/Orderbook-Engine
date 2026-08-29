#include <iostream>

extern void runOrderBookTests();
extern void runMatchingTests();
extern void runCancellationTests();
extern void runConcurrencyTests();

int main() {
    std::cout << "Running tests...\n";
    
    runOrderBookTests();
    runMatchingTests();
    runCancellationTests();
    runConcurrencyTests();
    
    std::cout << "All tests passed!\n";
    return 0;
}
