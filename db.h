#ifndef STRINGSUB_DB_H
#define STRINGSUB_DB_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

class Item {
    std::vector<std::string> keywords;
    std::string content;
};

typedef uint64_t List;

class Database {
    std::fstream file;

public:
    explicit Database(char const* path);

    List create_list();
    List get_list();
    void delete_list(List const& list);
};

#endif //STRINGSUB_DB_H
