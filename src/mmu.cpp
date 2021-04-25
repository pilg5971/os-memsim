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

            if(_processes[i]->variables[j]->type != DataType::FreeSpace)
            {
                printf(" %4d | %-13s |  0x%08X  | %10u \n", PID, varName.c_str(), virAddr, varSize);
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

bool Mmu::modifyTotalSpace(u_int32_t pid, uint32_t size)
{
    int i,j;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            //adjust total size limit
            if(_processes[i]->variables[0]->size - size >= 0)
            {
                _processes[i]->variables[0]->size -= size;
                return true;
            } else {
                std::cout << "error: allocation exceeds system memory, variable not allocated" << std::endl;
                return false;
            }
        }
    }
    //only reaches this if pid does not exist
    return true;
}

void Mmu::restoreFreeSpace(uint32_t pid, uint32_t address, uint32_t size, uint32_t page_size)
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
                Variable *var = _processes[i]->variables[j];
                //if this is a freespace before or after var_name, remove and change to/from
                if(var->type == DataType::FreeSpace)
                {
                    //if freespace comes after var_name
                    if(var->virtual_address == to && to%page_size != 0)
                    {
                        to = var->virtual_address + var->size;
                        removeVariableFromProcess(pid, var->virtual_address);
                        j--;
                    }

                    //if freespace comes before var_name
                    if((var->virtual_address + var->size) == from && from%page_size != 0)
                    {
                        from = var->virtual_address;
                        removeVariableFromProcess(pid, var->virtual_address);
                        j--;
                    }
                }
            }
        }
    }
    addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, to - from, from);
}

uint32_t Mmu::emptyPage(uint32_t pid, uint32_t page_size)
{
    int i,j,frame;

    for (i = 0; i < _processes.size(); i++)
    {
        //correct process
        if(_processes[i]->pid == pid)
        {
            for (j = 0; j < _processes[i]->variables.size(); j++)
            {
                if(_processes[i]->variables[j]->type == DataType::FreeSpace && _processes[i]->variables[j]->size == page_size)
                {
                    frame = _processes[i]->variables[j]->virtual_address/page_size;
                    removeVariableFromProcess(pid, _processes[i]->variables[j]->virtual_address);
                    return frame;
                }
            }
        }
    }
    return -1;
}

void Mmu::newFreeSpaceGap(u_int32_t pid, uint32_t size, uint32_t address)
{
    addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, size, address);
}

void Mmu::newFreeSpacePage(u_int32_t pid, uint32_t page_size, uint32_t page_address)
{
    addVariableToProcess(pid, "<FREE_SPACE>", DataType::FreeSpace, page_size, page_address);
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