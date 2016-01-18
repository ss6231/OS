typedef struct {
	int maxNumRes;
	int currentlyAvailUnits;
	int ID;
} resource;

typedef struct {
	char name [100];
	int taskNum;
	int resType;
	int num;
} activity;

typedef struct {
	int valid;
	int ID;
	activity listActiv [1000];
	int *curHeldUnits;
	int numActiv;
	int totalTime;
	int waiting;
	int aborted;
	int blocked;
	int curInstr;
	int timeOfLastActivity;
	int done;
	int timeBlocked;
	int computeUntilCycle;
	int computing;
	int *claimForEachRes;
	int fin;
	int *need;
} task;


typedef struct {
	task* task;
	int curInstr;
	int cycleToRelease;
	int valid;
	int unitsToRel;
} toBeReleasedStruct;
