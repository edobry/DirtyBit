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

const struct Reference{
	const int addr;
	const Action action;

	Reference(int addr, Action action) : addr(addr), action(action) {}
};

struct Frame;

struct Page{
	Frame* frameRef;
	bool valid;
	bool referenced;
	bool dirty;

	void AssignFrame(Frame* frame) {
		frameRef = frame;
	}

	Page() : valid(true) {}
};

class PageTable {
public:
	Page* GetPageAtAddress(int addr){
		int pageNum = addr / DirtyBitOptions.pageSize;
		int offset = addr - (pageNum * DirtyBitOptions.pageSize);
		cout << "Getting page #" << pageNum << " with offset " << offset << endl;
		return pages[pageNum];
	}

	Page* GetPageAtIndex(int index){ return pages.at(index); }

	int Count(){ return pages.size(); }

	PageTable(){
		int numPages = pow(2, DirtyBitOptions.VAbits)/DirtyBitOptions.pageSize;

		for(; numPages > 0; numPages--)
			pages.push_back(new Page());
	}
private:
	vector<Page*> pages;
};

struct Process {
public:
	const int ID;

	void setWaitTime(int waitTime) {
		this->waitTime = waitTime;
	}

	void AddReference(int id, Action action){
		references.push(new Reference(id, action));
	}

	Reference* GetNextReference(){
		Reference* ref = references.front();
		references.pop();
		return ref;
	}

	Page* GetReferencedPage(Reference* ref){
		return pages->GetPageAtAddress(ref->addr);
	}

	bool IsPageReferenced(*

	bool IsAnyPageReferencingFrameDirty(Frame* frame){
		for(int i = 0; i < pages->Count(); i++) 
			if(pages->GetPageAtIndex(i)->dirty)
				return true;
		
		return false;
	}

	Process(int id) : ID(id), waitTime(0) {
		references = queue<Reference*>();
		pages = new PageTable();
	}
private:
	queue<Reference*> references;
	PageTable* pages;
	int waitTime;
};

struct Frame {
	Process* owner;
	int waitTime;

	Frame() : owner(NULL), waitTime(0) {}
};

class FrameTable {
public:
	void DecrementWait(){
		for(int i = 0; i < frames.size; i++) {
			if (frames[i]->waitTime > 0)
				frames[i]->waitTime--;
		}
	}

	void ReleaseProcessFrames(Process* owner){
		for (int i = 0; i < frames.size; ++i) {
			Frame* curr = frames[i];
			if (curr->owner == owner) {
				curr->owner = NULL;
				//foreach frame where referencing page is dirty, set frame waitTime = dirtyPagePenalty
				if(owner->IsAnyPageReferencingFrameDirty(curr))
					curr->waitTime = DirtyBitOptions.dirtyPagePenalty;
			}
		}
	}

	Frame* GetFirstUnownedFrame(){
		for(int i = 0; i < frames.size; i++)
			if(frames[i]->owner == NULL)
				return frames[i];
	}

	FrameTable() {
		int numFrames = pow(2, DirtyBitOptions.VAbits - DirtyBitOptions.PAbits);
		frames = vector<Frame*>(numFrames);
	}
private:
	vector<Frame*> frames;
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

int main(int argc, char* argv[])
{
	ProcessQueue queue = ProcessQueue();
	readOpts();
	readProcesses(queue);
	
	FrameTable frameTable = FrameTable();

	Process* current = queue.GoToFront()->value;
	cout << "Current Process: " << current->ID << endl;
	while(true){
		Reference* curRef = current->GetNextReference();
		Page* page = current->GetReferencedPage(curRef);
		if(page->valid){
			//if reference is a write, mark the page it refers to as dirty
			if (curRef->action == Action::Write)
				page->dirty = true;
		}
		else{
			Frame* frame = frameTable.GetFirstUnownedFrame();
			//assign to page, mark page as valid and referenced, and not dirty
			page->AssignFrame(frame);
			page->valid = true;
			page->referenced = true;
			page->dirty = false;
			//set process waitTime to missPenalty
			current->setWaitTime(DirtyBitOptions.missPenalty);
			
			//set frame owner to current process
			frame->owner = current;

			// if the page is not valid, no free frame has been found
				//look for frames that have not been used recently
				//loop through frames:
					//if any frame is owned by a process which has a page with this... WTF SIMON
					//else, if any frame has a waitTime of 0
						//assign to page, mark page as valid and referenced
						//if dirty, add dirtyPagePenalty to process waitTime
						//set previous owner's page to not valid
						//set frame owner to current process
						//break
				//if page is invalid, do that^ loop... WTF SIMON
		}

	}
}