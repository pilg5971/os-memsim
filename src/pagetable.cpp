#include "pagetable.h"

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    std::vector<std::string> keys = sortedKeys();
    int frame, i;
    bool frames[keys.size()];

    // Find free frame
    //Create collection of used frame numbers
    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        frames[it->second] = true;
    }

    //frame gets the first unused frame number or a new frame that is higher than the rest
    frame = keys.size();
    for(i = 0; i < keys.size(); i++) 
    {
        if(!frames[i]){
            frame = i;
            break;
        }
    }

    _table[entry] = frame;
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset

    // TODO: implement this!
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        address = _table.at(entry) * _page_size + page_offset;
    }

    return address;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();
    std::string separator = "|";

    for (i = 0; i < keys.size(); i++)
    {
        size_t sepPosition = keys[i].find(separator);
        int PID = stoi(keys[i].substr(0, sepPosition));
        int pageNum = stoi(keys[i].substr(sepPosition + 1));
        //std::cout << " " << PID << " |\t\t  " << pageNum << " |\t\t " << _table[keys[i]] << std::endl;
        printf(" %4d | %11d | %12d \n", PID, pageNum, _table[keys[i]]);
    }
}

int PageTable::getNextPage(uint32_t pid)
{
    int i = 0;
    std::string entry = std::to_string(pid) + "|" + std::to_string(i);

    while(_table.count(entry) > 0){
        i++;
        entry = std::to_string(pid) + "|" + std::to_string(i);
    }

    return i;
}

void PageTable::printProcesses()
{
    int PID = 1023;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        size_t separator = it->first.find("|");
        uint32_t pid = std::stoi(it->first.substr(0, separator)); 

        //when a new pid is found
        if(pid != PID)
        {
            //update pid
            PID = pid;
            //print pid
            std::cout << pid << std::endl;
        }
    }
}

void PageTable::removeEntry(uint32_t pid, uint32_t page)
{
    std::string entry = std::to_string(pid) + "|" + std::to_string(page);

    //while entry exists
    _table.erase(entry);
}

int PageTable::getPageSize()
{
    return _page_size;
}
