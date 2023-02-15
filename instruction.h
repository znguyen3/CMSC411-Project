#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <array>
#include <algorithm>
#include <thread>
#include <mutex>
using namespace std;

enum State {inst_fetch = 1, inst_decode = 2, exec1 = 3, exec2 = 4, exec3 = 5, mem1 = 6, mem2 = 7, mem3 = 8, wr_back = 9};

class instruction {
    public:
        instruction();
        instruction(string, string, string, string, string);
        instruction(const instruction &);
        instruction& operator=(const instruction &);
        bool StageCompleted(State);
        void setStageCompletedAtCycle(State, int);
        void moveToState(State);
        void setAsDone(){m_done = true;}
        void start();
        bool empty();        
        string getLabel(){return m_label;}
        string getOperation(){return m_operation;}
        string getLocation1(){return m_location1;}
        string getLocation2(){return m_location2;}
        string getLocation3(){return m_location3;}
        State getState(){return m_state;}

        
        bool notStarted(){return m_started;}

        int getIFcycle(){return IFcycle;}
        int getIDcycle(){return IDcycle;}
        int getEX1cycle(){return EX1cycle;}
        int getEX2cycle(){return EX2cycle;}
        int getEX3cycle(){return EX3cycle;}
        int getMEMcycle(){return MEMcycle;}
        int getWBcycle(){return WBcycle;}
        bool running(){return m_started;}
        bool getDone(){return m_done;}
        
    private:
        string m_label;
        string m_operation;
        string m_location1;
        string m_location2;
        string m_location3;
        State m_state;
        int IFcycle = 0;
        int IDcycle = 0;
        int EX1cycle = 0;
        int EX2cycle = 0;
        int EX3cycle = 0;
        int MEMcycle = 0;
        int WBcycle = 0;
        bool m_done = false;
        bool m_started = false;
};