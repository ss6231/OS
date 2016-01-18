import java.io.*;
import java.util.*;
import java.util.ArrayList;

public class ss6231_lab1 {
	public static void main (String [] args) {
		BufferedInputStream buffer = new BufferedInputStream (System.in);
		System.out.println ("\nPlease input modules. Then press enter followed by CTRL+D");
		Scanner input = new Scanner (buffer);
		firstPass (input, buffer);
	}

	public static void firstPass (Scanner input, BufferedInputStream buffer) {
		ArrayList <String> symbolList = new ArrayList<String> ();
		ArrayList <Integer> locList = new ArrayList<Integer> ();
		ArrayList<String> [] useSym = (ArrayList<String>[]) new ArrayList[1000];
		int useSymCtr = 0;
		for (int i = 0; i < useSym.length; i++) {
			useSym[i] = new ArrayList <String> ();
		}
		ArrayList <Integer> [] useAdd = (ArrayList<Integer>[]) new ArrayList[1000];
		int useAddCtr = 0;
		for (int i = 0; i < useAdd.length; i++) {
			useAdd[i] = new ArrayList <Integer> ();
		}
		ArrayList<Integer> [] pgmAdd = (ArrayList<Integer>[]) new ArrayList [500];
		int pgmAddCtr = 0;
		for (int i = 0; i < pgmAdd.length; i++) {
			pgmAdd[i] = new ArrayList <Integer> ();
		}
		HashMap <String,Integer> moduleBaseAddr = new HashMap<String,Integer> ();
		ArrayList<Integer> tempDef = new ArrayList<Integer> ();
		int count = 0;
		int numUse = 0;
		int numDef = 0;
		int pgmLines = 0;
		int offset = 0;
		int moduleSize = 0;
		boolean isDup = false;
		int index = -1;
		while (input.hasNext ()) {
			//first process the definition list
			if (count % 3 == 0 && input.hasNextInt()) {
				numDef = input.nextInt();
				for (int i = 0; i < numDef*2; i++) {
					if (input.hasNextInt()) {
							int loc = input.nextInt();
							if (isDup) {
								locList.remove (index); 
								locList.add(index,loc+offset);
								isDup = false;
							}
							else {
								locList.add(loc + offset);
							}
						}
					else {
						String sym = input.next();
						if (symbolList.contains(sym)) {
							System.out.println ("\n\nError: " + sym + " is multiply defined. All but last value ignored");
							isDup = true;
							index = symbolList.indexOf(sym);
						}
						else {
							symbolList.add(sym);
						}
					}
				}
				count++;
				tempDef.addAll (locList);
			} //done with definition list
			
			//now we process use list 
			else if ((count-1) % 3 == 0) {
				if (input.hasNextInt()) {
					numUse = input.nextInt();
					if (numUse == 0) {
						//do nothing
					}
					else { //there are at least 1 or more usages here
						int counter_for_usage = 0;
						int current = numUse;
						String curSym = "";
						if (numUse > 500) {
							System.out.println ("\nThere are too many usages to process. Program is exiting");
							System.out.println ("Please input a module that has 500 or fewer definitions");
							System.exit(0);
						}
						while (counter_for_usage < numUse) {
							if (input.hasNextInt()) {
								current = input.nextInt();
								if (current != -1) {
									if (useAdd[moduleSize].contains(current)) {
										useAdd[moduleSize].add(useAddCtr, current);
										useSym[moduleSize].add(useSymCtr, curSym);
										System.out.println ("\n\nError: Multiple symbols used in instruction. All but last reference ignored");
										int idex = useAdd[moduleSize].indexOf(current);
										useAdd[moduleSize].remove (idex);
										useSym[moduleSize].remove (idex);
									}
									else {
										useAdd[moduleSize].add(useAddCtr, current);
										useAddCtr++;
										if (curSym != "") {
											useSym[moduleSize].add(useSymCtr,curSym);
											useSymCtr++;
										}
									}
								}
							}
							else {
								current = -5;
								curSym = input.next();
							}
							if (current == -1) {
								counter_for_usage++;
							}
						}
					}
				}
				count++;
			} //done with use list

			//lastly, we process the program text
			else if ((count+1)% 3 == 0) {
				if (input.hasNextInt ()) {
					pgmLines = input.nextInt();
					moduleBaseAddr.put("M" + moduleSize, offset);
					offset += pgmLines;
				}
				if (pgmLines == 0) {
					//do nothing
				}
				else {
					if (pgmLines > 500) {
						System.out.println ("\nProgram size is too large to process. Program is exiting");
						System.out.println ("Please input a module that has 500 or fewer definitions");
						System.exit(0);
					}
					for (int i = 0; i < pgmLines; i++) {
						if (input.hasNextInt()) {
							pgmAdd[moduleSize].add(pgmAddCtr, input.nextInt());
							pgmAddCtr++;
						}
					}
				}
				//check if address def'n exceeds this module's size 
				for (int i = 0; i < tempDef.size(); i++) {
					Integer curDef = tempDef.get(i)-moduleBaseAddr.get("M"+moduleSize);
					if (curDef >= pgmLines) {
						System.out.println ("\nError: Definition of symbol " + symbolList.get(i)+ ", location " + tempDef.get(i)  +" exceeds size of module");
						System.out.println ("New definition is last word of module");
						Integer newAdd = (pgmLines-1)+moduleBaseAddr.get("M"+moduleSize);
						tempDef.set(i, newAdd);
						int idx = locList.indexOf (curDef+moduleBaseAddr.get("M"+moduleSize));
						locList.set(idx, newAdd);
					}
				}
				//check if address in use list exceeds size of module and if so, ignore this reference
				for (int i = 0; i < useAdd[moduleSize].size(); i++) {
					if (useAdd[moduleSize].get(i) >= pgmLines) {
						System.out.println ("\nError: address in use list exceeds size of module. Ignoring this use");
						useAdd[moduleSize].remove(i);
						System.out.println (useAdd[moduleSize].toString());
						break;
					}
				}
				count++;
				tempDef.clear();
				moduleSize++;
				useSymCtr = 0;
				useAddCtr = 0;
				pgmAddCtr = 0;
			} //done with program text
		}
		System.out.println("\n");

		//copy symbol arraylist and location arraylist into a hashmap
		HashMap <String,Integer> symTable = new HashMap <String,Integer> ();

		for (int i = 0; i < symbolList.size(); i++) {
			symTable.put (symbolList.get(i),locList.get(i));
		}

		System.out.println ("\nSymbol Table");
		for (int i = 0; i < symbolList.size();i++) {
			System.out.println (symbolList.get(i) + " = " + symTable.get(symbolList.get(i)));
		}
		ArrayList<Integer> validAdd = new ArrayList<Integer>();
		for (int i = 0; i < 300; i++) {
			validAdd.add(i);
		}
		resolveAddress (useSym, useAdd, pgmAdd,symTable, moduleSize, moduleBaseAddr, validAdd);

	}
	//useSym, useAdd, pgmAdd all were arraylists - must change .contains () method
	public static void resolveAddress (ArrayList<String> [] useSym, ArrayList<Integer> [] useAdd, 
		ArrayList<Integer> [] pgmAdd, HashMap <String, Integer> symTable, int moduleSize, 
		HashMap <String,Integer> modBaseAddr, ArrayList<Integer> validAdd) {
		
		ArrayList <Integer> memMap = new ArrayList<Integer> ();
		ArrayList <String> symbolsUsed = new ArrayList <String> ();
		int lastDig = 0;
		Integer current = 0;
		Integer address = 0;
		int opcode = 0;
		int moduleAddress = 0;
		for (int i = 0; i < moduleSize; i++) {
			for (int j = 0; j < pgmAdd[i].size(); j++) {
				current = pgmAdd[i].get(j); //current program address
				if (current == null) break; //finished with program text when we hit null
				opcode = current / 10000;
				lastDig = current % 10; // either 1, 2, 3, or 4 for address type
				address = (current % (opcode*10000))/10;
				
				if (useAdd[i].contains(j)) { //symbol was used in pgm list, must resolve external address, address ends in 4
					int index = useAdd[i].indexOf(j);
					String symb = useSym[i].get(index);
					address = symTable.get(symb);
					symbolsUsed.add(symb);

					if (address == null) {
						System.out.println ("\nError: symbol " +  symb + " is referenced but not defined. Using location 111");
						address = 111;
					}
					memMap.add (opcode * 1000 + address);
				}
				else { //symbol was not used in pgm list, address will end in either 1,2, or 3
					if (lastDig == 1) {
						memMap.add (opcode * 1000 + address);
					}
					else if (lastDig == 2) { //must check machine size
						if (address >= 300) {
							System.out.println ("\nError: absolute address exceeds size of machine. Replacing with largest available legal value");
							address = validAdd.remove(validAdd.size()-1);
						}
						memMap.add (opcode * 1000 + address);
					}
					else if (lastDig == 3) {
						if (address >= pgmAdd[i].size()) {
							System.out.println ("\nError: Relative address exceeds size of module. Largest module address used");
							address = modBaseAddr.get("M" + i);
							memMap.add((opcode * 1000) + address + j);
						}
						else {
							moduleAddress = modBaseAddr.get("M" + i);
							memMap.add((opcode * 1000) + address + moduleAddress);
						}
					}
				}
			} // end inner for
		} //end outer for
		System.out.println ("\nMemory Map");
		for (int i = 0; i < memMap.size(); i++) {
			System.out.println (i + ": " + memMap.get(i));
		}

		//test if symbol is defined but not used 
			for (Map.Entry<String,Integer> entry: symTable.entrySet()) {
		 		String currSym = entry.getKey();
		 		if (!symbolsUsed.contains(currSym)) {
		 			System.out.println ("\nWarning: " + currSym + " was defined but never used");
		 		}
		}

	}


}







