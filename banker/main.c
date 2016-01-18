#include <stdio.h>
#include <stdlib.h>
# include <string.h>
#include <ctype.h>
#include "struct.c"

void fifo (task* tasks, resource* resources, int numTasks, int numResTypes);
void clearTasksForBanker (task*tasks, int numTasks);
void banker (task* tasks, resource *resources, int numTasks, int numResTypes, int claim);

int isNum (char *word);

int main (int argc, char**argv) {

	if (argc < 2) {
		printf ("ERROR: Not enough arguments. Exiting program\n");
		exit (EXIT_SUCCESS);
	}
	else if (argc > 2) {
		printf ("ERROR: Too many arguments. Ignoring superfluous input\n");
	} 
	FILE *fp;
	if (fopen (argv[1], "r")) {
		fp = fopen (argv[1], "r");
	}
	else {
		printf ("ERROR: File does not exist. Exiting program\n");
		exit(EXIT_SUCCESS);
	}
	int numTasks;
	int numResTypes;
	int maxNumRes;
	task* tasks;
	resource* resources;
	activity *curActiv = (activity *) malloc (sizeof (activity));

	int initCounter = 0;
	int numCounter;
	char singleWord [100];
	int claim;

	while (fscanf (fp, "%s", singleWord) == 1) {
		if (isNum(singleWord) && initCounter < 3) {
			int var = atoi(singleWord);

			//the next 3 if statements are used to grab all initialization information of the first line in the input file
			
			//this stores the task ID 
			if (initCounter == 0) {
				numTasks = var;
				tasks = malloc (sizeof (task ) * numTasks);
				for (int i = 0; i < numTasks; i++) {
					tasks[i].ID = i;
					tasks[i].valid = 1;
				}
				initCounter++;
			}

			//This just initializes the claim for each task (equivalent to the number of distinct resources)
			else if (initCounter == 1) {
				numResTypes = var;
				resources = malloc (sizeof (resource )* numResTypes);
				for (int i = 0; i < numTasks; i++) {
					tasks[i].claimForEachRes = (int *) malloc (sizeof (int) * numResTypes);
				}
				initCounter++;
			}
			//This initializes the maximum number of resources, currently available units, and ID of each resource
			else if (initCounter == 2) {
				maxNumRes = var;
				for (int i = 0; i < numResTypes; i++) {
					resources[i].maxNumRes= maxNumRes;
					resources[i].currentlyAvailUnits= maxNumRes;
					resources[i].ID = i+1;
				}
				initCounter++;
			}
		}
	//all initializations are retrieved, move on to activities
		else {
			//come across new activity, conver to string and store in the list of activities (curActiv)
			if (!isNum (singleWord)) {
				numCounter = 0;
				strcpy (curActiv->name, strdup(singleWord));
			}
			//we're inbetween activities, collecting numbers for each activity
			else {
				//grab the task number that associates with this activity
				int num = atoi(singleWord);
				if (numCounter == 0) {
					curActiv->taskNum = num;
				}
				//grab the resource ID that associates with this activity 
				else if (numCounter == 1) {
					curActiv->resType = num;
				}	
				//grab the number of units associated with this activity 
				else if (numCounter == 2) {
					curActiv->num = num;
					int taskNo = curActiv->taskNum-1;
					task *t = &tasks[taskNo];
					int index = t->numActiv;
					activity *cur = &t->listActiv[index];
					cur = malloc (sizeof (activity));
					cur = curActiv;
					t->listActiv[index] = *cur;
					t->numActiv = (t->numActiv)+1;
				}
				numCounter++;
			}
		}
	}
	fifo (tasks, resources, numTasks, numResTypes);
	fclose(fp);
}
//checks to see if this word is a number 
int isNum (char *word) {
	for (int i = 0; i < strlen(word); i++) {
		if (word[i] < 48 || word[i] > 57) {
			return 0;
		}
	}
	return 1;
}









