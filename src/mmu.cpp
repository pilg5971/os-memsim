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

void Mmu::removeVariableFromProcess(uint32_t pid, uint32_t address)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->virtual_address == address)
                {
                    _processes[i]->variables.erase(_processes[i]->variables.begin()+j);
                }
            }
        }
    }
}

void Mmu::print()
{
    int i, j, PID;
    std::string varName;
    uint32_t virAddr, varSize;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            PID = _processes[i]->pid;
            varName = _processes[i]->variables[j]->name;
            virAddr = _processes[i]->variables[j]->virtual_address;
            varSize = _processes[i]->variables[j]->size;

            //if(_processes[i]->variables[j]->type != DataType::FreeSpace)
            {
                //printf(" %4d | %-13s |  0x%08X  | %10u \n", PID, varName.c_str(), virAddr, varSize);
                printf(" %4d | %-13s |  %u  | %10u \n", PID, varName.c_str(), virAddr, varSize);
            }
        }
    }
}

void Mmu::removeProcess(uint32_t pid)
{
    int i, j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                removeVariableFromProcess(pid, _processes[i]->variables[j]->virtual_address);
                j--;//Super sketchy, but it doesn't seem to create an infinite loop
            }
        }
    }
}


uint32_t Mmu::getFreeSpace(uint32_t pid, uint32_t size, u_int32_t allocatedSpace) 
{
    int i, j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            //starts at 1 as variable 0 is the main free space
            for (j = 1; j < _processes[i]->variables.size(); j++)
            {
                //check for freespace
                if(_processes[i]->variables[j]->type == DataType::FreeSpace && size <= _processes[i]->variables[j]->size)
                {
                    return _processes[i]->variables[j]->virtual_address;
                } 
            }
            //return main free space address if no smaller free space fits
            if(size <= _processes[i]->variables[0]->size)
            {   
                return _processes[i]->variables[0]->virtual_address;
            } else {
                std::cout << "Allocation would exceed system memory" << std::endl;
                return -1;
            }
        }
    }
    //pid not found
    return -1;
}

void Mmu::modifyFreeSpace(u_int32_t pid, uint32_t size, uint32_t address, uint32_t offset)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->type == DataType::FreeSpace)
                {
                    if(_processes[i]->variables[j]->virtual_address == address)
                    {
                        Variable *free = _processes[i]->variables[j];

                        //if the size of the free space was exactly equal to the size of the new variable, then it should be removed (no more free space)
                        if(free->size == size)
                        {
                            removeVariableFromProcess(pid, address);
                        }
                        //If the size of the free space was larger than the size of the new variable, then the free space should be modified such that its
                        //virtual address is N bytes bigger and its size should be N bytes less (where N is the size of the new variable)
                        if(free->size > size)
                        {
                            free->virtual_address += size + offset;
                            free->size -= size + offset;
                        }
                        //if offset was needed, create free space at address with size of offset
                        if(offset > 0)
                        {
                            addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, offset, address);
                        }
                        return;
                    }
                }
            }
        }
    }
}

void Mmu::restoreFreeSpace(uint32_t pid, uint32_t address, uint32_t size)
{
    int i,j, from, to;

    from = address;
    to = address + size;

    //modify freespaces
    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 1; j < _processes[i]->variables.size(); j++)
            {
                Variable *free = _processes[i]->variables[j];
                //if this is a freespace before or after variable, remove and change to/from
                if(free->type == DataType::FreeSpace)
                {
                    //if freespace comes after variable
                    if(free->virtual_address == to)
                    {
                        to = free->virtual_address + free->size;
                        removeVariableFromProcess(pid, free->virtual_address);
                        j--;
                    }

                    //if freespace comes before variable
                    if((free->virtual_address + free->size) == from)
                    {
                        from = free->virtual_address;
                        removeVariableFromProcess(pid, free->virtual_address);
                        j--;
                    }
                }
            }
            //if new free space is adjecent to the main free space, edit main free space
            Variable *main_free = _processes[i]->variables[0];
            if(main_free->virtual_address == to)
            {
                main_free->virtual_address = from;
                main_free->size += to - from;
                return;
            }
        }
    }
    addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, to - from, from);
}

uint32_t Mmu::isEmptyPage(uint32_t pid, uint32_t from, uint32_t to)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->virtual_address >= from && _processes[i]->variables[j]->virtual_address < to)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

uint32_t Mmu::getAddress(uint32_t pid, std::string var_name)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->name == var_name)
                {
                    return _processes[i]->variables[j]->virtual_address;
                }
            }
        }
    }
    return -1;
}

uint32_t Mmu::getSize(uint32_t pid, std::string var_name)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->name == var_name)
                {
                    return _processes[i]->variables[j]->size;
                }
            }
        }
    }
    return -1;
}

bool Mmu::validProcess(uint32_t pid)
{
    int i;
    bool solution = false;
    for(i = 0; i < _processes.size(); i++)
    {
        if(_processes[i]->pid == pid)
        {
            solution = true;
        }
    }
    return solution;
}

bool Mmu::validVar(uint32_t pid, std::string var_name)
{
    int i,j;
    bool solution = false;
    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->name == var_name)
                {
                    solution = true;
                }
            }
        }
    }
    return solution;
}

DataType Mmu::returnDatatype(uint32_t pid, std::string var_name)
{
    int i,j;
    DataType solution;
    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->name == var_name)
                {
                    solution = _processes[i]->variables[j]->type;
                }
            }
        }
    }
    return solution;
} 

std::vector<Process*> Mmu::getProcesses()
{
    return _processes;
}