#include "db.h"

#include <filesystem>
#include <fstream>
#include <ios>

// DEFAULT LIST'S FIRST ELEMENT...FREE ZONE (zero if nonexistent)...STATUS...BLOCK

enum Block: char {
    TO_DELETE = 0,
    NODE = 1, //node for tree with end range and pointer to next node
    IN_USE = 2,
    OPEN_ZONE = 3 //block used for linking links to open zones (free spaces)
};

const uint64_t INDEX_SIZE = sizeof(uint64_t);
const uint64_t START_INDEX = INDEX_SIZE*2;

template<class T>
T read(std::fstream& file) {
    T x;
    file.read((char*)&x, sizeof(T));

    return x;
}

uint64_t read_uint(std::fstream& file) {
    return read<uint64_t>(file);
}

uint64_t get_eof(std::fstream& file) {
    file.seekg(0, std::fstream::end);
    return file.tellg();
}

uint64_t get_empty(std::fstream& file) {
    file.seekg(INDEX_SIZE);
    uint64_t empty = read_uint(file);

    if (empty != 0) return empty;
    else {
        file.seekg(1, std::fstream::end);
        return file.tellg();
    }
}

Database::Database(char const* path): file(path, std::ios::binary | std::ios::ate) {
    file.exceptions(std::fstream::badbit | std::fstream::failbit);
}

List Database::create_list() {
    file.seekg(0);
    bool empty = file.peek() == file.eof();

    List list;

    if (empty) {
        list = START_INDEX;

        file.seekp(0);
        file.write((char*)&list, INDEX_SIZE);
        file.seekg(list);
    } else
        list = get_empty(file);

    char status = NODE;
    file.write(&status, 1);

    uint64_t zero = 0;
    file.write((char*)&zero, INDEX_SIZE); //end range
    file.write((char*)&zero, INDEX_SIZE); //next element

    return list;
}

List Database::get_list() {
    file.seekg(0);
    //todo: no list exists should be a special exception
    List list = read_uint(file);

    return list;
}

//this isnt very efficient; i hope lists arent deleted often
void Database::delete_list(List const& list) {
    //we loop through elements and we update this position
    //to where all elements after it can be deleted and the file can be truncated
    uint64_t delete_from = LIST_SIZE; //initialize to first element

    //if default list is list, set delete_from to 0 (since the default list wont be needed)
    file.seekp(0);
    if(list == read_uint(file))
        delete_from = 0;

    file.seekp(list);

    uint64_t block_size;

    while(true) {
        char to_delete = 1;
        file.write(&to_delete, 1); //write true for deletion

        block_size = read_uint(file); //read block size

        if (block_size > 0) {
            file.seekg(block_size, std::fstream::cur); //skip block
            uint64_t next_block = read_uint(file);
            file.seekg(next_block); //go to next block
        } else {
            break;
        }
    }

    file.seekp(LIST_SIZE); //go to first element no matter which list

    while (!file.eof()) {
        char to_delete;
        file.read((char*)&to_delete, 1);

        uint64_t block_size = read_uint(file);
        file.seekg(block_size, std::fstream::cur); //skip block

        if (to_delete) {
            delete_from = (uint64_t)file.tellg() - sizeof(uint64_t) - 1; //get before block size and to delete
        } else {
            delete_from = file.tellg(); //reset to current position (after block)
            //after all blocks this should be EOF
        }
    }

    if (delete_from != file.tellg()) { //if its not EOF, we truncate file to delete_from

    }
}
