#include "database.h"
#include "log_structure.h"

Database::Database(const char *filename): file(), write_queue() {
    file.open(filename, std::fstream::in | std::fstream::out | std::fstream::binary);
    //if unopened,
    if (!file.good()) {
        //create file
        std::ofstream file_creator(filename);
        file_creator.open(filename, std::fstream::out);

        { //write tables logstructure
            char tables[LogStructure::SIZE];

            LogStructure ls;
            ls.empty_data(tables);

            file_creator.write((char*) &tables, LogStructure::SIZE);
        }

        { //write empties for indices
            char indices_empties[EMPTIES_SIZE];
            uint64_t last_idx = INDICES_EMPTIES + EMPTIES_HEADER_SIZE;
            memcpy(&indices_empties, &last_idx, sizeof(uint64_t));

            file_creator.write((char*)&indices_empties, EMPTIES_SIZE);
        }

        { //write empties for dynamically-sized blocks
            char dynamic_empties[EMPTIES_SIZE];
            uint64_t last_idx = DYNAMIC_EMPTIES + EMPTIES_HEADER_SIZE;
            memcpy(&dynamic_empties, &last_idx, sizeof(uint64_t));

            file_creator.write((char*)&dynamic_empties, EMPTIES_SIZE);
        }

        file_creator.close();

        //reopen file
        file.open(filename, std::fstream::in | std::fstream::out | std::fstream::binary);
    }
}

bool Database::is_eof(uint64_t pos) {
    file.seekg(pos);
    return file.eof();
}

void Database::read(char* buffer, uint64_t pos, uint64_t read_size) {
    std::lock_guard<std::mutex> lock(file_mutex);

    file.seekg(pos);
    file.read(buffer, read_size);
}

void Database::write(char* to_copy, uint64_t pos, uint64_t write_size) {
    char* bytes = new char[write_size];
    std::copy(to_copy, to_copy+write_size, bytes);

    write_queue.push_back(Write { pos, write_size, bytes });

    if (!writing) {
        writing = true;

        file_mutex.lock(); //lock before thread starts
        std::thread writing_thread([this] {
            this->flush_locked();
            this->writing = false;
        });

        writing_thread.detach();
    }
}

void Database::flush_locked() {
    std::vector<Write> tmp_write_queue;
    std::swap(write_queue, tmp_write_queue);

    //TODO: we can order write_queue by pos to optimize writes
    for (Write& x: tmp_write_queue) {
        file.seekp(x.pos);
        file.write(x.bytes, x.size);

        delete[] x.bytes;
    }

    file.flush();
    file_mutex.unlock();
}

void Database::flush() {
    file_mutex.lock();
    flush_locked();
}

void Database::add_empty(uint64_t pos, uint64_t size) {
    uint64_t last_idx;

    if (size == BLOCK_SIZE) {
        last_idx = read<uint64_t>(INDICES_EMPTIES);
    } else {
        uint16_t full_bytes = size / 255;

        const unsigned char U8_MAX = 255;
        unsigned char half_byte = size % U8_MAX;

        for (uint16_t b=0; b>full_bytes; b++) {
            write<unsigned char>(U8_MAX, pos);
        }

        last_idx = read<uint64_t>(DYNAMIC_EMPTIES);
    }

    write(&pos)
}

Database::~Database() {
    //will lock mutex and ensure thread is stopped
    flush();

    file.close();
}
