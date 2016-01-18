#include "struct.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void fifo2 (task* tasks, resource* resources, int numTasks, int numResTypes);
void updateReleaseTask (toBeReleasedStruct toBeReleased [], resource* resources);
void addToReleaseArray (task *taskToAdd, toBeReleasedStruct toBeReleased [], int );
void initReleaseArray (toBeReleasedStruct toBeReleased []);
void initBlockedArray (task blocked []);
void addToBlockedArray (task *, task blocked[], int remainingTasks,  toBeReleasedStruct toBeReleased [], resource * resources, int numTasks);
int request (task *reqTask, resource * resources, task blocked [], int numTasks, toBeReleasedStruct toBeReleased [], int remainingTasks);
task * getSmallestValidBlockedTask (task blocked []);
void updateBlockedTasks (task * tasks, task blocked [], resource * resources,toBeReleasedStruct toBeReleased [], int numTasks, int remainingTasks);
int equal (task *, task*);
task* findTask (task *tasks, task* reqTask, int numTasks);
void clearCycleBits (task *tasks, int numTasks);
int hasThisTaskGoneInThisCycle (task* task);
void initHeldArray (task* tasks, int numTasks, int numResTypes);
void initLastActivity (task*, int numTasks);
int checkIfMultipleReady (task* potentiallyReady, task blocked [], int numTasks,resource* resources);
void pickFifoTask (task* potentiallyReady, task blocked [], int numTasks, int lenPtr, task * tasks, resource* resources,toBeReleasedStruct toBeReleased [], int remainingTasks);
void updateComputingTasks (task* tasks, int numTasks);
void printResults (task* tasks, int numTasks);


int i = 0;
int cycle;

toBeReleasedStruct toBeReleased [500];
int maxSizeOfReleaseArr = 500;
task blocked [500];
int maxSizeofBlockedArr = 500;
int sizeOfBlockedArr;
int done;

void fifo (task* tasks, resource* resources, int numTasks, int numResTypes) {
	initReleaseArray (toBeReleased);
	initBlockedArray (blocked);
	initHeldArray (tasks, numTasks, numResTypes);
	initLastActivity (tasks, numTasks);
	int remainingTasks = numTasks;

	printf ("\n");

	while (1) {
		//first update previously released resources

		updateReleaseTask (toBeReleased, resources);
		updateBlockedTasks (tasks, blocked,resources, toBeReleased, numTasks, remainingTasks);
		updateComputingTasks (tasks, numTasks);
		

		//then update blocked tasks
		//now process any unblocked and ready tasks

		task* curTask = &tasks[i];
		activity *curActivList = curTask->listActiv;
		int curInstr = curTask->curInstr;
		int goneBefore = hasThisTaskGoneInThisCycle (curTask);
		if (curTask->blocked == 0 && curTask->aborted == 0 && goneBefore == 0 && curTask->done == 0 && curTask->computing == 0) {
			if (strcmp (curActivList[curInstr].name, "initiate") == 0) {
				curTask->curInstr+=1;
			}
			else if (strcmp(curActivList[curInstr].name, "compute") == 0) {
				int computeTime = curActivList[curInstr].resType;
				curTask->computing = 1;
				curTask->computeUntilCycle = computeTime + cycle;
			}
			else if (strcmp(curActivList[curInstr].name, "request") == 0) {
				request (curTask, resources, blocked, numTasks, toBeReleased, remainingTasks);
				curTask->curInstr += 1;
			}
			else if (strcmp(curActivList[curInstr].name, "release") == 0) {
				int unitsBeingReleased = curActivList[curInstr].num;
				int forThisResource = curActivList[curInstr].resType;
				addToReleaseArray (curTask, toBeReleased, unitsBeingReleased);
				curTask->curInstr += 1;
			}
			else if (strcmp(curActivList[curInstr].name, "terminate") == 0) {
				curTask->done = 1;
				curTask->totalTime = cycle;
				done++;
				remainingTasks--;
			}
			if (curTask->computing == 0) {
				curTask->timeOfLastActivity = cycle;
			}
		}

		i++;
		if (i % numTasks == 0) {
			i = 0;
			cycle++;
		}
		if (done == numTasks) {
			printResults (tasks, numTasks);
			break;
		}
	}
}
void initReleaseArray (toBeReleasedStruct toBeReleased[]) {
	int i;
	for (i = 0; i < maxSizeOfReleaseArr; i++) {
		toBeReleased[i].valid = 0;
	}
}
/*
This function grants a request so long as the units being request <= current units available. If so, it updates
the current units available for that resource and the currently held units for that process. If request exceeds
currently available units, the process is blocked. 
*/
int request (task *reqTask, resource * resources, task blocked [], int numTasks, toBeReleasedStruct toBeReleased [], int remainingTasks) {
	activity *curActivList = reqTask->listActiv;
	int unitsBeingRequested = curActivList[reqTask->curInstr].num;
	int processNum = curActivList[reqTask->curInstr].resType;
	int unitsCurAvail = resources[processNum-1].currentlyAvailUnits;
	if (unitsBeingRequested <= unitsCurAvail) {
		//grant request & update currently avail res in resources array
		resources[processNum-1].currentlyAvailUnits -= curActivList[reqTask->curInstr].num;

		/*Note: reqTask->curHeldUnits[x] is initialized to -1, so if this is the first time this task is asking for a request,
		then the curHeldUnits = unitsBeingRequest, otherwise, it is a cumulative calculation
		*/

		if (reqTask->curHeldUnits[processNum-1] != -1) {
			reqTask->curHeldUnits[processNum-1] += unitsBeingRequested;
		}
		else {
			reqTask->curHeldUnits[processNum-1] = unitsBeingRequested;
		}
		return 1;
	}
	else {
		if (reqTask->blocked != 1) {
			reqTask->timeBlocked = cycle;
			addToBlockedArray (reqTask, blocked, remainingTasks, toBeReleased, resources, numTasks);
		}
		return 0;
	}
}

/*
This function adds a task to the toBeRelease array of type toBeReleasedStruct, which stores the units to be released
and the cycle at which to release this task
*/
void addToReleaseArray (task *taskToAdd, toBeReleasedStruct toBeReleased [], int unitsToRel) {
	int g;
	for (g = 0; g < maxSizeOfReleaseArr; g++) {
	task* taskInArr = toBeReleased[g].task;
		if (toBeReleased[g].valid == 0) {
			toBeReleased[g].task = taskToAdd;
			toBeReleased[g].cycleToRelease = cycle+1;
			toBeReleased[g].valid = 1;
			toBeReleased[g].unitsToRel = unitsToRel;
			return;
		}
	}
}
/*
This function loops through the array of tasks to be released and checks to see if it is possible to release them
(ie: when thecycleToRelease variable is equal to the current cycle). Then the units it previously held are released
and ready to be used 
*/
void updateReleaseTask (toBeReleasedStruct toBeReleased [], resource* resources) {
	int i;
	for (i = 0; i < maxSizeOfReleaseArr; i++) {
		if (toBeReleased[i].valid == 1 && toBeReleased[i].cycleToRelease == cycle) {
			//then release the units previously held by this task for that particular resource

			task* curTask = toBeReleased[i].task;
			int unitsBeingReleased = toBeReleased[i].unitsToRel;
			int forThisResource = curTask->listActiv[curTask->curInstr-1].resType;
			resources[forThisResource-1].currentlyAvailUnits += unitsBeingReleased;
			curTask->curHeldUnits[forThisResource-1] -= unitsBeingReleased;
			//zero out name to represent the removal of this task from the array 
			toBeReleased[i].valid = 0;
		}
	}
}

/*
This function initializes the array of blocked tasks
*/
void initBlockedArray (task blocked[]) {
	int i;
	for (i = 0; i < maxSizeofBlockedArr; i++) {
		blocked[i].valid = 0;
	}
}

/*
This function adds a task to the array of blocked tasks and updates such information as the time of block, size of
blocked array, and the time the task has to spend waiting. This method also checks for any deadlocks when the size 
of the blocked array is equal to the number of remaining valid tasks (ie: tasks that are not aborted/terminated).
While the requested units > available units for that task, we must abort tasks - these aborted tasks are chosen by
smallest ID number. This will continue aborting tasks so long as if requested units > available units. When such 
an aborted task is found, various bits of information are updated, the most important of which is adding the task
to the toBeReleased array which will make the aborted task's units available at the next cycle
*/
void addToBlockedArray (task *taskToAdd, task blocked[], int remainingTasks, toBeReleasedStruct toBeReleased [], resource* resources, int numTasks) {
	int i;
	for (i = 0; i < maxSizeofBlockedArr; i++) {
		if (blocked[i].valid == 0) {
			blocked[i] = *taskToAdd;
			taskToAdd->blocked = 1;
			blocked[i].valid = 1;
			blocked[i].blocked = 1;
			sizeOfBlockedArr++;
			taskToAdd->waiting += 1;
			break;
		}
	}
	//must check for deadlocked case - this occurs when the num of blocked proc is equal to the total num of tasks

	if (sizeOfBlockedArr == remainingTasks) {
		//while taskToAdd's request cannot be granted, then keep aborting things from blocked array 
		int reqUnits = taskToAdd->listActiv[taskToAdd->curInstr].num;
		int process = taskToAdd->listActiv[taskToAdd->curInstr].resType;
		int avail = resources [process-1].currentlyAvailUnits;  
		while (reqUnits > avail) {
			task *toBeAborted;
			int smallestIndex = numTasks;
			int i;
			for (i = 0; i < maxSizeofBlockedArr; i++) {
				if (blocked[i].valid == 1 && sizeOfBlockedArr == 1 && smallestIndex == blocked[i].ID+1) {
					printf ("ALL TASKS ABORTED");
					exit (EXIT_SUCCESS);
					//now ALL TASKS ARE ABORTED EXIT PROGRAM
				}
				if (blocked[i].valid == 1 && smallestIndex > blocked[i].ID+1){
					toBeAborted = &blocked[i];
					blocked[i].valid = 0;
					toBeAborted->blocked = 0;
					toBeAborted->aborted = 1;
					sizeOfBlockedArr--;
					avail += toBeAborted->curHeldUnits[process-1];
					toBeAborted->timeBlocked = 0;
					blocked[i].aborted = 0;
					remainingTasks--;
					done++;
					addToReleaseArray (toBeAborted, toBeReleased, toBeAborted->curHeldUnits[process-1]);
					if (reqUnits <= avail) {
						break;
					}
				}
			}
		}
	}
}

/*
This function updates any blocked requests in the blocked array. The first part of this function (the first if
statement) is only there to see if multiple request are eligible to be unblocked and granted - if so, then it 
picks the one that was blocked first to be freed from this array and granted its request. Otherwise, this function
loops through each of the tasks and calls the request method - if its a successful request, the task is unblocked,
otherwise the task remains blocked 
*/
void updateBlockedTasks (task * tasks, task blocked [], resource * resources, toBeReleasedStruct toBeReleased [], int numTasks, int remainingTasks) {
	//first check if there are multiple tasks that are ready to be unblocked according to current resources available
	task* potentiallyReady = (task *) (malloc (numTasks * sizeof(task)));
	int lenPtr = 0;
	lenPtr = checkIfMultipleReady (potentiallyReady, blocked, numTasks, resources);
	if (lenPtr > 1) {
		pickFifoTask (potentiallyReady, blocked, numTasks, lenPtr, tasks, resources, toBeReleased, remainingTasks);
	}
	else {
		int i;
		for (i = 0; i < maxSizeofBlockedArr; i++) {
			if (blocked[i].valid == 1) {
				task *reqTask = &blocked[i];
				task *taskInOrigArr = findTask (tasks, reqTask, numTasks);
				if (request (reqTask, resources, blocked, numTasks, toBeReleased, remainingTasks) == 1) {
					blocked[i].valid = 0;
					blocked[i].blocked = 0;
					sizeOfBlockedArr--;
					reqTask->blocked = 0;
					taskInOrigArr->blocked = 0;
					taskInOrigArr->timeOfLastActivity = cycle;
					taskInOrigArr->waiting = cycle - taskInOrigArr->timeBlocked;
					taskInOrigArr->timeBlocked = 0;
				}
			}
		}
	}
	free (potentiallyReady);
}
/*
Given the tasks array and the reqTask task structure, this function returns a pointer pointing to the
equivalent reqTask task in the tasks array by checking to see if reqTask is equal to any of the tasks in the array
*/
task* findTask (task *tasks, task* reqTask, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		if (equal (&tasks[i], reqTask)) {
			return &tasks[i];
		}
	}
}
/*
This method checks to see if 2 tasks are equal to each other by checking major variables specific to each task such
as ID, number of activities, values of each activity (and the order in which it occurs) etc.
*/
int equal (task *cur, task* test) {
	if (cur->ID != test->ID) {
		return 0;
	}
	else if (cur->numActiv != test->numActiv) {
		return 0;
	}
	int i;
	for (i = 0; i < cur->numActiv; i++) {
		if (strcmp(cur->listActiv[i].name, test->listActiv[i].name) != 0) {
			return 0;
	}
		else if (cur->listActiv[i].taskNum != test->listActiv[i].taskNum) {
			return 0;
		}
		else if (cur->listActiv[i].resType != test->listActiv[i].resType) {
			return 0;
		}
		else if (cur->listActiv[i].num != test->listActiv[i].num) {
			return 0;
		}
	}
	return 1;
}
/*
This function checks to see if the current task has executed an activity in the current cycle 
*/
int hasThisTaskGoneInThisCycle (task *curTask) {
	if (curTask->timeOfLastActivity == cycle && cycle != 0) {
		return 1;
	}
	return 0;
}

/*
This method just allocates memory to the currently held units of each task for each resource (ie: each task has a
pointer to curHeldUnits, which stores the units of each resource this task is currently holding on to). It also
initializes the held units to 0
*/
void initHeldArray (task* tasks, int numTasks, int numResTypes) {
	int i;
	for (i = 0; i < numTasks; i++) {
		tasks[i].curHeldUnits = (int *) malloc (numResTypes * sizeof(int));
		int j;
		for (j = 0; j < numResTypes; j++) {
			tasks[i].curHeldUnits[j] = -1;
		}
	}
}
/*
This function initializes each task's time of last activity, so that no executes more than 1 activity in a cycle
*/
void initLastActivity (task* tasks, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		tasks[i].timeOfLastActivity = 0;
	}
}
/*
This function goes hand in hand with updateBlocked() function to see if there are more than 1 blocked tasks eligble
to be freed and requests granted. 
*/
int checkIfMultipleReady (task* potentiallyReady,  task blocked [], int numTasks, resource * resources) {
	int len = 0;
	int i;
	for (i = 0; i < maxSizeofBlockedArr; i++) {
		if (blocked[i].valid == 1){
			task *curTask = &blocked[i];
			activity *curActivList = curTask->listActiv;
			int unitsBeingRequested = curActivList[curTask->curInstr].num;
			int forThisResource = curActivList[curTask->curInstr].resType;
			int unitsCurAvail = resources[forThisResource-1].currentlyAvailUnits;
			if (unitsBeingRequested <= unitsCurAvail) {
				potentiallyReady[i] = *curTask;
				len++;
			}
		}
	}
	return len;
}
/*
This function also works with updateBlocked(). If there is more than 1 eligible blocked task, it picks the one
that was blocked the earliest. Then that earliest task's request is checked to see if it is possible and successful
*/
void pickFifoTask (task* potentiallyReady, task blocked [], int numTasks, int lenPtr, task * tasks, resource* resources,toBeReleasedStruct toBeReleased [], int remainingTasks) {
	int smallestTime = 1000;
	int index = -1;
	int i;
	for (i = 0; i < lenPtr; i++) {
		if (potentiallyReady[i].valid == 1 && potentiallyReady[i].timeBlocked < smallestTime) {
			smallestTime = potentiallyReady[i].timeBlocked;
			index = i;
		}
	}
	//by the end of for loop we should have the right FIFO task - time to grant request
	if (index != -1) {
		task * fifoTask = &potentiallyReady[index];
		task *taskInOrigArr = findTask (tasks, fifoTask, numTasks);
		request (fifoTask, resources, blocked, numTasks, toBeReleased, remainingTasks);
		task *inBlockedArray = findTask(blocked, fifoTask, maxSizeofBlockedArr);
		inBlockedArray->valid = 0;
		inBlockedArray->blocked = 0;
		sizeOfBlockedArr--;
		fifoTask->blocked = 0; 
		taskInOrigArr->waiting = cycle - taskInOrigArr->timeBlocked;
		taskInOrigArr->blocked = 0;
		taskInOrigArr->timeOfLastActivity = cycle;
	}

}
/*
This function is called at the beginning of every cycle to check if a task that is currently computing is ready
to be released
*/
void updateComputingTasks (task* tasks, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		task *curTask = &tasks[i];
		if (curTask->computing == 1) {
			if (curTask->computeUntilCycle == cycle) {
				curTask->computing = 0;
				curTask->curInstr += 1;
			}
		}
	}
}

/*
Prints the formatted results of FIFO
*/

void printResults (task * tasks, int numTasks) {
	int totalTerm = 0;
	int totalWait = 0;
	int i;
	for (i = 0; i < numTasks; i++) {
		if (tasks[i].totalTime) {
			totalTerm += tasks[i].totalTime;
			totalWait += tasks[i].waiting;
		}
	}
	printf ("\nFIFO OUTPUT\n");
	for (i = 0; i < numTasks; i++) {
		if (tasks[i].totalTime) {
			printf ("For task  %d: \n", tasks[i].ID+1);
			printf ("Terminated: %d\n", tasks[i].totalTime);
			printf ("Waiting: %d\n", tasks[i].waiting);
			printf ("Percent spent waiting: %.0f%%\n", 100*(float)(tasks[i].waiting)/(float)tasks[i].totalTime);
			printf("\n");
			if (i == numTasks - 1) {
				printf ("Total execution: %d    %d    %.0f%%\n", totalTerm, totalWait,100*(float)(totalWait)/(float)totalTerm);
				printf ("\n");
			}
		}
		else {
			printf ("Task %d aborted\n", tasks[i].ID+1);
		}
	}
	printf ("\n");
}





