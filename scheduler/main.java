import java.io.*;
import java.util.*;
import java.util.regex.*;
public class main {
	public static void main (String [] args) {
		if (args.length < 1) {
			System.out.println ("Missing arguments. Run with proper arguments");
			System.exit(1);
		}
		try {
			int fileArg = 0;
			boolean verbose = false;
			if (args.length == 1) {
				fileArg = 0;
			}
			else if (args.length == 2) {
				fileArg = 1;
				verbose = true;
			}
			File file = new File (args[fileArg]);
			Scanner sc = new Scanner (file);
			int curRandNum = 0;
			int scCounter = 0;
			int arrCounter = 0;
			int procInfoCounter = 0;
			int numProc = 0;
			ArrayList<ArrayList<Integer>> procInfo = new ArrayList <ArrayList<Integer>> ();
			process [] arrProcess = null;
			while (sc.hasNext()) {
				//get number of processes
				if (scCounter == 0) {
					numProc = sc.nextInt();
					arrProcess = new process [numProc];
					scCounter++;
					for (int i = 0; i < numProc; i++) {
						procInfo.add (new ArrayList <Integer> ());
					}
					continue;
				}
				//extract one of the 4 processor numbers
				if (sc.hasNextInt()) {
					int i = sc.nextInt();
					procInfo.get(arrCounter).add(i);
				}
				else {
					String a = sc.next();
					if (a.contains("(")) {
						Pattern p = Pattern.compile ("\\d+");
						Matcher m = p.matcher(a);
						if (m.find()) {
							int i = Integer.parseInt(m.group());
							procInfo.get(arrCounter).add(i);
						}
					}
					else if (a.contains(")")) {
						Pattern p = Pattern.compile ("\\d+");
						Matcher m = p.matcher(a);
						if (m.find()) {
							int i = Integer.parseInt(m.group());
							procInfo.get(arrCounter).add(i);
						}
						arrCounter++;
					}
				}
			}//end while
			
			//read random numbers file, store in arraylist
			ArrayList<Integer> random = new ArrayList <Integer> ();
			file = new File ("random-numbers");
			sc = new Scanner (file);
			while (sc.hasNextInt()) {
				random.add(sc.nextInt());
			}
			curRandNum = 0;

			//create process objects and insert in array of processes 
			createProcess (arrProcess, numProc, procInfo);

			//create scheduler 
			scheduler s = new scheduler (random);

			//run scheduler
			runScheduler (arrProcess, s, verbose);
		}
		catch (FileNotFoundException ex) {
			System.out.println ("Unable to open file");
		}
		catch (IOException ex) {
			System.out.println ("Error reading file");
		}
	}

	public static void createProcess (process [] arrProcess, int numProc, ArrayList<ArrayList<Integer>> procInfo) {
		for (int i = 0; i < numProc; i++) {
			process p;
			int a = procInfo.get(i).get(0);
			int b = procInfo.get(i).get(1);
			int c = procInfo.get(i).get(2);
			int d = procInfo.get(i).get(3);
			p = new process (a,b,c,d);
			arrProcess[i] = p;
			p.rank = i;
		}
	}

	public static void runScheduler (process [] arrProcess, scheduler s, boolean verbose) {
		int q = 2;
		s.FCFS(arrProcess, verbose);
		reInit (arrProcess);
		s.RR(arrProcess, q, verbose);
		reInit (arrProcess);
		s.uni (arrProcess, verbose);
		reInit(arrProcess);
		s.SJF(arrProcess, verbose);
	}
	//clear reference variables of each process for the next algorithm
	public static void reInit (process [] arrProcess) {
		for (int i = 0; i < arrProcess.length; i++) {
			process cur = arrProcess[i];
			cur.nullify();
		}
	}
}