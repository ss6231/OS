public class process implements Comparable <process>{
	int arrival;
	int b;
	int totalCPU;
	int m;
	int CPUburst;
	int ioBurst;
	int timeLeft;
	String status;
	int turnaround;
	int totalCPUBurst;
	int rank;
	boolean getNewBurst = true;
	int finish;
	int waiting = 0;
	int io = 0;
	boolean SJF = false;
	int cycle = 0;

	public process (int a, int b, int c, int d) {
		arrival = a;
		this.b = b;
		totalCPU = c;
		m = d;
		status = "unstarted";
		timeLeft = totalCPU;
	}
	//modify compareto to prioritize SJF, arrival time, and rank in input
	public int compareTo (process other) {
		if (other.SJF && this.SJF) {
			int pri1 = this.totalCPU - this.cycle;
			int pri2 = other.totalCPU - other.cycle;
			if (pri1 < pri2) {
				return -1;
			}
			else if (pri1 == pri2) {
				if (this.arrival < other.arrival) {
					return -1;
				}
				else if (this.arrival == other.arrival) {
					if (this.rank < other.rank) {
						return -1;
					}
					else {
						return 1;
					}
				}
				else {
					return 1;
				}
			}
			else {
				return 1;
			}
		}
		else {
			if (this.arrival < other.arrival) {
				return -1;
			}
			else if (this.arrival == other.arrival) {
				if (this.rank < other.rank) { //this occurs first in arrProcess, so it has higher priority
					return -1;
				}
				else {
					return 1;
				}
			}
			else {
				return 1;
			}
		}
	}
	//clear up variables for next algorithm use
	public void nullify () {
		ioBurst = 0;
		timeLeft = totalCPU;
		status = "unstarted";
		turnaround = 0;
		totalCPUBurst = 0;
		getNewBurst = true;
		finish = 0;
		waiting = 0;
		io = 0;
		SJF = false;
	}
	public String toString () {
		String s = "(";
		s+=arrival + " " + b + " " + totalCPU + " " + m + ")";
	return s;
	}
}