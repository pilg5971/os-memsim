#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }

    Variable *var = new Variable();
    var->name = var_name;
    var->type = type;
    var->virtual_address = address;
    var->size = size;
    if (proc != NULL)
    {
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding <FREE_SPACE> entries)
            //if(_processes[i]->variables[j]->type != DataType::FreeSpace)
            {
                std::cout << _processes[i]->pid << " | " << _processes[i]->variables[j]->name << " | " << _processes[i]->variables[j]->virtual_address << " | " << _processes[i]->variables[j]->size << std::endl;
            }
        }
    }
}


uint32_t Mmu::getFreeSpace(uint32_t pid, u_int32_t size, u_int32_t allocatedSpace) 
{
    int i, j, stored;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            //starts at 1 as variable 0 is the total free space
            for (j = 1; j < _processes[i]->variables.size(); j++)
            {
                //check for freespace
                if(_processes[i]->variables[j]->type == DataType::FreeSpace)
                {
                    //check that space is large enough
                    if(_processes[i]->variables[j]->size >= size)
                    {
                        return _processes[i]->variables[j]->virtual_address;
                    }
                } 
                //Not freespace, check if new var will fit in allocated space (assuming)
                else if(_processes[i]->variables[j]->virtual_address + _processes[i]->variables[j]->size + size >= allocatedSpace)
                {
                    
                }
            }
        }
    }
    return -1;
}

void Mmu::modifyFreeSpace(u_int32_t pid, uint32_t size, uint32_t address)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            //adjust total size limit
            _processes[i]->variables[0]->size -= size;

            for (j = 1; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->type == DataType::FreeSpace && _processes[i]->variables[j]->size >= size)
                {
                    _processes[i]->variables[j]->size -= size;
                    _processes[i]->variables[j]->virtual_address += size;
                    break;
                }
            }
        }
    }
}

void Mmu::newFreeSpaceGap(u_int32_t pid, uint32_t size, uint32_t address)
{
    addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, size, address);
}

void Mmu::newFreeSpacePage(u_int32_t pid, uint32_t page_size, uint32_t page_address)
{
    addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, page_size, page_address);
}