#ifndef __MMU_H_
#define __MMU_H_

#include <iostream>
#include <string>
#include <vector>

enum DataType : uint8_t {FreeSpace, Char, Short, Int, Float, Long, Double};

typedef struct Variable {
    std::string name;
    DataType type;
    uint32_t virtual_address;
    uint32_t size;
} Variable;

typedef struct Process {
    uint32_t pid;
    std::vector<Variable*> variables;
} Process;

class Mmu {
private:
    uint32_t _next_pid;
    uint32_t _max_size;
    std::vector<Process*> _processes;

public:
    Mmu(int memory_size);
    ~Mmu();

    uint32_t createProcess();
    void addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address);
    void removeVariableFromProcess(uint32_t pid, uint32_t address);

    void print();

    uint32_t getAddress(uint32_t pid, std::string var_name);
    uint32_t getSize(uint32_t pid, std::string var_name);

    uint32_t emptyPage(uint32_t pid, uint32_t page_size);

    uint32_t getFreeSpace(uint32_t pid, uint32_t size, uint32_t allocatedSpace);
    void modifyFreeSpace(u_int32_t pid, uint32_t size, uint32_t address);
    void newFreeSpaceGap(u_int32_t pid, uint32_t size, uint32_t address);
    void newFreeSpacePage(u_int32_t pid, uint32_t page_size, uint32_t page_address);
    void restoreFreeSpace(uint32_t pid, uint32_t address, uint32_t size, uint32_t page_size);
    bool modifyTotalSpace(u_int32_t pid, uint32_t size);
};

#endif // __MMU_H_
