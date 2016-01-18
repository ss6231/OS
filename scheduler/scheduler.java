import java.util.*;
public class scheduler {
	ArrayList<Integer> random;
	int timer;
	int currentRandNum;
	ArrayList<process> ready = new ArrayList<process> ();
	ArrayList<process> blocked = new ArrayList<process> ();
	ArrayList<process> unstarted = new ArrayList<process> ();
	ArrayList<process> done = new ArrayList<process> ();
	int maxFin;
	double sumCPUTime = 0;
	double avgwaiting = 0;
	double avgturnaround = 0;
	double numBlockedCycles = 0;

	public scheduler (ArrayList<Integer> random) {
		this.random = random;
	}
	public void FCFS (process [] arrProcess, boolean verbose) {
		timer = 0;
		currentRandNum = 0;
		ArrayList<process> temp = new ArrayList<process> ();
		System.out.println ("\nFCFS start");
		System.out.println ("==============\n");
		init(arrProcess, verbose);
		while (done.size() != arrProcess.length) {
			if (ready.size() != 0) {
				process cur = ready.remove(0);
				if (cur.getNewBurst) {
					cur.CPUburst = randomOS(cur.b); 
					cur.status = "running";
					if(cur.timeLeft < cur.CPUburst) {
						cur.CPUburst = cur.timeLeft;
					}
					cur.ioBurst = cur.CPUburst * cur.m;
				}
				cur.status = "running";
				//increment timer, update cpu bursts
				timer++;
				if (verbose) printStatus (arrProcess, timer);	
				else collectInfo (arrProcess);
				cur.timeLeft-=1;
				cur.CPUburst -= 1;
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				//current process is done
				if (cur.timeLeft == 0) {
					done.add(cur);
					cur.status = "terminated";
					cur.finish = timer;
					cur.turnaround = cur.finish - cur.arrival;
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				//current process has io burst, move to blocked
				else if (cur.CPUburst == 0) {
					cur.status = "blocked";
					blocked.add(cur);
					cur.getNewBurst = true;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				//cur must continue cpu burst, insert at front of ready to be used once again
				else if (cur.CPUburst != 0) {
					ready.add(0,cur);
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
			}
			else { //nothing in ready, either in running or blocked or unstarted
				timer++;
				if (verbose) printStatus (arrProcess, timer);
				else collectInfo (arrProcess);
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				if (temp.size() != 0) {
					resolveCurrent (null,temp);
				}
			}
		}
		System.out.println ("\nFirst come first served used");
		printFinal (arrProcess);
		summary(arrProcess);
		System.out.println ("\nFCFS end");
		System.out.println ("==============");
		clearUp();
	}
	public void RR (process [] arrProcess, int q, boolean verbose) {
		ArrayList<process> temp = new ArrayList<process>();
		timer = 0;
		currentRandNum = 0;
		System.out.println ("\nRR start");
		System.out.println ("==============\n");
		init (arrProcess, verbose);
		while (done.size() != arrProcess.length) {
			if (ready.size() != 0) {
				process cur = ready.remove(0);
				//get new burst only when coming out of blocked list
				if (cur.getNewBurst == true	) {
					cur.totalCPUBurst = randomOS(cur.b);
					cur.CPUburst = 2;
					cur.status = "running";
					if(cur.totalCPUBurst < cur.CPUburst) {
						cur.CPUburst = cur.totalCPUBurst;
					}
					cur.ioBurst = cur.totalCPUBurst * cur.m;
				}
				timer++;
				cur.status = "running";
				if (verbose) printStatus (arrProcess, timer);	
				else collectInfo (arrProcess);
				//update current process's cpu burst and remaining total cpu time
				cur.timeLeft-=1;
				cur.CPUburst -= 1;
				cur.totalCPUBurst-=1;
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				//process is done
				if (cur.timeLeft == 0) {
					done.add(cur);
					cur.status = "terminated";
					cur.finish = timer;
					cur.turnaround = cur.finish - cur.arrival;
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				//process cpu burst is done, move to blocked list for io burst
				else if (cur.totalCPUBurst == 0) {
					cur.status = "blocked";
					blocked.add(cur);
					cur.getNewBurst = true;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				else if (cur.CPUburst == 0) { //quantum is up, preempt into ready 
					cur.CPUburst = 2;
					if(cur.totalCPUBurst < cur.CPUburst) {
						cur.CPUburst = cur.totalCPUBurst;
					}
					cur.status = "ready";
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(cur, temp);
					}
					else {
						ready.add(cur);
					}
				}
				else if (cur.CPUburst != 0) { //keep running, quantum not finished
					ready.add (0,cur);
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
			}
			else { //nothing in ready, either running or in blocked or unstarted, still must update timer 
				timer++;	
				if (verbose) printStatus (arrProcess, timer);
				else collectInfo (arrProcess);
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				//here is problem
				resolveCurrent (null, temp);

				if (temp.size() != 0) {
					resolveCurrent (null,temp);
				}
			}
		}
		System.out.println ("\nRound robin used");
		printFinal (arrProcess);
		summary(arrProcess);
		System.out.println ("\nRR end");
		System.out.println ("==============\n");
		clearUp();

	}
	public void uni (process [] arrProcess, boolean verbose) {
		timer = 0;
		currentRandNum = 0;
		ArrayList<process> temp = new ArrayList<process> ();
		System.out.println ("\nUniprogram start");
		System.out.println ("==============\n");
		init(arrProcess, verbose);
		while (done.size() != arrProcess.length) {
			if (ready.size() != 0) {
				process cur = ready.remove(0);
				if (cur.getNewBurst) {
					cur.CPUburst = randomOS(cur.b); 
					cur.status = "running";
					if(cur.timeLeft < cur.CPUburst) {
						cur.CPUburst = cur.timeLeft;
					}
					cur.ioBurst = cur.CPUburst * cur.m;
				}
				cur.status = "running";
				timer++;	
				if (verbose) printStatus (arrProcess, timer);	
				else collectInfo (arrProcess);
				//update cpu status 
				cur.timeLeft-=1;
				cur.CPUburst -= 1;
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				//process done 
				if (cur.timeLeft == 0) {
					done.add(cur);
					cur.status = "terminated";
					cur.finish = timer;
					cur.turnaround = cur.finish - cur.arrival;
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				//this process's cpu burst is over, move to blocked list 
				else if (cur.CPUburst == 0) {
					cur.status = "blocked";
					blocked.add(cur);
					cur.getNewBurst = true;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
					//while this process is blocked, update timer and it's io burst
					//do not let any other ready process execute 
					while (cur.status == "blocked") {
						timer++;
						if (verbose) printStatus(arrProcess, timer);
						else collectInfo (arrProcess);
						cur.ioBurst-=1;
						if (cur.ioBurst == 0) {
							cur.status = "running";
							blocked.remove(cur);
							ready.add(0,cur);
						}
						updateUnstarted (unstarted, timer, temp);
						resolveReady(temp);
					}
				}
				//this process not done with current cpu burst
				//add to front of ready to be used again 
				else if (cur.CPUburst != 0) {
					ready.add(0,cur);
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
			}
			else { //nothing in ready, either blocked or unstarted, but update timer
				timer++;	
				if (verbose) printStatus (arrProcess, timer);
				else collectInfo (arrProcess);
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				resolveCurrent (null, temp);
				if (temp.size() != 0) {
					resolveCurrent (null,temp);
				}
			}
		}
		System.out.println ("\nUniprogram used");
		printFinal (arrProcess);
		summary(arrProcess);
		System.out.println ("\nUniprogram end");
		System.out.println ("==============\n");
		clearUp();
	}
	public void SJF (process [] arrProcess, boolean verbose) {
		timer = 0;
		currentRandNum = 0;
		ArrayList<process> temp = new ArrayList<process> ();
		System.out.println ("\nSJF start");
		System.out.println ("==============\n");
		init(arrProcess, verbose);
		isSJF(arrProcess);
		while (done.size() != arrProcess.length) {
			if (ready.size() != 0) {
				process cur = ready.remove(0);
				if (cur.getNewBurst) {
					cur.CPUburst = randomOS(cur.b); 
					cur.status = "running";
					if(cur.timeLeft < cur.CPUburst) {
						cur.CPUburst = cur.timeLeft;
					}
					cur.ioBurst = cur.CPUburst * cur.m;
					
				}
				cur.status = "running";
				timer++;
				cur.cycle++;
				if (verbose) printStatus (arrProcess, timer);	
				else collectInfo (arrProcess);
				//update process cpu time
				cur.timeLeft-=1;
				cur.CPUburst -= 1;
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				//current process terminated 
				if (cur.timeLeft == 0) {
					done.add(cur);
					cur.status = "terminated";
					cur.finish = timer;
					cur.turnaround = cur.finish - cur.arrival;
					cur.getNewBurst = false;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				//process's cpu burst is done, move to blocked list
				else if (cur.CPUburst == 0) {
					cur.status = "blocked";
					blocked.add(cur);
					cur.getNewBurst = true;
					if (temp.size() != 0) {
						resolveCurrent(null, temp);
					}
				}
				//this process's cpu burst is not done
				else if (cur.CPUburst != 0) {
					ready.add(0,cur);
					cur.getNewBurst = false;
				}
			}
			else { //nothing in ready, either in running or blocked or unstarted
				timer++;
				if (verbose) printStatus (arrProcess, timer);
				else collectInfo (arrProcess);
				updateIO (blocked, temp);
				updateUnstarted (unstarted, timer, temp);
				resolveReady(temp);
				if (temp.size() != 0) {
					resolveCurrent (null,temp);
				}
			}
		}
		System.out.println ("\nShortest job first used");
		printFinal (arrProcess);
		summary(arrProcess);
		System.out.println ("\nSJF end");
		System.out.println ("==============\n");
		clearUp();
	}
	public int randomOS (int u) {
		int x = random.get(currentRandNum);
		currentRandNum++;
		return 1 + (x % u);
	}
	//method to potentially move blocked processes to ready list
	public void updateIO (ArrayList<process> blocked, ArrayList<process>temp) {
		for (int i = 0; i < blocked.size(); i++) {
			process cur = blocked.get(i);
			cur.ioBurst-=1;
			
			if (cur.ioBurst == 0) {
				blocked.remove (i);
				temp.add(cur);
				cur.status = "ready";
				cur.getNewBurst = true;
				i--;
			}
		}
	}
	//method to potentially move unstarted processes to ready 
	public void updateUnstarted (ArrayList<process> unstarted, int timer,ArrayList<process> temp) {
		for (int i = 0; i < unstarted.size(); i++) {
			process cur = unstarted.get(i);
			if (cur.arrival == timer) {
				unstarted.remove(i);
				temp.add(cur);
				cur.status = "ready";
				cur.getNewBurst = true;
				i--;
			}
		}
	}
	public void printStatus (process [] arrProcess, int timer) {
		boolean hasBlocked = false;
		System.out.print ("Before cycle: " + timer);
		for (int i = 0; i < arrProcess.length; i++) {
			process cur = arrProcess[i];
			System.out.printf ("%12s", cur.status + " ");
				if (cur.status == "running") {
					System.out.printf (cur.CPUburst + "\t");
				}
				else if (cur.status == "blocked") {
					if (!hasBlocked) {
						numBlockedCycles++;
						hasBlocked = true;
					}
					System.out.print (cur.ioBurst + "\t");
					cur.io += 1;
				}
				else if (cur.status == "ready") {
					System.out.print (0 + "\t");
					cur.waiting += 1;
				}
				else {
					System.out.print (0 + "\t");
				}
		}
		System.out.println ();
	}
	//collect info about waittime, io time, and blocked cycles
	public void collectInfo (process [] arrProcess) {
		boolean hasBlocked = false;
		for (int i = 0; i < arrProcess.length; i++) {
			process cur = arrProcess[i];
			if (cur.status == "blocked") {
					if (!hasBlocked) {
						numBlockedCycles++;
						hasBlocked = true;
					}
				cur.io += 1;
			}
			else if (cur.status == "ready") {
				cur.waiting += 1;
			}
		}
	}
	//initialize processes into ready, blocked, or unstarted lists
	public void init (process [] arrProcess, boolean verbose) {
		ready = new ArrayList<process> ();
		blocked = new ArrayList<process> ();
		unstarted = new ArrayList<process> ();
		done = new ArrayList<process> ();
		System.out.println ("Before sorting: " + Arrays.toString(arrProcess));
		Arrays.sort(arrProcess);
		System.out.println ("After sorting: " + Arrays.toString (arrProcess));
		System.out.println ();
		if (verbose)printStatus (arrProcess, 0);
		else collectInfo (arrProcess);
		for (int i = 0; i < arrProcess.length; i++) {
			if (arrProcess[i].arrival == 0) {
				ready.add(arrProcess[i]);
				arrProcess[i].status = "ready";
			}
			else {
				unstarted.add(arrProcess[i]);
				arrProcess[i].status = "unstarted";
			}
		}
	}
	public void resolveReady (ArrayList<process> temp) {
		Collections.sort(temp);
	}
	//any processes that are added to ready list at the same time must be resolved
	//from temp, processes are added to ready and then sorted by priority according to the specific algorithm 
	public void resolveCurrent (process cur, ArrayList<process> temp) {
		if (cur == null) {
			for (int i = 0; i < temp.size(); i++) {
				ready.add(temp.get(i));
			}
			if (temp.size() != 0) {
				if (temp.get(0).SJF) {
					Collections.sort(ready);
				}
			}
		}
		else {
			temp.add(cur);
			Collections.sort(temp);
			for (int j = 0; j < temp.size(); j++) {
				ready.add(temp.get(j));
			}
			if (cur.SJF) {
				Collections.sort(ready);
			}
		}
		temp.clear();
	}
	//to specify if currently using SJF algorithm, which will change the Collections.sort() method
	public void isSJF (process [] arrProcess) {
		for (int i = 0; i < arrProcess.length; i++) {
			arrProcess[i].SJF = true;
		}
	}
	//clear up global vars
	public void clearUp () {
		done = null;
		ready = null;
		blocked = null;
		unstarted = null;
		maxFin = 0;
		sumCPUTime = 0;
		avgturnaround = 0;
		avgwaiting = 0;
		numBlockedCycles = 0;
	}
	public void printFinal (process[] arrProcess) {
		maxFin = arrProcess[0].finish;
		for (int i = 0; i < arrProcess.length; i++) {
			if (arrProcess[i].finish > maxFin) {
				maxFin = arrProcess[i].finish;
			}
			process cur = arrProcess[i];
			System.out.println ("Process: " + i);
			System.out.println ("(A,B,C,M) = (" + cur.arrival +", "+ cur.b +", "+ cur.totalCPU +", "+ cur.m + ")");
			sumCPUTime += cur.totalCPU;
			System.out.println ("Finishing time: " + cur.finish);
			System.out.println ("Turnaround time: " + cur.turnaround);
			avgturnaround += cur.turnaround;
			System.out.println ("I/O time: " + cur.io);
			System.out.println ("Waiting time: " + cur.waiting);
			avgwaiting += cur.waiting;
			System.out.println ();
		}
		avgwaiting = avgwaiting/arrProcess.length;
		avgturnaround = avgturnaround/arrProcess.length;
	}
	public void summary (process [] arrProcess) {
		System.out.println ("\nSummary Data:");
		System.out.println ("Finishing time: " + maxFin);
		System.out.println ("CPU Utilization: " + sumCPUTime/maxFin);
		System.out.println ("I/O Utilization: " + numBlockedCycles/maxFin);
		System.out.println ("Throughput: " + ((double)100*arrProcess.length/maxFin) + " processes per hundred cycles");
		System.out.println ("Average turnaround time: " + avgturnaround);
		System.out.println ("Average waiting time: " + avgwaiting + "\n");
	}
}

