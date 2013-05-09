#include <iostream>
#include <istream>
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
			missPenalty = atoi(val.c_str());
		} else if(name == "dirtyPagePenalty"){
			dirtyPagePenalty = atoi(val.c_str());
		} else if(name == "pageSize"){
			pageSize = atoi(val.c_str());
		} else if(name == "VAbits"){
			VAbits = atoi(val.c_str());
		} else if(name == "PAbits"){
			PAbits = atoi(val.c_str());
		} else if(name == "Debug"){
			Debug = val == "true";
		}
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

			if(optName.length() > 0 && val.length() > 0){
				DirtyBitOptions.SetOpt(optName, val);
				cout << optName << ": " << val << endl;
			}
		}
		cout << "==============================================================" << endl << endl;
		scheduling.close();
	}
}

enum Action{ Read, Write };

struct Reference{
	int addr;
	Action action;

	Reference(int addr, Action action) : addr(addr), action(action) {}
};

struct Process {
public:
	int ID;

	void AddReference(int id, Action action){
		references.push(new Reference(id, action));
	}

	Reference* GetNextReference(){
		Reference* ref = references.front();
		references.pop();
		return ref;
	}

	Process(int id) : ID(id) { references = queue<Reference*>(); }
private:
	queue<Reference*> references;
};

struct ProcessNode{
	Process* value;
	ProcessNode* next;

	ProcessNode(Process* value, ProcessNode* next) : value(value), next(next) {}
};

class ProcessQueue{
public:
	void AddProcess(Process* process) {
		ProcessNode* node = new ProcessNode(process, NULL);

		//Queue is empty
		if(head == NULL){
			head = node;
			node->next = node;
			curr = head;
		}
		else{
			//Queue has one element
			if(tail == NULL){
				head->next = node;
				tail = node;
				tail->next = head;
			}
			else{
				//Insert before current
				ProcessNode* before = head;
				while(before->next != curr)
					before = before->next;

				before->next = node;
				node->next = curr;
			}
		}
		count++;
	}

	int Count() { return count; }

	ProcessNode* Next(){
		curr = curr->next;
		return curr;
	}

	ProcessNode* GoToFront() {
		curr = head;
		return curr;
	}

	ProcessQueue() : count(0) {};
private:
	ProcessNode* head;
	ProcessNode* curr;
	ProcessNode* tail;

	int count;
};

void readProcesses(ProcessQueue& processes){
	ifstream processFile(DirtyBitOptions.referenceFile);

	if (processFile.is_open())
	{
		string line;
		while(processFile.good())
		{
			getline(processFile, line);
			int numProc = atoi(line.c_str());
			
			for(; numProc > 0; numProc--){
				//Skip line				
				getline(processFile, line);

				getline(processFile, line);				
				int pid = atoi(line.c_str());
				
				getline(processFile, line);				
				int numRef = atoi(line.c_str());

				Process* newProc = new Process(pid);
				processes.AddProcess(newProc);

				//Add references
				for(; numRef > 0; numRef--){					
					getline(processFile, line);
					stringstream ss(line);

					int refId;
					string action;
					ss >> refId >> action;
					
					newProc->AddReference(refId, action == "R" ? Read : Write);
				}
			}
		}
		processFile.close();

		cout << "Processes queued: " << processes.Count() << endl;
	}
}

struct Page{
	int startAddress;
};

class PageTable {
public:
	Page* GetPageAtAddress(int addr){
		int pageNum = floor(addr / DirtyBitOptions.pageSize);
		int offset = addr - (pageNum * DirtyBitOptions.pageSize);
		cout << "Getting page #" << pageNum << " with offset " << offset << endl;
		return pages[pageNum];
	}

	PageTable(){
		int numPages = pow(2, DirtyBitOptions.VAbits)/DirtyBitOptions.pageSize;

		for(; numPages > 0; numPages--)
			pages.push_back(new Page());
	}
private:
	vector<Page*> pages;
};

struct PageTableEntry{
	int frameNum;
	int validBits;
	int referencedBits;
	int dirtyBits;
};

int main(int argc, char* argv[])
{
	ProcessQueue queue = ProcessQueue();
	readOpts();
	readProcesses(queue);

	PageTable table = PageTable();

	Process* current = queue.GoToFront()->value;
	cout << "Current Process: " << current->ID << endl;
	while(true){
		Reference* curRef = current->GetNextReference();
		table.GetPageAtAddress(curRef->addr);
	}
}