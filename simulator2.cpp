#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <array>
#include <algorithm>
#include "instruction.cpp"
#include <thread>
using namespace std;

//Input file name
const string instructionFileName = "inst.txt";
const string dataSegmentFileName = "data_segment.txt";

//Output file name
const string outputFileName = "output2.txt";

void readInstructionFile(string , instruction *);
bool oprcmp(string , string);
void writeFile(instruction instructions[], int cache_miss, int data_access);

int main(){
    instruction instructions[20];
    State nextState[] = {inst_decode, exec1, exec2, exec3, mem, wr_back};
    
    mutex StageMutex[7];
    bool done_starting_instrcutions = false;
    readInstructionFile(instructionFileName, instructions);

    for(int cycle = 0; cycle < 100; cycle++){
        for(int i = 0; i < 20; i++){
            if(!instructions[i].running()){
                instruction *curr_instr = &instructions[i];
                State curr_state = curr_instr->getState();
                if(curr_state == inst_fetch, inst_decode, exec1, exec2, exec3, mem){
                    if(curr_instr->StageCompleted(curr_state)){
                        curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                    }
                    //below if statment make seg fault because of Mutex
                    if(StageMutex[nextState[curr_state]].try_lock()){
                        StageMutex[curr_state].unlock();
                        curr_instr->moveToState(nextState[curr_state]);
                    }
                } else {
                    curr_instr->setStageCompletedAtCycle(curr_state, cycle);
                    StageMutex[curr_state].unlock();
                    curr_instr->setAsDone();
                }
            }
        }
        
        int instr_ptr = 0;
        while(!done_starting_instrcutions){
            if(!instructions[instr_ptr].notStarted()){
                if(StageMutex[0].try_lock()){
                    instructions[instr_ptr].start();
                } else {
                    done_starting_instrcutions = true;
                }
            }
            if(instr_ptr = 19){
                done_starting_instrcutions = true;
            } else {
                instr_ptr++;
            }
        }
    }
    
    writeFile(instructions, 0, 0);
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
    if(c.compare(d) == 0)
    {
        return true;
    }
    else {   
        return false;
    }    
}

string eraseWhiteSpace(string str){
    str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
    return str;
}

void readInstructionFile(string fileName, instruction instructions[]){
    ifstream input(fileName);
    string temp_label;
    string temp_operation;
    string temp_location1;
    string temp_location2;
    string temp_location3;
    if(instructions[0].empty()){
        if(input.is_open()){
            int counter = 0;
            while (getline(input, temp_label, '|') &&
                   getline(input, temp_operation, '|'))
            {
                if(oprcmp(temp_operation, "LI") || oprcmp(temp_operation, "LW") || oprcmp(temp_operation, "SW")){
                    getline(input, temp_location1, ',');
                    getline(input, temp_location2, '\n');
                    temp_location3.clear();
                } else if(oprcmp(temp_operation, "HLT")){
                    temp_location1.clear();
                    temp_location2.clear();
                    temp_location3.clear();
                } else {
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

void writeFile(instruction instructions[], int cache_miss, int data_access){
    
    ofstream output(outputFileName);
    if (output.is_open())
    {
        //Table Format
        output << left << setw(30) << "Cycle Number for Each Stage"
               << left << setw(6) << "IF"
               << left << setw(6) << "ID"
               << left << setw(6) << "EX3"
               << left << setw(6) << "MEM"
               << left << setw(6) << "WB" << endl << endl;
        //Instructions        
        for(int i = 0; i < 20; i++)
        {
            cout << instructions[i].getLocation2() << endl;
            if(!instructions[i].empty()){
                output << setw(6) << instructions[i].getLabel() << "|";
                if(oprcmp(instructions[i].getOperation(), "HLT"))
                {
                    output << setw(24) << instructions[i].getOperation() + "|";
                    output << setw(6) << instructions[i].getIFcycle();
                    output << setw(6) << instructions[i].getIDcycle();
                    output << endl;
                } else {
                    if(oprcmp(instructions[i].getOperation(), "LI") || oprcmp(instructions[i].getOperation(), "LW") || oprcmp(instructions[i].getOperation(), "SW")){
                        output << setw(24) << instructions[i].getOperation() + "|" + instructions[i].getLocation1() + ", " + eraseWhiteSpace(instructions[i].getLocation2());
                        output << setw(6) << instructions[i].getIFcycle();
                        output << setw(6) << instructions[i].getIDcycle();
                        output << setw(6) << instructions[i].getEX3cycle();
                        output << setw(6) << instructions[i].getMEMcycle();
                        output << setw(6) << instructions[i].getWBcycle();
                        output << endl;
                    } else if(oprcmp(instructions[i].getOperation(), "BNE") || oprcmp(instructions[i].getOperation(), "BEQ") || oprcmp(instructions[i].getOperation(), "J")){
                        output << setw(24) << instructions[i].getOperation() + "|" + instructions[i].getLocation1() + ", " + instructions[i].getLocation2() + ", " + eraseWhiteSpace(instructions[i].getLocation3());
                        output << setw(6) << instructions[i].getIFcycle();
                        output << setw(6) << instructions[i].getIDcycle();
                        output << setw(6) << instructions[i].getEX1cycle();
                        output << endl;
                    } else {
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
        int request_instruction = 10;//size of array
        int hit_instruction = request_instruction-cache_miss; //unique instructions in cache
        output << "\nTotal number of access request for instruction cache: " << request_instruction << endl;
        output << "Number of instruction cache hits: " << hit_instruction << endl;
        output << endl;
    }

    output.close();
}
