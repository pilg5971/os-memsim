#include <iostream>
#include <string>
#include <cstring>
#include "mmu.h"
#include "pagetable.h"

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);

//Command Conversion Methods
void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);
DataType stringToDataType(std::string text);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory'
    uint32_t mem_size = 67108864;
    void *memory = malloc(mem_size); // 64 MB (64 * 1024 * 1024)

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(mem_size);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::vector<std::string> command_list;
    std::string lead_command;
    std::cout << "> ";
    std::getline (std::cin, command);
    while (command != "exit") {
        // Handle command
        // TODO: implement this!

        //command = current command
        //command_list = Command + Arguments in Array format.... I.E. command_list[0] = create [1] = 1024 ... etc
        splitString(command, ' ', command_list);
        lead_command = command_list[0];
        /*
        std::cout << "command_list[0]: " << command_list[0] << std::endl;
        std::cout << "command_list[1]: " << command_list[1] << std::endl;
        std::cout << "command_list[2]: " << command_list[2] << std::endl;
        std::cout << "lead_command: " << lead_command << std::endl;
        */

        //Valid Command Check
        if(lead_command == "create"){
            //std::cout << "Success! --> create" << std::endl;

            //create process
            int textSize = stoi(command_list[1]);
            int dataSize = stoi(command_list[2]); 
            createProcess(textSize, dataSize, mmu, page_table);
        }
        else if(lead_command == "allocate"){
            //std::cout << "Success! --> allocate" << std::endl;
            int pid = stoi(command_list[1]);
            std::string varName = command_list[2];
            DataType dataType = stringToDataType(command_list[3]);
            int numEl = stoi(command_list[4]);
            allocateVariable(pid, varName, dataType, numEl, mmu, page_table);
        }
        else if(lead_command == "set"){
            //std::cout << "Success! --> set" << std::endl;

        }   
        else if(lead_command == "free"){
            //std::cout << "Success! --> free" << std::endl;
            int pid = stoi(command_list[1]);
            std::string varName = command_list[2];
            freeVariable(pid, varName, mmu, page_table);
        }
        else if(lead_command == "terminate"){
            //std::cout << "Success! --> terminate" << std::endl;

        }
        else if(lead_command == "print"){
            //std::cout << "Success! --> print" << std::endl;
            std::string whatToPrint = command_list[1];
            if(whatToPrint == "mmu"){
                mmu->print();
            }
            else if(whatToPrint == "page"){
                page_table->print();
            }
            else if(whatToPrint == "processes"){
                page_table->printProcesses();
            }
            
            //Variable printing
            /*TBD
            else{

            }
            */
        }

        //Invalid Command
        else{
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
    }

    // Cean up
    free(memory);
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    //[1]: create new process in the MMU
    uint32_t PID = mmu->createProcess();

    //[2]: allocate new variables for <TEXT>, <GLOBALS>, and <STACK>
    //<STACK> = 65536
    int tot_size = text_size + data_size + 65536;
    int page_size = page_table->getPageSize();
    int space = 0;
    int i = 0;

    mmu->addVariableToProcess(PID, "<TEXT>", Char, text_size, 0);
    mmu->addVariableToProcess(PID, "<GLOBALS>", Char, data_size, text_size);
    mmu->addVariableToProcess(PID, "<STACK>", Char, 65536, text_size + data_size);

    while(space < tot_size){
        page_table->addEntry(PID, i);
        i++;
        space += page_size;
    }

    //initialize free space
    if(space == tot_size){
        mmu->newFreeSpacePage(PID, page_size, space);
    } else {
        mmu->newFreeSpaceGap(PID, space - tot_size, space - (space - tot_size));
    }
    //[3]: print PID
    std::cout << PID << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{

    //size of data
    uint32_t n;
    if(type == Char){
        n = num_elements;
    }
    else if(type == Short){
        n = num_elements * 2;
    }
    else if(type == Int || type == Float){
        n = num_elements * 4;
    }
    else if(type == Long || type == Double){
        n = num_elements * 8;
    }

    //error check if allocation will exceed system memory
    if(!mmu->modifyTotalSpace(pid, n))
    {
        return;
    }

    //next unallocated page
    int page = page_table->getNextPage(pid);
    int page_size = page_table->getPageSize();

    //[1]: find first free space within a page already allocated to this process that is large enough to fit the new variable

    //Location of the first free space big enough for data 'n' (-1 if no free space)
    uint32_t address = mmu->getFreeSpace(pid, n, page_size * page);
    
    //[2]: if no hole is large enough, allocate new page(s)
    if(address == -1)
    {
        page_table->addEntry(pid, page);
        address = page_size * page;
        mmu->newFreeSpacePage(pid, page_size, address);
    }

    //[3]: insert variable into MMU
    mmu->addVariableToProcess(pid,var_name,type,n,address); 
    //modify free space
    mmu->modifyFreeSpace(pid, n, address);

    //[4]: print virtual memory address
    std::cout << address << std::endl;
}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    //get size and address
    uint32_t address = mmu->getAddress(pid, var_name);
    uint32_t size = mmu->getSize(pid, var_name);
    uint32_t page_size = page_table->getPageSize();

    //[1]: remove entry from MMU
    mmu->removeVariableFromProcess(pid, address);

    //edit freespace
    mmu->restoreFreeSpace(pid, address, size, page_size);

    //[2]: free page if this variable was the only one on a given page

    //returns the frame if it only contains var_name, otherwise returns -1
    int frame = mmu->emptyPage(pid, page_size);

    if(frame != -1)
    {
        page_table->removeEntry(pid, frame);
    }

}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    //   - free all pages associated with given process
}

//--------------------------------------------------STRING METHODS--------------------------------------------------//
//                                               (From Assignment #2)

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}

DataType stringToDataType(std::string text){
    DataType solution;
    if(text == "Int" || text == "int"){
        solution = Int;
    }
    else if(text == "Char" || text == "char"){
        solution = Char;
    }
    else if(text == "Short" || text == "short"){
        solution = Short;
    }
    else if(text == "Float" || text == "float"){
        solution = Float;
    }
    else if(text == "Long" || text == "long"){
        solution = Long;
    }
    else if(text == "Double" || text == "double"){
        solution = Double;
    }
    else{
        std::cout << "error: allocate command parameter not recognized" << std::endl;
    }
    return solution;
}