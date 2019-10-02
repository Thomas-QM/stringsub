#include "log_structure.h"

#include "database.h"

uint64_t LogStructure::each_length() {
    return sizeof(uint64_t) * (NUM_INDICES ^ level);
}

uint64_t LogStructure::length() {
    return BLOCK_SIZE * (NUM_INDICES ^ level);
}

LogStructure LogStructure::derive() {
    LogStructure log;
    log.level = level + 1;

    return log;
}

void LogStructure::empty_data(char* data) {
    data[0] = level;
}

void LogStructure::operate_target(std::function<void(char*, uint64_t, uint64_t)> base_op,
                                  std::function<void(char*, uint64_t, uint64_t)> target_op,
                                  uint64_t file_pos, char* buffer, uint64_t pos, uint64_t size) {
    if (level == 0) {
        base_op(buffer, file_pos + pos, size);
    }

    uint64_t start_idx = pos / each_length();
    uint64_t start_offset = pos % each_length();

    uint64_t idx_span = size / each_length();
    //get all indices needed in one go
    uint64_t* file_positions = new uint64_t[idx_span];
    base_op((char*)file_positions, file_pos + (start_idx*sizeof(uint64_t)), idx_span*sizeof(uint64_t));

    LogStructure sub;
    sub.level = level-1;

    for (uint64_t i = 0; i < idx_span; i++) { // spans multiple indices
        //read chunk recursively
        sub.operate_target(target_op, target_op,
                           file_positions[i], buffer + (each_length() * i), start_offset, std::min(size, BLOCK_SIZE));

        size -= each_length();
        start_offset = 0; //i hope this gets optimized or something idk
    }
}

void LogStructure::operate(std::function<void(char*, uint64_t, uint64_t)> op,
        uint64_t file_pos, char* buffer, uint64_t pos, uint64_t size) {

    operate_target(op, op, file_pos, buffer, pos, size);
}

//use lambdas cuz we lazy arses

void LogStructure::write_db(Database* db_ptr, uint64_t file_pos, char* buffer, uint64_t pos, uint64_t write_size) {
    operate([db_ptr](char* buffer, uint64_t pos, uint64_t size) { db_ptr->write(buffer, pos, size); },
            file_pos, buffer, pos, write_size);
}

void LogStructure::read_db(Database* db_ptr, uint64_t file_pos, char* buffer, uint64_t pos, uint64_t read_size) {
    operate([db_ptr](char* buffer, uint64_t pos, uint64_t size) { db_ptr->read(buffer, pos, size); },
            file_pos, buffer, pos, read_size);
}
