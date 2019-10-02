#ifndef STRINGSUB_LOG_STRUCTURE_H
#define STRINGSUB_LOG_STRUCTURE_H

#include <vector>
#include <cstdint>
#include <functional>

#include "definitions.h"

class LogStructure {
public:
    static const uint64_t NUM_INDICES = BLOCK_SIZE / sizeof(uint64_t); //64 indices
    static const uint64_t SIZE = BLOCK_SIZE + 1; //index with level byte is 4097 bytes

    /// 0 = data, 1 and up are index blocks
    /// only 255 levels supported, which is around 1024/16 (size of each index)^255 ( i think ) bytes able to be stored by one block
    char level;

    uint64_t each_length();

    //so we can just use pos + size > length to check if its overflown, then just allocate a new logstructure
    uint64_t length();
    //derive a new logstructure with one more level
    LogStructure derive();
    //fill first char of data with level
    void empty_data(char* data);

    //with base and target, we can operate on memory and file at the same time
    void operate_target(std::function<void(char*, uint64_t, uint64_t)> base_op,
                        std::function<void(char*, uint64_t, uint64_t)> target_op,
                        uint64_t file_pos, char* buffer, uint64_t pos, uint64_t size);

    void operate(std::function<void(char*, uint64_t, uint64_t)> op,
            uint64_t file_pos, char* buffer, uint64_t pos, uint64_t size);

    void write_db(Database* db_ptr, uint64_t file_pos, char* buffer, uint64_t pos, uint64_t write_size);
    void read_db(Database* db_ptr, uint64_t file_pos, char* buffer, uint64_t pos, uint64_t read_size);
};


#endif //STRINGSUB_LOG_STRUCTURE_H
