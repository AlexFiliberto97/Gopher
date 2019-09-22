#ifndef PROCESS_H_
	
	#define PROCESS_H_

	typedef struct _Process {
		
		pid_t pid;
		int running;
		int socketIndex;
	} Process;
	
	int initProcess();
	int processIndex();
	int startProcess(void* (*f)(void*), void*, int);
	void* processCollector(void*);
	void stopProcessCollector();
	void destroyProcess();

#endif