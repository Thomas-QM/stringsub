#ifndef STRINGSUB_DATABASE_H
#define STRINGSUB_DATABASE_H

#include <vector>
#include <mutex>
#include <fstream>
#include <thread>

#include "definitions.h"
#include "log_structure.h"

class Database {
public:
    std::fstream file;
    std::mutex file_mutex;

    Database(const char* filename);

    bool is_eof(uint64_t pos);

    void read(char* buffer, uint64_t pos, uint64_t read_size);
    void write(char* bytes, uint64_t pos, uint64_t write_size);

    void flush();

    template<class T>
    T read(uint64_t pos) {
        T t;
        read((char*)&t, pos, sizeof(t));
        return t;
    }

    template<class T>
    void write(T& t, uint64_t pos) {
        write((char*)t, pos, sizeof(T));
    }

    //run flush & close file
    ~Database();

private:
    //writes and write queue
    struct Write {
        uint64_t pos;
        uint64_t size;
        char* bytes;
    };

    void flush_locked();

    bool writing = false;
    std::vector<Write> write_queue;

    //empty blocks
    static const uint64_t EMPTIES_HEADER_SIZE = sizeof(uint64_t) + 1;
    static const uint64_t EMPTIES_SIZE = EMPTIES_HEADER_SIZE + (BLOCK_SIZE / sizeof(uint64_t));

    static const uint64_t INDICES_EMPTIES = LogStructure::SIZE;
    static const uint64_t DYNAMIC_EMPTIES = LogStructure::SIZE + EMPTIES_SIZE;

    void add_empty(uint64_t pos, uint64_t size);
};


#endif //STRINGSUB_DATABASE_H
