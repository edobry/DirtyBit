#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <unordered_map>
#include <queue>

using namespace std;

//Utilities
struct Options{
	string referenceFile;
	int missPenalty, dirtyPagePenalty, pageSize, VAbits, PAbits;
	bool Debug;

	void SetOpt(string name, string val) {
		if(name == "referenceFile"){
			referenceFile = val;
		} else if(name == "missPenalty"){
			missPenalty = strtol(val.c_str());
		} else if(name == "dirtyPagePenalty"){
			dirtyPagePenalty = strtol(val.c_str());
		} else if(name == "pageSize"){
			pageSize = strtol(val.c_str());
		} else if(name == "VAbits"){
			VAbits = strtol(val.c_str());
		} else if(name == "PAbits"){
			PAbits = strtol(val.c_str());
		} else if(name == "Debug"){
			Debug = val == "true";
		} else 
	};

	Options() {}
};
Options DirtyBitOptions;

//Initialization
void readOpts(){
	DirtyBitOptions = Options();
	ifstream scheduling("MemoryManagement.txt");

	if (scheduling.is_open())
	{
		string line;
		while(scheduling.good())
		{
			getline(scheduling, line);
			int splitPos = line.find('=');
			string optName = line.substr(0, splitPos);
			string val = line.substr(splitPos+1);

			DirtyBitOptions.SetOpt(optName, val);
			cout << optName << ": " << val << endl;
		}
		cout << "==============================================================" << endl << endl;
		scheduling.close();
	}
}

struct ProcessNode{
	Process* value;
	ProcessNode* next;

	ProcessNode(Process* value, ProcessNode* next) : value(value), next(next) {}
};

class ProcessQueue{
public:
	ProcessQueue(){};

	void AddProcess(Process* process) {
		ProcessNode* node = new ProcessNode(process, NULL);

		if(head == NULL){
			head = node;
			node->next = node;
		}
		else{

		}
	}

private:
	ProcessNode* head;
	ProcessNode* tail;
};

void readProcesses(ProcessQueue& processes){
	ifstream processFile(DirtyBitOptions.referenceFile);

	if (processFile.is_open())
	{
		string line;
		while(processFile.good())
		{
			getline(processFile, line);
			stringstream ss(line);
			int id, aT, tC, aB;
			ss >> id >> aT >> tC >> aB;
			processes.Enqueue(new Process(id, aT, tC, aB));
		}
		processFile.close();

		cout << "Processes queued: " << processes.Count() << endl;
	}
}


struct Process {
public:
	int ID;
	int totalCPU;
	int avgBurst;

	Process(int id, int aT, int tC, int aB)
		: ID(id), totalCPU(tC), avgBurst(aB), CPUelapsed(0) {}

private: 	
	int CPUelapsed;
};

class PageTable{
public:

private:

};

struct PageTableEntry{
	int frameNum;
	int validBits;
	int referencedBits;
	int dirtyBits;
};

int main(int argc, char* argv[])
{

}