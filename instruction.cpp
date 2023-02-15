#include "instruction.h"

//Null Constructor
instruction::instruction(){
    m_label = "";
    m_operation = "";
    m_location1 = "";
    m_location2 = "";
    m_location3 = "";
}
//copy constructor
instruction::instruction(const instruction &rhs){
    m_label = rhs.m_label;
    m_operation = rhs.m_operation;
    m_location1 = rhs.m_location1;
    m_location2 = rhs.m_location2;
    m_location3 = rhs.m_location3;
    m_state = rhs.m_state;
    m_done = rhs.m_done;
}
//Overload constructor
instruction& instruction::operator=(const instruction &rhs){
    m_label = rhs.m_label;
    m_operation = rhs.m_operation;
    m_location1 = rhs.m_location1;
    m_location2 = rhs.m_location2;
    m_location3 = rhs.m_location3;
    m_state = rhs.m_state;
    m_done = rhs.m_done;
    return *this;
}
//Constructor With values
instruction::instruction(string label, string operation, string location1, string location2, string location3){
    m_label = label;
    m_operation = operation;
    m_location1 = location1;
    m_location2 = location2;
    m_location3 = location3;
}


void instruction::setStageCompletedAtCycle(State state, int cycle){
    switch (state)
    {
    case inst_fetch:
        IFcycle = cycle;
        break;
    case inst_decode:
        IDcycle = cycle;
        break;
    case exec1:
        EX1cycle = cycle;
        break;
    case exec2:
        EX2cycle = cycle;
        break;
    case exec3:
        EX3cycle = cycle;
        break;
    case mem3:
        MEMcycle = cycle;
        break;
    case wr_back:
        WBcycle = cycle;
        break;
    default:
        break;
    }
}

void instruction::moveToState(State state){
    m_state = state;
}
void instruction::start(){
    m_state = inst_fetch;
    m_started = true;
}
bool instruction::StageCompleted(State state){
    bool completed = false;
    switch (state)
    {
    case inst_fetch:
        if(IFcycle != 0){
            completed = true;
        }
        break;
    case inst_decode:
        if(IDcycle != 0){
            completed = true;
        }
        break;
    case exec1:
        if(EX1cycle != 0){
            completed = true;
        }
        break;
    case exec2:
        if(EX2cycle != 0){
            completed = true;
        }
        break;
    case exec3:
        if(EX3cycle != 0){
            completed = true;
        }
        break;
    case mem3:
        if(MEMcycle != 0){
            completed = true;
        }
        break;
    case wr_back:
        if(WBcycle != 0){
            completed = true;
        }
        break;
    default:
        break;
    }
    return completed;
}

bool instruction::empty(){
    if(m_operation == ""){
        return true;
    } else {
        return false;
    }
}
