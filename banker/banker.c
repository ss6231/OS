#include "struct.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void banker (task* tasks,resource* resources,int numTasks,int numResTypes, int claim);
int req (task *reqTask, resource * resources, task blocked [], int numTasks, toBeReleasedStruct toBeReleased [], int numResTypes, task* tasks); 
void goingToBeReleased (task *taskToAdd, toBeReleasedStruct toBeReleased [], int );
void results (task* tasks, int numTasks);
void initHoldingUnits (task* tasks, int numTasks, int numResTypes);
int isThisSafe (int unitsAlreadyUsed, int claim, int unitsCurAvail, int unitsBeingRequested, task* curTask);
void initiate (task* curTask, int numResTypes, resource* resources);
void blockThisTask (task *, task blocked[],  toBeReleasedStruct toBeReleased [], resource * resources, int numTasks);
void updateBlocked (task * tasks, task blocked [], resource * resources,toBeReleasedStruct toBeReleased [], int numTasks, int numResTypes);
task* findProcess (task *tasks, task* reqTask, int numTasks);
int isEqual (task *cur, task* test);
void initializeReleaseArray (toBeReleasedStruct toBeReleased []);
void initializeBlockedArray (task blocked []);
void updateRelease (toBeReleasedStruct toBeReleased [], resource* resources);
void clearLastActivity (task* tasks, int numTasks);
int taskGoneBefore (task * task);
int checkToUpdateCycle (task* tasks, int numTasks);
int checkIfNewStateSafe (task* tasks, int numTasks, resource* resources, int numResTypes, int reqUnits);
int needMatchesAvailable (int *need,int available [], int numResTypes) ;
void updateAvailable (task* curTask,int available [], int numResTypes);
int multipleReady (task* potentiallyReady, task blocked [], int numTasks,resource* resources);
void pickEarliestBlock (task* potentiallyReady, task blocked [], int numTasks, int lenPtr, task * tasks, resource* resources,toBeReleasedStruct toBeReleased [], int numResTypes);
void updateCompute (task* tasks, int numTasks);

int cycle;
int i;
int done;
toBeReleasedStruct toBeReleased [500];
int capacityReleaseArr = 500;
task blocked [500];
int capacityBlockedArr = 500;
int sizeBlockedArr;
int validTasks;

void banker (task* tasks,resource* resources,int numTasks,int numResTypes, int claim) {
	initializeReleaseArray (toBeReleased);
	initializeBlockedArray (blocked);
	initHoldingUnits (tasks, numTasks, numResTypes);
	clearLastActivity (tasks, numTasks);
	validTasks = numTasks;

	//initialize claims for each resource of each task
	int a;
	for (a = 0; a < numTasks; a++) {
		task* curTask = &tasks[a];
		curTask->claimForEachRes = (int *) (malloc (sizeof (int) * numResTypes));
		int j;
		for (j = 0; j < numResTypes; j++) {
			curTask->claimForEachRes[j] = claim;
		}  
	}

	int validTasks = numTasks;

	while (1) {

		updateRelease (toBeReleased, resources);
		updateBlocked (tasks, blocked,resources, toBeReleased, numTasks, numResTypes);
		updateCompute (tasks, numTasks);
			
		task* curTask = &tasks[i];
		int curInstr = curTask->curInstr;
		activity *curActivList = curTask->listActiv;
		int goneBefore = taskGoneBefore (curTask);

		//if the task is not aborted/blocked/computing/done allow it to be processed
		if (curTask->blocked == 0 && curTask->aborted == 0 && goneBefore == 0 && curTask->done == 0 && curTask->computing == 0) {
			if (strcmp (curActivList[curInstr].name, "initiate") == 0) {
				//this function will initiate the claims for this task for each resource
				initiate (curTask, numResTypes, resources);
				if (curTask->aborted != 1) {
					curTask->curInstr+=1;
				}
			}
			else if (strcmp(curActivList[curInstr].name, "compute") == 0) {
				int computeTime = curActivList[curInstr].resType;
				curTask->computing = 1;
				curTask->computeUntilCycle = computeTime + cycle;
			}
			else if (strcmp (curActivList[curInstr].name, "request") == 0) {
				req (curTask, resources, blocked, numTasks, toBeReleased, numResTypes, tasks);
				curTask->curInstr += 1;
			}
			else if (strcmp (curActivList[curInstr].name, "release") == 0) {
				int unitsBeingReleased = curActivList[curInstr].num;
				int forThisResource = curActivList[curInstr].resType;
				goingToBeReleased (curTask, toBeReleased, unitsBeingReleased);
				curTask->curInstr += 1;
			}
			else if (strcmp(curActivList[curInstr].name, "terminate") == 0) {
				curTask->done = 1;
				validTasks--;
				curTask->totalTime = cycle;
				done++;
			}
			curTask->timeOfLastActivity = cycle;
		}
		//i updates the current task examined
		i++;
		//update all tasks if the time of last activity for all the tasks is equal to the current cycle
		if (checkToUpdateCycle(tasks, numTasks)) {
			cycle++;
			i=0;
		}
		if (done == numTasks) {
			results (tasks, numTasks);
			break;
		}
	}
}

/*
The request method begins by temporarily "granting" the current request & giving it the desired resources, so long
as the units the process is requesting is <= units available for that resource. The the method checks whether 
this new state is safe. Note that the CheckIfNewStates() method returns 1 for sucess, 0 for fail, and -1 if a
task suddenly aborts (ie: when the request > claim). If successful, this req method will return 1 to let the program
move forward. In any failed case (when 0 or -1 is returned), this method unallocates the request initially granted
and blocks if necessary 
*/


int req (task *reqTask, resource * resources, task blocked [], int numTasks, toBeReleasedStruct toBeReleased [], int numResTypes, task* tasks) {
	activity *curActivList = reqTask->listActiv;
	int unitsBeingRequested = curActivList[reqTask->curInstr].num;
	int processNum = curActivList[reqTask->curInstr].resType;
	int unitsAlreadyUsed = reqTask->curHeldUnits[processNum-1];
	int claim = reqTask->claimForEachRes[processNum-1];
	int unitsCurAvail = resources[processNum-1].currentlyAvailUnits;


	if (unitsBeingRequested <= unitsCurAvail) {
		//"grant" the request as if it actually happened
		resources[processNum-1].currentlyAvailUnits -= curActivList[reqTask->curInstr].num;
		reqTask->curHeldUnits[processNum-1] += unitsBeingRequested;

		//now check if the new state is safe
		int result = checkIfNewStateSafe(tasks, numTasks, resources, numResTypes, unitsBeingRequested);
		if (result == 1) {
			return 1;
		}
		//aborted task from the newly safe state
		else if (result == -1) {
			resources[processNum-1].currentlyAvailUnits += curActivList[reqTask->curInstr].num;
			reqTask->curHeldUnits[processNum-1] -= unitsBeingRequested;
			validTasks--;
			return 0;
		}
		else {
			resources[processNum-1].currentlyAvailUnits += curActivList[reqTask->curInstr].num;
			reqTask->curHeldUnits[processNum-1] -= unitsBeingRequested;
			if (reqTask->blocked != 1) {
				reqTask->timeBlocked = cycle;
				blockThisTask (reqTask, blocked, toBeReleased, resources, numTasks);
				return 0;
			}
			return 0;
		}
	}
	else {
		if (reqTask->blocked != 1) {
				reqTask->timeBlocked = cycle;
				blockThisTask (reqTask, blocked, toBeReleased, resources, numTasks);
				return 0;
		}
		return 0;
	}
}

/*
This method just allocates memory to the currently held units of each task for each resource (ie: each task has a
pointer to curHeldUnits, which stores the units of each resource this task is currently holding on to). It also
initializes the held units to 0
*/

void initHoldingUnits (task* tasks, int numTasks, int numResTypes) {
	int i;
	for (i = 0; i < numTasks; i++) {
		tasks[i].curHeldUnits = (int *) malloc (numResTypes * sizeof(int));
		int j;
		for (j = 0; j < numResTypes; j++) {
			tasks[i].curHeldUnits[j] = 0;
		}
	}
}

/*
Printing out final results
*/

void results (task* tasks, int numTasks) {
	int totalTerm = 0;
	int totalWait = 0;
	int i;
	for (i = 0; i < numTasks; i++) {
		if (tasks[i].totalTime) {
			totalTerm += tasks[i].totalTime;
			totalWait += tasks[i].waiting;
		}
	}
	printf ("\nBANKER OUTPUT\n");
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

/*
When the current activity is an initiate, the method stores the claim for each task and resource and also checks 
to see if any claims > units of any resource present. This will lead to an abort of that task
*/

void initiate (task* curTask, int numResTypes, resource* resources) {
	int i;
	for (i = 0; i < numResTypes; i++) {
		activity* curActivList = curTask->listActiv;
		int claim = curActivList[curTask->curInstr].num;
		curTask->claimForEachRes[i] = claim;
		int maxUnitsInRes = resources[i].maxNumRes;
		if (claim > maxUnitsInRes) {
				curTask->aborted = 1;
				curTask->valid = 0;
				validTasks--;
				curTask->blocked = 0;
				sizeBlockedArr--;
				curTask->timeBlocked = 0;
				done++;
				return;
		} 
	}
}

/*
Given the parameters, listed, this function will add a task to an array that stores blocked tasks and updates 
variables, such as the number of blocked tasks, waiting variable of each task, if the blocked task is valid, 
whether the task is actually blocked, etc.
*/

void blockThisTask (task *taskToAdd, task blocked[], toBeReleasedStruct toBeReleased [], resource* resources, int numTasks) {
	int i;
	for (i = 0; i < capacityBlockedArr; i++) {
		if (blocked[i].valid == 0) {
			blocked[i] = *taskToAdd;
			taskToAdd->blocked = 1;
			blocked[i].valid = 1;
			blocked[i].blocked = 1;
			sizeBlockedArr++;
			taskToAdd->waiting += 1;
			break;
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

void updateBlocked (task * tasks, task blocked [], resource * resources, toBeReleasedStruct toBeReleased [], int numTasks, int numResTypes) {
	task* potentiallyReady = (task *) (malloc (numTasks * sizeof(task)));
	int lenPtr = 0;
	lenPtr = multipleReady (potentiallyReady, blocked, numTasks, resources);
	if (lenPtr > 1) {
		pickEarliestBlock (potentiallyReady, blocked, numTasks, lenPtr, tasks, resources, toBeReleased, numResTypes);
	}
	int i;
		for (i = 0; i < capacityBlockedArr; i++) {
			if (blocked[i].valid == 1) {
				task *reqTask = &blocked[i];
				task *taskInOrigArr = findProcess (tasks, reqTask, numTasks);
				if (taskGoneBefore(taskInOrigArr) == 0 && req (reqTask, resources, blocked, numTasks, toBeReleased, numResTypes, tasks) == 1) {
					blocked[i].valid = 0;
					blocked[i].blocked = 0;
					sizeBlockedArr--;
					reqTask->blocked = 0;
					taskInOrigArr->blocked = 0;
					taskInOrigArr->waiting = cycle - taskInOrigArr->timeBlocked;
					taskInOrigArr->timeBlocked = 0;
				}
				taskInOrigArr->timeOfLastActivity = cycle;
			}
		}
	free (potentiallyReady);
}

/*
Given the tasks array and the reqTask task structure, this function returns a pointer pointing to the
equivalent reqTask task in the tasks array by checking to see if reqTask is equal to any of the tasks in the array
*/
task* findProcess (task *tasks, task* reqTask, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		if(tasks[i].valid == 1) {
			if (isEqual (&tasks[i], reqTask)) {
				return &tasks[i];
			}
		}
	}
}

/*
This method checks to see if 2 tasks are equal to each other by checking major variables specific to each task such
as ID, number of activities, values of each activity (and the order in which it occurs) etc.
*/

int isEqual (task *cur, task* test) {
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
This function adds a task to the toBeReleased array of toBeReleasedStructs where each struct stores the 
units to be released and the cycle the units will be released it. This function works in conjunction with 
updateReleased(), which is checked at the beginning of each cycle
*/

void goingToBeReleased (task *taskToAdd, toBeReleasedStruct toBeReleased [], int unitsToRel) {
	int g;
	for (g = 0; g < capacityReleaseArr; g++) {
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
This function initializes the array of toBeReleasedStructs
*/

void initializeReleaseArray (toBeReleasedStruct toBeReleased[]) {
	int i;
	for (i = 0; i < capacityReleaseArr; i++) {
		toBeReleased[i].valid = 0;
	}
}

/*
This function initializes the blocked array of tasks 
*/

void initializeBlockedArray (task blocked[]) {
	int i;
	for (i = 0; i < capacityBlockedArr; i++) {
		blocked[i].valid = 0;
	}
}

/*
This function loops through the array of tasks to be released and checks to see if it is possible to release them
(ie: when thecycleToRelease variable is equal to the current cycle). Then the units it previously held are released
and ready to be used 
*/

void updateRelease (toBeReleasedStruct toBeReleased [], resource* resources) {
	int i;
	for (i = 0; i < capacityReleaseArr; i++) {
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
This function initializes each task's time of last activity, so that no executes more than 1 activity in a cycle
*/
void clearLastActivity (task* tasks, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		tasks[i].timeOfLastActivity = -1;
	}
}

/*
This function checks to see if the current task has executed an activity in the current cycle 
*/

int taskGoneBefore (task *curTask) {
	if (curTask->timeOfLastActivity == cycle && cycle != 0) {
		return 1;
	}
	return 0;
}
int checkToUpdateCycle (task *tasks, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		if (tasks[i].valid) {
			if (tasks[i].aborted || tasks[i].done || tasks[i].computing) {
				continue;
			}
			if (tasks[i].timeOfLastActivity != cycle) {
				return 0;
			}
		}
	}
	return 1;
}

/*
This function checks to see if the "granted" request from the req method results in a safe state. It works by first
calculating a need vector for each task (calculated as claim - units already used by this task for this process)
and an available vector, which is just the currently available units for this resource. If the request units exceed
the claim, the task is aborted and this method returns a -1 to the req () function. Otherwise, the next step 
is to check for safety of the current state given the "request" done in the req () function. It does so by looping
through each task and looking for a task such that task[fin] = 0 (this means its needs > available and it cannot
proceed YET. This does not automatically mean an unsafe state) AND the need <= available. When such a task is found,
task[fin] is marked as 1 to indicate that this process can successfully run to completion. The loop will continue 
until all task[fin] = 1 (ie: all future requests run to completion) or when no such task exists such that need <= available.
The last loop of this function checks to see if the current state is safe or not. The only way it is safe is if ALL states are safe
(when all tasks[fin] = 1).
*/

int checkIfNewStateSafe (task* tasks, int numTasks, resource* resources, int numResTypes, int reqUnits) {
	//first set up need for each task & available for each resource
	int available [numResTypes];
	int i;
	for (i = 0; i < numTasks; i++) {
		if (tasks[i].valid) {
			task* curTask = &tasks[i];
			curTask->fin = 0;
			curTask->need = (int *) malloc (sizeof (int) * numResTypes);
			int* need = curTask->need;
			//now calculate the need & available units for each resource of each task
			int j;
			for (j = 0; j < numResTypes; j++) {
				available[j] = resources[j].currentlyAvailUnits;
				int claimForThisResource = curTask->claimForEachRes[j];
				int unitsAlreadyUsed = curTask->curHeldUnits[j];
				need[j] = claimForThisResource - unitsAlreadyUsed; 
				if (need[j] < 0) {
					curTask->aborted = 1;
					curTask->valid = 0;
					goingToBeReleased (curTask, toBeReleased, reqUnits);
					validTasks--;
					done++;
					return -1;
				}
			}
		}
	}
	//now we can check for safety
	int a = 0;
	for (a = 0; a < numTasks; a++) {
		if(tasks[a].valid) {
			if (tasks[a].fin == 0) {
				if (needMatchesAvailable (tasks[a].need, available, numResTypes)) {
					int b;
					for (b = 0; b < numResTypes; b++) {
						updateAvailable (&tasks[a], available, numResTypes);
						tasks[a].fin = 1;
						a = -1;
						break;
					}
				}
			}
		}
	}
	int numSafe = 0;
	int numUnsafe = 0;
	int k;
	for (k = 0; k < numTasks; k++) {
		if (tasks[k].valid) {
			if (tasks[k].fin == 1 || tasks[k].aborted) {
				numSafe++;
			}
			else {
				numUnsafe++;
			}
		}
	}
	if (numSafe >= validTasks) return 1;
	else return 0;
}

/*
This function checks to see if each of the needs are <= available - if so, then a safe state is possible now
*/
int needMatchesAvailable (int *need,int available [], int numResTypes) {
	int i;
	for (i = 0; i < numResTypes; i++) {
		if (need[i] <= available[i]) {
			continue;
		}
		else {
			return 0;
		}
	}
	return 1;
}

/*
When such a task is found in which need <= available, this means the task can run to completion and therefore be
able to return any units it had previously used for some resource. This function returns those units back to the
available vector so that any task that previously could not proceed now has the potential to do so. 
*/
void updateAvailable (task* curTask,int available [], int numResTypes) {
	int i;
	for (i = 0; i < numResTypes; i++) {
		available[i] += curTask->curHeldUnits[i];
	}
}

/*
This function goes hand in hand with updateBlocked() function to see if there are more than 1 blocked tasks eligble
to be freed and requests granted. 
*/
int multipleReady (task* potentiallyReady,  task blocked [], int numTasks, resource * resources) {
	int len = 0;
	int i;
	for (i = 0; i < capacityBlockedArr; i++) {
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
void pickEarliestBlock (task* potentiallyReady, task blocked [], int numTasks, int lenPtr, task * tasks, resource* resources,toBeReleasedStruct toBeReleased [], int numResTypes) {
	int smallestTime = 1000;
	int index = -1;
	int i;
	for (i = 0; i < lenPtr; i++) {
		if (potentiallyReady[i].valid == 1 && potentiallyReady[i].timeBlocked < smallestTime) {
			smallestTime = potentiallyReady[i].timeBlocked;
			index = i;
		}
	}
	if (index != -1) {
		task * fifoTask = &potentiallyReady[index];
		task *taskInOrigArr = findProcess (tasks, fifoTask, numTasks);
		if (req (fifoTask, resources, blocked, numTasks, toBeReleased, numResTypes, tasks) == 1) {
			task *inBlockedArray = findProcess(blocked, fifoTask, capacityBlockedArr);
			inBlockedArray->valid = 0;
			inBlockedArray->blocked = 0;
			sizeBlockedArr--;
			fifoTask->blocked = 0; 
			taskInOrigArr->waiting = cycle - taskInOrigArr->timeBlocked;
			taskInOrigArr->blocked = 0;
			taskInOrigArr->timeOfLastActivity = cycle;
		}
	}
}

/*
This function is called at the beginning of every cycle to check if a task that is currently computing is ready
to be released
*/
void updateCompute (task* tasks, int numTasks) {
	int i;
	for (i = 0; i < numTasks; i++) {
		if (tasks[i].valid) {
			task *curTask = &tasks[i];
			if (curTask->computing == 1) {
				if (curTask->computeUntilCycle == cycle) {
					curTask->computing = 0;
					curTask->curInstr += 1;
				}
			}
		}
	}
}




