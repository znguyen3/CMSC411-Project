#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <array>
#include <algorithm>
#include "instruction.cpp"
#include <thread>
#include <queue>
#include <bitset>
using namespace std;

//Input file name
const string instructionFileName = "inst.txt";
const string dataSegmentFileName = "data_segment.txt";

//Output file name
const string outputFileName = "output2.txt";

void readInstructionFile(string, instruction *);
bool oprcmp(string, string);
bool regcmp(string, string);
bool statecmp(State a, State b);
string eraseWhiteSpace(string);
void writeFile(instruction instructions[], int cache_miss);
string getEnum(State state)
{
    switch (state)
    {
    case inst_fetch:
        return "IF";
        break;
    case inst_decode:
        return "ID";
        break;
    case exec1:
        return "EX1";
        break;
    case exec2:
        return "EX2";
        break;
    case exec3:
        return "EX3";
        break;
    case mem1:
        return "MEM";
        break;
    case mem2:
        return "MEM";
        break;
    case mem3:
        return "MEM";
        break;
    case wr_back:
        return "WB";
        break;
    default:
        return "No";
        break;
    }
}

int main()
{
    instruction instructions[20];
    State nextState[] = {inst_fetch, inst_decode, exec1, exec2, exec3, mem1, mem2, mem3, wr_back};
    State state = inst_fetch;
    mutex StageMutex[7];
    bool done_starting_instrcutions = false;
    readInstructionFile(instructionFileName, instructions);

    queue<instruction> I_CACHE;
    int cache_miss = 0;
    int counterForCache = 0;
    vector<bitset<32>> D_CACHE;
    bool structural_hazard = false;

    if (I_CACHE.empty())
    {
        for (int i = 0; i < 4; i++)
        {
            I_CACHE.push(instructions[i + counterForCache / 4]);
        }
        cache_miss++;
    }
    int instr_ptr = 0;
    for (int cycle = 0; cycle < 100; cycle++)
    {
        for (int i = 0; i < 20; i++)
        {
            instruction *prev_instr;
            if (i != 0)
            {
                prev_instr = &instructions[i - 1];
            }
            if (!instructions[i].empty())
            {
                if (instructions[i].running() && !instructions[i].getDone())
                {
                    instruction *curr_instr = &instructions[i];
                    State curr_state = curr_instr->getState();

                    cout << cycle << " : " << curr_instr->getOperation() << " | " << getEnum(curr_state) << endl;

                    //filter the opration type so how much it will go process the next
                    if (oprcmp(curr_instr->getOperation(), "HLT"))
                    {
                        if (curr_state == inst_fetch)
                        {
                            if (!curr_instr->StageCompleted(curr_state))
                            {
                                curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            }
                            //below if statment make seg fault because of Mutex
                            if (StageMutex[nextState[curr_state]].try_lock())
                            {
                                StageMutex[curr_state].unlock();
                                curr_instr->moveToState(nextState[curr_state]);
                            }
                        }
                        else
                        {
                            curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            StageMutex[curr_state].unlock();
                            curr_instr->setAsDone();
                        }
                    }
                    else if (oprcmp(curr_instr->getOperation(), "LI") || oprcmp(curr_instr->getOperation(), "LW") || oprcmp(curr_instr->getOperation(), "SW"))
                    {
                        bool data_harzard = false;
                        structural_hazard = false;
                        if (i > 0)
                        {
                            State prev_state = prev_instr->getState();
                            if (curr_state == inst_decode)
                            {
                                if (!prev_instr->getDone())
                                {
                                    if (prev_state == mem1 || prev_state == mem2 || prev_state == mem3 || prev_state == wr_back)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                    if (prev_state == exec1 || prev_state == exec2 || prev_state == exec3)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {                                            
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                }
                            }
                            
                            if (curr_state == inst_fetch)
                            {   
                                if(prev_state == inst_decode)
                                {                                    
                                    structural_hazard = true;
                                    cout << "Structal Hazard" << endl;   
                                }

                            }
                        }
                        //process to next state
                        if (curr_state == inst_fetch || curr_state == inst_decode || curr_state == exec1 || curr_state == exec2 || curr_state == exec3 || curr_state == mem1 || curr_state == mem2 || curr_state == mem3)
                        {
                            if (!curr_instr->StageCompleted(curr_state))
                            {
                                curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            }
                            //below if statment make seg fault because of Mutex
                            if (StageMutex[nextState[curr_state]].try_lock())
                            {
                                if (!data_harzard && !structural_hazard)
                                {
                                    StageMutex[curr_state].unlock();
                                    curr_instr->moveToState(nextState[curr_state]);
                                }
                            }
                        }
                        else
                        {
                            curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            StageMutex[curr_state].unlock();
                            curr_instr->setAsDone();
                        }
                    }
                    else if (oprcmp(curr_instr->getOperation(), "BNE") || oprcmp(curr_instr->getOperation(), "BNQ"))
                    {
                        bool data_harzard = false;
                        if (i > 0)
                        {
                            State prev_state = prev_instr->getState();
                            if (curr_state == inst_decode)
                            {
                                if (!prev_instr->getDone())
                                {
                                    if (prev_state == mem1 || prev_state == mem2 || prev_state == mem3 || prev_state == wr_back)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                    if (prev_state == exec1 || prev_state == exec2 || prev_state == exec3)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {                                            
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                }
                            }
                            
                            if (curr_state == inst_fetch)
                            {
                                if(prev_state == inst_decode)
                                {
                                    structural_hazard = true;
                                    cout << "Structal Hazard" << endl;
                                }

                            }
                        }

                        //process to next state
                        if (curr_state == inst_fetch || curr_state == inst_decode || curr_state == exec1)
                        {
                            if (!curr_instr->StageCompleted(curr_state))
                            {
                                curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            }
                            //below if statment make seg fault because of Mutex
                            if (StageMutex[nextState[curr_state]].try_lock())
                            {
                                StageMutex[curr_state].unlock();
                                curr_instr->moveToState(nextState[curr_state]);
                            }
                        }
                        else
                        {
                            curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            StageMutex[curr_state].unlock();
                            curr_instr->setAsDone();
                        }
                    }
                    else if (oprcmp(curr_instr->getOperation(), "J"))
                    {
                        bool data_harzard = false;
                        if (i > 0)
                        {
                            State prev_state = prev_instr->getState();
                            if (curr_state == inst_decode)
                            {
                                if (!prev_instr->getDone())
                                {
                                    if (prev_state == mem1 || prev_state == mem2 || prev_state == mem3 || prev_state == wr_back)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                    if (prev_state == exec1 || prev_state == exec2 || prev_state == exec3)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {                                            
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                }
                            }
                            
                            if (curr_state == inst_fetch)
                            {
                                if(prev_state == inst_decode)
                                {
                                    structural_hazard = true;
                                    cout << "Structal Hazard" << endl;
                                }

                            }
                        }
                        //process to next state
                        if (curr_state == inst_fetch || curr_state == inst_decode || curr_state == exec1 || curr_state == exec2 || curr_state == exec3 || curr_state == mem1 || curr_state == mem2 || curr_state == mem3)
                        {
                            if (!curr_instr->StageCompleted(curr_state))
                            {
                                curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            }
                            //below if statment make seg fault because of Mutex
                            if (StageMutex[nextState[curr_state]].try_lock())
                            {
                                StageMutex[curr_state].unlock();
                                curr_instr->moveToState(nextState[curr_state]);
                            }
                        }
                        else
                        {
                            curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            StageMutex[curr_state].unlock();
                            curr_instr->setAsDone();
                        }
                    }
                    else
                    {
                        bool data_harzard = false;
                        if (i > 0)
                        {
                            State prev_state = prev_instr->getState();
                            if (curr_state == inst_decode)
                            {
                                if (!prev_instr->getDone())
                                {
                                    if (prev_state == mem1 || prev_state == mem2 || prev_state == mem3 || prev_state == wr_back)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                    if (prev_state == exec1 || prev_state == exec2 || prev_state == exec3)
                                    {
                                        if (regcmp(curr_instr->getLocation1(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation2(), prev_instr->getLocation1()) || regcmp(curr_instr->getLocation3(), prev_instr->getLocation1()))
                                        {                                            
                                            data_harzard = true;
                                            cout << "Data Hazard" << endl;
                                        }
                                    }
                                }
                            }
                            
                            if (curr_state == inst_fetch)
                            {
                                if(prev_state == inst_decode)
                                {
                                    structural_hazard = true;
                                    cout << "Structal Hazard" << endl;
                                }

                            }
                        }
                        //process to next state
                        if (curr_state == inst_fetch || curr_state == inst_decode || curr_state == exec1 || curr_state == exec2 || curr_state == exec3 || curr_state == mem1 || curr_state == mem2 || curr_state == mem3)
                        {
                            if (!curr_instr->StageCompleted(curr_state))
                            {
                                curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            }
                            //below if statment make seg fault because of Mutex
                            if (StageMutex[nextState[curr_state]].try_lock())
                            {
                                StageMutex[curr_state].unlock();
                                curr_instr->moveToState(nextState[curr_state]);
                            }
                        }
                        else
                        {
                            curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                            StageMutex[curr_state].unlock();
                            curr_instr->setAsDone();
                        }
                    }
                }
            }
        }

        //I_chche miss handle
        if (I_CACHE.empty())
        {
            for (int i = 0; i < 4; i++)
            {
                I_CACHE.push(instructions[i + counterForCache / 4]);
            }
            cache_miss++;
        }
        if (!done_starting_instrcutions && !structural_hazard)
        {
            if (!instructions[instr_ptr].notStarted())
            {
                if (!instructions[instr_ptr].empty())
                {
                    if (StageMutex[0].try_lock())
                    {
                        cout << cycle << " : " << instructions[instr_ptr].getOperation() << " Start" << endl;
                        instructions[instr_ptr].start();
                        counterForCache++;
                        I_CACHE.pop();
                    }
                    else
                    {
                        done_starting_instrcutions = true;
                    }
                }
            }
            if (instr_ptr == 19)
            {
                done_starting_instrcutions = true;
            } else {
                instr_ptr++;
            }
        }
    }

    writeFile(instructions, cache_miss);
    return 0;
}

bool oprcmp(string a, string b)
{
    string c = a;
    string d = b;
    //make lower case
    transform(c.begin(), c.end(), c.begin(), ::tolower);
    transform(d.begin(), d.end(), d.begin(), ::tolower);
    //erase the white spaces to do something.
    c.erase(remove_if(c.begin(), c.end(), ::isspace), c.end());
    d.erase(remove_if(d.begin(), d.end(), ::isspace), d.end());
    if (c.compare(d) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

string eraseWhiteSpace(string str)
{
    str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
    return str;
}

void readInstructionFile(string fileName, instruction instructions[])
{
    ifstream input(fileName);
    string temp_label;
    string temp_operation;
    string temp_location1;
    string temp_location2;
    string temp_location3;
    if (instructions[0].empty())
    {
        if (input.is_open())
        {
            int counter = 0;
            while (getline(input, temp_label, '|') &&
                   getline(input, temp_operation, '|'))
            {
                if (oprcmp(temp_operation, "LI") || oprcmp(temp_operation, "LW") || oprcmp(temp_operation, "SW"))
                {
                    getline(input, temp_location1, ',');
                    getline(input, temp_location2, '\n');
                    temp_location3.clear();
                }
                else if (oprcmp(temp_operation, "HLT"))
                {
                    temp_location1.clear();
                    temp_location2.clear();
                    temp_location3.clear();
                }
                else
                {
                    getline(input, temp_location1, ',');
                    getline(input, temp_location2, ',');
                    getline(input, temp_location3, '\n');
                }
                instructions[counter] = instruction(temp_label, temp_operation, temp_location1, temp_location2, temp_location3);
                counter++;
            }
        }
    }
}

void writeFile(instruction instructions[], int cache_miss)
{

    ofstream output(outputFileName);
    if (output.is_open())
    {
        //Table Format
        output << left << setw(30) << "Cycle Number for Each Stage"
               << left << setw(6) << "IF"
               << left << setw(6) << "ID"
               << left << setw(6) << "EX3"
               << left << setw(6) << "MEM"
               << left << setw(6) << "WB" << endl
               << endl;
        //Instructions
        for (int i = 0; i < 20; i++)
        {
            if (!instructions[i].empty())
            {
                output << setw(6) << instructions[i].getLabel() << "|";
                if (oprcmp(instructions[i].getOperation(), "HLT"))
                {
                    output << setw(24) << instructions[i].getOperation() + "|";
                    output << setw(6) << instructions[i].getIFcycle();
                    output << setw(6) << instructions[i].getIDcycle();
                    output << endl;
                }
                else
                {
                    if (oprcmp(instructions[i].getOperation(), "LI") || oprcmp(instructions[i].getOperation(), "LW") || oprcmp(instructions[i].getOperation(), "SW"))
                    {
                        output << setw(24) << instructions[i].getOperation() + "|" + instructions[i].getLocation1() + ", " + eraseWhiteSpace(instructions[i].getLocation2());
                        output << setw(6) << instructions[i].getIFcycle();
                        output << setw(6) << instructions[i].getIDcycle();
                        output << setw(6) << instructions[i].getEX3cycle();
                        output << setw(6) << instructions[i].getMEMcycle();
                        output << setw(6) << instructions[i].getWBcycle();
                        output << endl;
                    }
                    else if (oprcmp(instructions[i].getOperation(), "BNE") || oprcmp(instructions[i].getOperation(), "BEQ") || oprcmp(instructions[i].getOperation(), "J"))
                    {
                        output << setw(24) << instructions[i].getOperation() + "|" + instructions[i].getLocation1() + ", " + instructions[i].getLocation2() + ", " + eraseWhiteSpace(instructions[i].getLocation3());
                        output << setw(6) << instructions[i].getIFcycle();
                        output << setw(6) << instructions[i].getIDcycle();
                        output << setw(6) << instructions[i].getEX1cycle();
                        output << endl;
                    }
                    else
                    {
                        output << setw(24) << instructions[i].getOperation() + "|" + instructions[i].getLocation1() + ", " + instructions[i].getLocation2() + ", " + eraseWhiteSpace(instructions[i].getLocation3());
                        output << setw(6) << instructions[i].getIFcycle();
                        output << setw(6) << instructions[i].getIDcycle();
                        output << setw(6) << instructions[i].getEX3cycle();
                        output << setw(6) << instructions[i].getMEMcycle();
                        output << setw(6) << instructions[i].getWBcycle();
                        output << endl;
                    }
                }
            }
        }
        //Summary
        int request_instruction = 10;                           //size of array
        int hit_instruction = request_instruction - cache_miss; //unique instructions in cache
        output << "\nTotal number of access request for instruction cache: " << request_instruction << endl;
        output << "Number of instruction cache hits: " << hit_instruction << endl;
        output << endl;
    }

    output.close();
}

bool regcmp(string a, string b)
{
    string c = eraseWhiteSpace(a);
    string d = eraseWhiteSpace(d);
    transform(c.begin(), c.end(), c.begin(), ::toupper);
    transform(d.begin(), d.end(), d.begin(), ::toupper);
    if (c.find(d) >= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool statecmp(State a, State b)
{
    if (a == b)
    {
        return true;
    }
    else
    {
        return false;
    }
}