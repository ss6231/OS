import java.util.ArrayList;
public class process {
	int num;
	int numRefLeft;
	boolean goneBefore = false;
	int jobMix;
	int numFaults;
	int nRef;
	int numEvicted;
	int numLoaded;
	int oldRef;
	int nextRef;
	int residencyTime;
	boolean done;

 double A = 0;
 double B = 0;
 double C = 0;

	public process ( int num, int numRefLeft, int jobMix) {
		this.num = num;
		this.numRefLeft = numRefLeft;
		this.jobMix = jobMix;
		this.nRef = nRef;
	}

	/**
	 * Returns the value of the next memory reference. The last case (case 4, ie: the last else statement here) returns a -1 so that the main
	 * program can update the random number variable accordingly 
	 * @param  w          [description]
	 * @param  s          [description]
	 * @param  curRandPtr [description]
	 * @param  random     [description]
	 * @return            [description]
	 */
	int getReference (int w, int s, int curRandPtr, ArrayList<Integer> random) {
		double y = random.get(curRandPtr) /  (Integer.MAX_VALUE + 1d);
		if (y < this.A) {
			return (w + 1 + s) % s;
		}
		else if (y < this.A + this.B) {
			return (w - 5 + s) % s;
		}
		else if (y < this.A + this.B + this.C) {
			return (w + 4 + s) % s;
		}
		else {
			return -1;
		}
	}
}