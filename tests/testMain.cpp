#include <iostream>
#include <sstream>

#include "../utl.h"

#include "test.hpp"

using namespace contourtree;



int main() {
    generateData("toy.raw");
    toyProcessing("toy.raw");
    toyFeatures("toy.raw");

}

