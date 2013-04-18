//
// Given some operations and a set of input analyses, generate
// a new output analysis. Mostly, manipulate the systematic errors.
//
//  1) Add a new systematic error, whatever it is.
//  2) Take the central value differences and add it as a new sys error on output
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/BinBoundaryUtils.h"

#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace BTagCombination;

// Helper routines forward defined.
void Usage(void);

string eatArg (char **argv, int &index, const int maxArg)
{
  if (index == (maxArg-1)) 
    throw runtime_error ("Not enough arguments.");
  index++;
  return argv[index];
}

// Main program - run & control everything.
int main (int argc, char **argv)
{
  if (argc <= 1) {
    Usage();
    return 1;
  }

  try {
    // Parse the input args for commands
    vector<string> otherArgs;
    
    string outputAna, outputFlavor;
    string newsys;
    string newsysval;

    for (int i = 1; i < argc; i++) {
      string a(argv[i]);
      if (a == "outputAna") {
	outputAna = eatArg(argv, i, argc);
      } else if (a == "outputFlavor") {
	outputFlavor = eatArg(argv, i, argc);
      } else if (a == "addSysError") {
	newsys = eatArg(argv, i, argc);
	newsysval = eatArg(argv, i, argc);
      } else {
	otherArgs.push_back(a);
      }
    }

    // Check the arguments
    if (outputAna == "" && outputFlavor == "") {
      cout << "outputAna or outputFlavor must be specified" << endl;
      Usage();
      return 1;
    }

    if (newsysval == "") {
      cout << "No options for systematic error manipulation!" << endl;
      Usage();
      return 1;
    }

    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs (otherArgs, info, otherFlags);

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "check") {
      } else {
	cerr << "Unknown command line option --" << otherFlags[i] << endl;
	Usage();
	return 1;
      }
    }

    //
    // If we are doing the add a sys, then just loop through
    //

    vector<CalibrationAnalysis> results;
    if (newsys.size() != 0) {
      if (info.Analyses.size() != 1)
	throw new runtime_error ("Can only add a systematice error to a single analysis!");
      
      CalibrationAnalysis newAna (info.Analyses[0]);
      
      // Parse the error size
      istringstream inParam (newsysval);
      double amount;
      char c;
      inParam >> amount >> c;
      bool isPercent = c == '%';

      // Loop through all the bins and add what we need to add.
      for (size_t ib = 0; ib < newAna.bins.size(); ib++) {
	CalibrationBin &b(newAna.bins[ib]);
	SystematicError err;
	err.name = newsys;
	err.value = amount;
	if (isPercent)
	  err.value *= b.centralValue / 100.0;
	b.systematicErrors.push_back(err);
      }
      

      results.push_back(newAna);

    } else {
    }
    
    //
    // Get the results out
    //

    for (size_t i = 0; i < results.size(); i++) {
      if (outputAna.size() > 0)
	results[i].name = outputAna;
      if (outputFlavor.size() > 0)
	results[i].flavor = outputFlavor;
      cout << results[i];
    }

  } catch (exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }


    //
    // Get the analyses and go from there.
    //

  //const vector<CalibrationAnalysis> &calibs(info.Analyses);

  return 0;
}

void Usage(void)
{
  cout << "FTManipSys <file-list-and-options>" << endl;
  cout << "  ouputAna <ana>                      - The new analysis should be called this." << endl;
  cout << "                                        Applied to the below guys" << endl;
  cout << "  outputFlavor <flavor>               - Change to this flavor for output" << endl;
  cout << "  addSysError <newsys> <value>        - Add a systematic error name, and value." << endl;
  cout << "                                        Value has a % at the end if it is fractional." << endl;
  cout << "  calcRelDiff <ana1> <ana2> <newsys>  - Calculate the relative difference between" << endl;
  cout << "                                        ana1 and ana2, make it the new sys error" << endl;
  cout << "                                        ana1 is used as the template with the new analysis" << endl;
  cout << "  setUncorrelated <name>              - Set a particular systematic as uncorrelated" << endl;
  cout << "                                        on output analysis" << endl;
  cout << endl;
  cout << " Command fails if duplicate analyses would be created by the operations" << endl;
  cout << " Only will combine common taggers, operating points, and jet alg, etc." << endl;
}
