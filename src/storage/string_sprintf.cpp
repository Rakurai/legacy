#include <iostream>
#include <string>
#include <cstdio>

#include "string_sprintf.h"

int main() {
    std::string name = "World";
    my_printf("Hello, %s %d!\n", name, 5);
}
