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

    void removeProcess(uint32_t pid);

    uint32_t getAddress(uint32_t pid, std::string var_name);
    uint32_t getSize(uint32_t pid, std::string var_name);

    uint32_t isEmptyPage(uint32_t pid, uint32_t from, uint32_t to);

    uint32_t getFreeSpace(uint32_t pid, uint32_t size, u_int32_t allocatedSpace);
    void modifyFreeSpace(u_int32_t pid, uint32_t size, uint32_t address, uint32_t offset);
    void restoreFreeSpace(uint32_t pid, uint32_t address, uint32_t size);

    bool validProcess(uint32_t pid);
    bool validVar(uint32_t pid, std::string var_name);
    DataType returnDatatype(uint32_t pid, std::string var_name);

    std::vector<Process*> getProcesses();

};

#endif // __MMU_H_
