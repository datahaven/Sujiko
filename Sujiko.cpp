// Sujiko Solver
// Adrian Dale 14/01/2014
// 
// This program will solve Sujiko puzzles.
// It also has an "Info Mode" parameter to display general information about this
// puzzle.
// Run it with -h to show help screen
// -i for Info Mode
// -s 000708000/22,15,17,21 to solve the puzzle
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>

using namespace std;

// Set used to store all possible non-symmetrical variants of the puzzle.
// A puzzle is held as the four bytes of an int.
// This makes it work nicely with a set without having to write a compare function
// May help performance slightly, too.
set<unsigned int> allPossiblePuzzles;

struct Solution
{
	int mValues[9];
	void DisplaySolution();
};

void Solution::DisplaySolution()
{
	cout << mValues[0] << " " << mValues[1] << " " << mValues[2] << endl;
	cout << mValues[3] << " " << mValues[4] << " " << mValues[5] << endl;
	cout << mValues[6] << " " << mValues[7] << " " << mValues[8] << endl;
}

class Sujiko
{
public:
	Sujiko() {};
	Sujiko(string s);
	bool isValidSolution(int *solution);
	int countSolutions();
	void allSolutions(vector<Solution> &results);
	void Display();
private:
	bool isSumCheckOK(int *solution);
	int mClues[9];
	int mCentres[4];
	static void DisplayGiven(int g);
};

// Parse puzzle in form 000000000/aa,bb,cc,dd
// No real error handling here, so take care!
Sujiko::Sujiko(string s)
{
	// Remove spaces from string
	s.erase(remove_if(s.begin(), s.end(), isspace), s.end());

	for (int i = 0; i < 9; ++i)
	{
		mClues[i] = s[i] - '0';
	}
	if (s[9] != '/')
	{
		cout << "Parse Error" << endl;
		return;
	}
	istringstream ss(s.substr(10));
	for (int i = 0; i < 4; ++i)
	{
		string digits;
		getline(ss, digits, ',');
		istringstream dss(digits);
		dss >> mCentres[i];
	}
}

void Sujiko::DisplayGiven(int g)
{
	if (g == 0)
		cout << " ";
	else
		cout << g;
}

// Dump out the puzzle in a roughly readable layout
void Sujiko::Display()
{
	DisplayGiven(mClues[0]); cout << "  ";
	DisplayGiven(mClues[1]); cout << "  ";
	DisplayGiven(mClues[2]); cout << endl;
	cout << " " << mCentres[0] << " " << mCentres[1] << endl;
	DisplayGiven(mClues[3]); cout << "  ";
	DisplayGiven(mClues[4]); cout << "  ";
	DisplayGiven(mClues[5]); cout << endl;
	cout << " " << mCentres[2] << " " << mCentres[3] << endl;
	DisplayGiven(mClues[6]); cout << "  ";
	DisplayGiven(mClues[7]); cout << "  ";
	DisplayGiven(mClues[8]); cout << endl;
}

// Exhaustively calculate all possible solutions for the puzzle
void Sujiko::allSolutions(vector<Solution> &results)
{
	results.clear();
	Solution guess;

	int start_guess[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	for (int i = 0; i < 9; ++i)
		guess.mValues[i] = start_guess[i];

	do
	{
		if (isValidSolution(guess.mValues))
		{
			results.push_back(guess);
		}
	} while (next_permutation(guess.mValues, guess.mValues + 9));
}

// Separated out this code in case I want to skip checking for non-matching clues.
// This improves performance if I don't need to check clues, such as when counting
// possible puzzles.
inline bool Sujiko::isSumCheckOK(int *solution)
{
	// Check against sums in centre circles
	if (solution[0] + solution[1] + solution[3] + solution[4] != mCentres[0]) return false;
	if (solution[1] + solution[2] + solution[4] + solution[5] != mCentres[1]) return false;
	if (solution[3] + solution[4] + solution[6] + solution[7] != mCentres[2]) return false;
	if (solution[4] + solution[5] + solution[7] + solution[8] != mCentres[3]) return false;
	return true;
}

inline bool Sujiko::isValidSolution(int *solution )
{
	// Make sure we don't contradict our given clues
	// NB This check makes the Info Mode take twice as long as it would if the 
	// check weren't present. It would be relatively simple to alter the code to
	// omit this check if necessary. I just didn't do it as it meant an extra
	// parameter getting passed through several functions, or having to be
	// stored in the puzzle.
	for (int i = 0; i < 9; ++i)
		if (mClues[i] != 0 && mClues[i] != solution[i]) return false;
	// Check against sums in centre circles
		return isSumCheckOK(solution);
}

int Sujiko::countSolutions()
{
	vector<Solution> solutions;
	allSolutions(solutions);
	return solutions.size();
}

void Info()
{
	// How many possible combinations of centre are there?
	// This number counts each symmetrical layout only once.
	// This page http://simomaths.wordpress.com/2013/01/13/burnsides-lemma-and-polya-enumeration-theorem-1/
	// suggests there should be n(n+1)(n^2+n+2)/8 where n is 21 => 26796
	// NB: 10,14,19,22 (1 solution) is a different puzzle from 14,10,19,22 which has no solutions.
	cout << "Number of ways to layout a puzzle = " << allPossiblePuzzles.size() << endl;

	int count = 0;
	int bestSolCount = 0;
	int singleSolCount = 0;
	int anySolCount = 0;

	cout << "Solving all possible puzzles..." << endl;

	time_t startTime;
	time(&startTime);

	// To count all puzzles including symmetrical versions there are 21^4 = 194481 possiblities,
	// which will take roughly seven times longer to count up.
	for (auto it = allPossiblePuzzles.begin(); it != allPossiblePuzzles.end(); ++it)
	{
		unsigned int puzDef = *it;
		++count;
		
		// Convert our puzzle to a string so we can initialise the Sujiko object
		ostringstream puzDefStr;
		puzDefStr << "000000000/"
			<< (puzDef & 0xff) << ","
			<< ((puzDef >> 8) & 0xff) << ","
			<< ((puzDef >> 16) & 0xff) << ","
			<< ((puzDef >> 24) & 0xff);

		Sujiko puz(puzDefStr.str());

		int solCount = puz.countSolutions();
		if (solCount > 0)
		{
			++anySolCount;
			cout << puzDefStr.str() << " has " << solCount << " solutions" << endl;

			if (solCount > bestSolCount) bestSolCount = solCount;
			if (solCount == 1) ++singleSolCount;
		}
		//if (count % 100 == 0) cout << "Combinations checked so far = " << count << endl;
	}
	
	time_t endTime;
	time(&endTime);

	cout << "Processing Time = " << difftime(endTime, startTime) << " seconds" << endl;
	cout << "Number of potential puzzles = " << count << endl; // 26796
	cout << "Most solutions to a single puzzle = " << bestSolCount << endl;
	cout << "Number of single solution puzzles = " << singleSolCount << endl;
	cout << "Number of solvable puzzles = " << anySolCount << endl;
}

bool isTransformationKnown(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	unsigned int puzDef = a + (b << 8) + (c << 16) + (d << 24);
	return allPossiblePuzzles.find(puzDef) != allPossiblePuzzles.end();
}

// Generate all possible puzzle layouts.
// All this does is try out every single possible combination of numbers from 10
// to 30 to make up the four centre circled numbers.
// All symmetrical transformations of the combination are checked to see if we've already seen
// this variant.
// Note that 10 = 1+2+3+4 and 30 = 6+7+8+9, hence the minimum and maximum possible
// values for the sums of four squares containing unique digits from 1-9
void GenerateAllPossiblePuzzles()
{
	allPossiblePuzzles.clear();

	for (unsigned int a = 10; a <= 30; ++a)
	for (unsigned int b = 10; b <= 30; ++b)
	for (unsigned int c = 10; c <= 30; ++c)
	for (unsigned int d = 10; d <= 30; ++d)
	{
		if (isTransformationKnown(a,b,c,d) == false	&&
			isTransformationKnown(b,a,d,c) == false && // horiz reflection
			isTransformationKnown(c,d,a,b) == false && // vert reflection
			isTransformationKnown(a,c,b,d) == false && // diagonal reflections
			isTransformationKnown(d,b,c,a) == false &&
			isTransformationKnown(c,a,d,b) == false && // 90 degree rotations
			isTransformationKnown(b,d,a,c) == false &&
			isTransformationKnown(d,c,b,a) == false) // 180 degree rotation
		{
			unsigned int puzDef = a + (b << 8) + (c << 16) + (d << 24);
			allPossiblePuzzles.insert(puzDef);
		}
	}
}

int main(int argc, char* argv[])
{
	cout << "Sujiko Explorer - Adrian Dale 2014" << endl;
	string option = "";
	if (argc > 1)
	{
		option = argv[1];
	}
	
	if (option.compare("-s")==0 && argc > 2)
	{
		Sujiko puz(argv[2]);
		
		cout << "Solving Puzzle:" << endl;
		puz.Display();

		vector<Solution> solutions;
		puz.allSolutions(solutions);
		cout << "Found " << solutions.size() << " solutions:" << endl;
		for_each(solutions.begin(), solutions.end(), [&](Solution &s){
			s.DisplaySolution();
			cout << endl;
		});
	}
	else if (option.compare("-i") == 0)
	{
		cout << "General Info:" << endl;
		GenerateAllPossiblePuzzles();
		Info();
	}
	else if (option.compare("") == 0 || option.compare("-h")==0 || option.compare("-?") == 0 )
	{
		cout << "Options:" << endl;
		cout << "-s 123456789/aa,bb,cc,dd" << endl;
		cout << "   to solve a puzzle, where puzzle format is:" << endl;
		cout << "   1  2  3" << endl;
		cout << "    aa bb" << endl;
		cout << "   4  5  6" << endl;
		cout << "    cc dd" << endl;
		cout << "   7  8  9   (Specify 0 for blank cells)" << endl;
	}

	return 0;
}

