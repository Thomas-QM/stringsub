#include <cassert>
#include <iostream>
#include <cstring>

#include "database.h"

int main() {
    Database db("./data.txt");

    char i_like_dogs[] = "i like dogs";
    db.write(i_like_dogs, 0);

    char x[15];
    db.read(x, 0, 12);
    for (char y: x) {
        std::cout << (int)y << std::endl;
    }

    assert( strcmp(x, "i like dogs\0") );

    return 0;
}