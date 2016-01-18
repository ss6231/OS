
import java.util.ArrayList;
import java.util.Scanner;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
public class main {
	public static void main (String [] args) {
		validInput (args);
		int machineSize = Integer.parseInt(args[0]);
		int pageSize = Integer.parseInt(args[1]);
		int virtAdd = Integer.parseInt(args[2]);
		int jobMix = Integer.parseInt(args[3]);
		int numRef = Integer.parseInt(args[4]);
		String algo = args[5];

		//Set up time, current quantum, and current process number 
		int time = 1;
		int q = 0;
		int curProcNum = 0;

		System.out.println ("\nThe machine size is: " + machineSize);
		System.out.println ("The page size is: " + pageSize);
		System.out.println ("The process size is: " + virtAdd);
		System.out.println ("The job mix is: " + jobMix);
		System.out.println ("The number of references per process is: "  + numRef);
		System.out.println ("The replacement algorithm is: " + algo);

		int numVP = virtAdd / pageSize;
		int numPF = machineSize / pageSize;
		int framesAvail = numPF;

		ArrayList<process> processes = createProcs (jobMix, numRef);
		int numProcs = processes.size();
		ArrayList<frame> frames = createFrames (numPF);
		ArrayList<process> done = new ArrayList<process>();
		ArrayList<Integer> random = readRandomNum ();
		int curRandPtr = 0;
		int curWord = 111 * processes.get(0).num % virtAdd;
		int numPages = processes.size();

		while (true) {
			process curProcess = processes.get(curProcNum);
			if (curProcess.goneBefore == false) {
				curWord = 111 * curProcess.num % virtAdd;
				curProcess.goneBefore = true;
			}
			else {
				curWord = curProcess.nextRef;
			}
			//if page is not a hit, it is a page fault - continue to the stated page replacement algorithm 
			int pageNum = getPageNum (numVP, pageSize, curWord, curProcess);
			if (!isHit (curProcess, curWord, frames, pageNum, time)) {
				curProcess.numFaults += 1;
				if (algo.equals("lru")) {
					lru (curProcess, curWord, frames, time, processes, pageNum);
				}
				else if (algo.equals ("lifo")) {
					lifo (curProcess, curWord, frames, time, processes, pageNum);
				}
				else {
					curRandPtr = random (curProcess, curWord, frames, time, processes, pageNum, random.get(curRandPtr), curRandPtr);
				}
			}
			//increment the number of memory references for this process & check if it has completed
			curProcess.nRef += 1;
			if (isDone (curProcess, done, numRef)) {
				processes.remove (curProcess);
				if (done.size() == numProcs) {
					printResults(done);
					break;
				}
				//setting up next memory reference if this process has completed
				q = 0;
				time++;
				curProcess.oldRef = curWord;
				curProcess.nextRef = curProcess.getReference (curWord, virtAdd, curRandPtr++, random);
				if (curProcess.nextRef == -1) {
					int rand2 = random.get(curRandPtr++);
					curProcess.nextRef = rand2 % virtAdd;
				}
				curProcess = processes.get(curProcNum);
				continue;
			}
			//setting up next memory reference if this process has NOT completed 
			curProcess.nextRef = curProcess.getReference (curWord, virtAdd, curRandPtr++, random);
			if (curProcess.nextRef == -1) {
				int rand2 = random.get(curRandPtr++);
				curProcess.nextRef = rand2 % virtAdd;
			}
			//increment quanta, time, and check if this process has finished its quanta 
			q++;
			time++;
			curProcess.oldRef = curWord;
			if ((q == 3 || curProcess.done) && processes.size() != 0) {
				curProcNum = (curProcNum + 1 + processes.size()) % processes.size();
				q = 0;
			}
			curProcess = processes.get(curProcNum);
		}
	}
	/**
	 * @param jobmix
	 * @param numRef
	 * @return 
	 * creates each process and sets up probably according ot jobMix
	 */
	public static ArrayList<process> createProcs (int jobMix, int numRef) {
		ArrayList<process> list = new ArrayList<process> ();
		if (jobMix == 1) {
			process p = new process (1, numRef, jobMix);
			list.add (p);
			p.A = 1; p.B = 0; p.C = 0;
		}
		else {
			for (int i = 1; i < 5; i++) {
				process p = new process (i, numRef, jobMix);
				list.add(p);
				if (jobMix == 2) {
					p.A = 1; p.B = 0; p.C = 0;
				}
				else if (jobMix == 3) {
					p.A = 0; p.B = 0; p.C = 0;
				}
				else {
					setProb (i, p);
				}
			}
		}
		return list;
	}
	/**
	 * @param numPF
	 * @return
	 * create the specified number of frames and return an array list of frames
	 */
	public static ArrayList<frame> createFrames (int numPF) {
		ArrayList<frame> frames = new ArrayList<frame>();
		for (int i = 0; i < numPF; i++) {
			frame f = new frame ();
			frames.add (f);
		}
		return frames;
	}
	/**
	 * @param numVP
	 * @param pageSize
	 * @param curWord
	 * @param curProcess
	 * @return
	 * given the current word, return which page this word belongs in
	 */
	public static int  getPageNum (int numVP, int pageSize, int curWord, process curProcess) {
		int startAdd = 0;
		for (int i = 0; i < numVP; i++) {
			if (startAdd <= curWord && curWord < startAdd + pageSize) {
				return i;
			}
			else {
				startAdd += pageSize;
			}
		}
		return -1;
	}

	/**
	 * @param  curProcess
	 * @param  curWord    
	 * @param  frames     
	 * @param  pageNum    
	 * @param  time       
	 * @return            
	 * Given the params below, check to see if there is a page hit
	 */
	public static boolean isHit (process curProcess, int curWord, ArrayList<frame> frames, int pageNum, int time) {
		for (int i = 0; i < frames.size(); i++) {
			if (frames.get(i).valid == 1) {
				if (curProcess.num == frames.get(i).process.num && pageNum == frames.get(i).page) {
					frames.get(i).process = curProcess;
					frames.get(i).valid = 1;
					frames.get(i).lastTimeUsed = time;
					return true;
				}
			}
		}
		return false;
	}

/**
 * Checks to see if this process has completed. If so, store it in an array of done processes
 * @param  curProcess            
 * @param  ArrayList<process>done
 * @param  numRef                 
 * @return                       
 */
	public static boolean isDone (process curProcess, ArrayList<process>done,int numRef) {
		if (curProcess.nRef == numRef) {
			if (!done.contains (curProcess)) {
				done.add (curProcess);
				curProcess.done = true;
			}
			return true;
		}
		else {
			return false;
		}
	}
	
	/**
	 * Prints results once all the processes have completed
	 * @param done
	 */
	public static void printResults (ArrayList<process> done) {
		int totalFaults = 0;
		int totalResidency = 0;
		int totalEvictions = 0;
		System.out.println ("\n");
		for (int i = 0; i < done.size(); i++) {
			process curProcess = done.get(i);
			double avgResidency = (double) curProcess.residencyTime / curProcess.numEvicted;
			if (curProcess.numEvicted == 0) {
				System.out.println ("Process " + curProcess.num + " had " + curProcess.numFaults + " faults and " +
					" the average residency is undefined");
			}
			else {
				double avgRes = (double) curProcess.residencyTime / curProcess.numEvicted;
				System.out.println ("Process " + curProcess.num + " had " + curProcess.numFaults + " faults and " + avgRes
					 + " average residency");
			}
			totalFaults += curProcess.numFaults;
			totalResidency += curProcess.residencyTime;
			totalEvictions += curProcess.numEvicted;
		}
		if (totalEvictions == 0) {
			System.out.println ("The total number of faults is: " + totalFaults + " and the overall average residency is undefined");
		}
		else {
			double totalAvgRes = (double) totalResidency / totalEvictions; 
			System.out.println ("The total number of faults is: " + totalFaults + " and the overall average residency is " + totalAvgRes);
		}
		System.out.println ("\n");
	}

	/**
	 * Reads the list of random numbers (assuming the random-numbers file is in the same directory as this java file), and stores
	 * as an array list of integers
	 * @return [description]
	 */
	public static ArrayList<Integer> readRandomNum () {
		ArrayList<Integer> random = new ArrayList<Integer>();
		try {
			File file = new File ("random-numbers");
			Scanner sc = new Scanner (file);
			while (sc.hasNextInt()) {
					random.add(sc.nextInt());
			}
		}
		catch (FileNotFoundException ex) {
			System.out.println ("Unable to open random-numbers file. Please ensure this file exists. Exiting program");
			System.exit (1);
		}
		return random;
	}

	/**
	 * Running the LRU algorithm
	 * @param curProcess                  
	 * @param curWord                     
	 * @param ArrayList<frame>frames      
	 * @param time                        
	 * @param ArrayList<process>processes 
	 * @param pageNum                     
	 */
		public static void lru (process curProcess, int curWord, ArrayList<frame>frames, int time, ArrayList<process>processes, int pageNum) {
		int min = Integer.MAX_VALUE;
		int index = -1;
		//first check if free frame
		int freeFrame = freeFrame (frames);
		if (freeFrame != -1) {
			frame curFrame = frames.get(freeFrame);
			curFrame.process = curProcess;
			curFrame.valid = 1;
			curFrame.page = pageNum;
			curFrame.lastTimeUsed = time;
			curFrame.loadTime = time;
		}
		//no free frame, choose LRU page
		else {
			for (int i = 0; i < frames.size(); i++) {
				if (frames.get(i).lastTimeUsed <= min) {
					min = frames.get(i).lastTimeUsed;
					index = i;
				}
			}
			//get the page to be evicted & update its residency time and number of times it has been evicted
			//also update the frame with the new page
			frame curFrame = frames.get(index);
			process toBeEvicted = curFrame.process;
			toBeEvicted.residencyTime += (time - curFrame.loadTime);
			toBeEvicted.numEvicted += 1;
			curFrame.process = curProcess;
			curFrame.loadTime = time;
			curFrame.page = pageNum;
			curFrame.lastTimeUsed = time;
		}
	}

/**
 * Random page replacement algorithm
 * @param  curProcess                  
 * @param  curWord                     
 * @param  ArrayList<frame>frames      
 * @param  time                        
 * @param  ArrayList<process>processes 
 * @param  pageNum                     
 * @param  random                      
 * @param  randPtr                     
 * @return                             
 */
	public static int random (process curProcess, int curWord, ArrayList<frame>frames, int time, ArrayList<process>processes, int pageNum, int random, int randPtr) {
		int index = -1;
		//first check to see if there is a free frame
		int freeFrame = freeFrame(frames);
		//if no free frame, get the page to be evicted & update its residency time & number of times it has been evicted
		//then update the frame to the newly added page
		//returns an incremented random number pointer because a random reference has been used
		if (freeFrame == -1) {
			index = random % frames.size();
			frame curFrame = frames.get(index);
			process toBeEvicted = curFrame.process;
			curFrame.process = curProcess;
			toBeEvicted.residencyTime += (time - curFrame.loadTime);
			toBeEvicted.numEvicted += 1;
			curFrame.page = pageNum;
			curFrame.loadTime = time;
			return randPtr+1;
		}
		//else there is a free page, so update the frame to this current process
		//return a non-incremented random number pointer because a random reference has NOT been used
		else {
			frame curFrame = frames.get(freeFrame);
			curFrame.process = curProcess;
			curFrame.valid = 1;
			curFrame.page = pageNum;
			curFrame.loadTime = time;
			return randPtr;
		}
	}

	/**
	 * LIFO page repl
	 * @param curProcess                  [description]
	 * @param curWord                     [description]
	 * @param ArrayList<frame>frames      [description]
	 * @param time                        [description]
	 * @param ArrayList<process>processes [description]
	 * @param pageNum                     [description]
	 */
	public static void lifo (process curProcess, int curWord, ArrayList<frame>frames, int time, ArrayList<process>processes, int pageNum) {
		//first check to see if there is a free frame
		int index = -1;
		int freeFrame = freeFrame (frames);
		//if there is no free frame, evict the page that was most recently used & update its residency time & number of times it has been evicted
		//then update the frame to house the new page of this process
		if (freeFrame == -1) {
			int min = Integer.MIN_VALUE;
			for (int i = 0; i < frames.size(); i++) {
				if (frames.get(i).loadTime >= min) {
					min = frames.get(i).loadTime;
					index = i;
				}
			}
			frame curFrame = frames.get(index);
			process toBeEvicted = curFrame.process;
			curFrame.process = curProcess;
			toBeEvicted.residencyTime += (time - curFrame.loadTime);
			toBeEvicted.numEvicted += 1;
			curFrame.page = pageNum;
			curFrame.loadTime = time;
		}
		//else a free frame was found, so have the frame house the page of this process
		else {
			frame curFrame = frames.get(freeFrame);
			curFrame.process = curProcess;
			curFrame.valid = 1;
			curFrame.page = pageNum;
			curFrame.loadTime = time;
		}
	}

	/**
	 * Checks to see if there is a free frame in the frames array list
	 * @param  frames 
	 * @return        
	 */
	public static int freeFrame (ArrayList<frame> frames) {
		for (int i = frames.size()-1; i >= 0; i--) {
			if (frames.get(i).valid == 0) {
				return i;
			}
		}
		return -1;
	}
/**
 * Sets the probability for when jobMix = 4
 * @param i 
 * @param p 
 */
	public static void setProb (int i, process p) {
		if (i == 1) {
			p.A = .75;
			p.B = .25;
		}
		else if (i == 2) {
			p.A = .75;
			p.C = .25;
		}
		else if (i == 3) {
			p.A = .75;
			p.B = .125;
			p.C = .125;
		}
		else {
			p.A = .5;
			p.B = .125;
			p.C = .125;
		}
	}
	/**
	 * Checks to see if the command line arguments are valid. Otherwise exits program 
	 * @param args [description]
	 */
	public static void validInput (String [] args) {
		if (args.length < 6) {
			System.out.println ("\nNot enough arguments. Exiting program\n");
			System.exit (1);
		}
		else if (args.length > 6) {
			System.out.println ("\nToo many arguments. Exiting program\n");
			System.exit(1);
		}
		else if (!args[5].equals("lru") && !args[5].equals("lifo") && !args[5].equals("random")) {
			System.out.println ("\nInvalid page replacement algorithm. Please enter lru, lifo, or random. Exiting program\n");
			System.exit(1);
		}
	} 
}




