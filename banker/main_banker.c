#include <stdio.h>
#include <stdlib.h>
# include <string.h>
#include <ctype.h>
#include "struct.c"

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

			//the next 3 if statements are used to grab all initialization information
			if (initCounter == 0) {
				numTasks = var;
				tasks = malloc (sizeof (task ) * numTasks);
				int i;
				for (i = 0; i < numTasks; i++) {
					tasks[i].ID = i;
					tasks[i].valid = 1;
				}
				initCounter++;
			}
			else if (initCounter == 1) {
				numResTypes = var;
				resources = malloc (sizeof (resource )* numResTypes);
				int i;
				for (i = 0; i < numTasks; i++) {
					tasks[i].claimForEachRes = (int *) malloc (sizeof (int) * numResTypes);
				}
				initCounter++;
			}
			else if (initCounter == 2) {
				maxNumRes = var;
				int i;
				for (i = 0; i < numResTypes; i++) {
					resources[i].maxNumRes= maxNumRes;
					resources[i].currentlyAvailUnits= maxNumRes;
					resources[i].ID = i+1;
				}
				initCounter++;
			}
		}
	//all initializations are retrieved, move on to activities
		else {
			//come across new activity
			if (!isNum (singleWord)) {
				numCounter = 0;
				strcpy (curActiv->name, strdup(singleWord));
			}
			//we're inbetween activities, collecting numbers for each activity
			else {
				int num = atoi(singleWord);
				if (numCounter == 0) {
					curActiv->taskNum = num;
				}
				else if (numCounter == 1) {
					curActiv->resType = num;
				}	
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
	banker (tasks, resources, numTasks, numResTypes, claim);
	fclose(fp);
}

int isNum (char *word) {
	int i;
	for (i = 0; i < strlen(word); i++) {
		if (word[i] < 48 || word[i] > 57) {
			return 0;
		}
	}
	return 1;
}









