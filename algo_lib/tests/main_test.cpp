#include <iostream>
#include "test_algorithms.hpp"
#include "test_datastructures.hpp"
#include "test_sorting.hpp"

int main() {
    runAlgorithmTests();
    runDataStructureTests();
    runSortingTests();
    std::cout << "All tests passed!" << std::endl;
}
