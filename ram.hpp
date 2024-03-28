#pragma once

#include <cstddef> 
#include <string>  

namespace nuPython {

enum class RAM_CELL_TYPES
{
    RAM_TYPE_INT,
    RAM_TYPE_REAL,
    RAM_TYPE_STR,
    RAM_TYPE_PTR,
    RAM_TYPE_BOOLEAN,
    RAM_TYPE_NONE
};

class RAM_CELL
{
public:
    union
    {
        int i;
        double d;
        char* s; 
    } types;

    std::string identifier;  
    RAM_CELL_TYPES ram_cell_type; 
};

class RAM
{
public:
    RAM_CELL* cells;  
    int num_values;   // Number of values currently stored
    int memory_size;  // Total number of cells available

    RAM();            // Constructor for initialization
    ~RAM();           // Destructor for cleanup
};

// Function declarations
RAM* ram_init();
void ram_free(RAM* memory);

int ram_get_addr(RAM* memory, const char* identifier);

RAM_CELL* ram_get_cell_by_id(RAM* memory, const char* identifier);
RAM_CELL* ram_get_cell_by_addr(RAM* memory, int address);

void ram_write_int_by_id(RAM* memory, const char* identifier, int value);
void ram_write_int_by_addr(RAM* memory, int address, int value);
void ram_write_real_by_id(RAM* memory, const char* identifier, double value);
void ram_write_real_by_addr(RAM* memory, int address, double value);
void ram_write_str_by_id(RAM* memory, const char* identifier, const char* value);
void ram_write_str_by_addr(RAM* memory, int address, const char* value);
void ram_write_ptr_by_id(RAM* memory, const char* identifier, int value);
void ram_write_ptr_by_addr(RAM* memory, int address, int value);

void ram_print(RAM* memory);

} // namespace nuPython
