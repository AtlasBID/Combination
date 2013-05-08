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
#include <cmath>
#include <fstream>

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

// Return an analysis from a list.
bool getAnalysis (CalibrationAnalysis &foundAna, const string &aname, const vector<CalibrationAnalysis> &list)
{
  for (size_t i = 0; i < list.size(); i++) {
    if (list[i].name == aname) {
      foundAna = list[i];
      return true;
    }
  }
  return false;
}

bool getBin (CalibrationBin &bin, const CalibrationBin &proto, const vector<CalibrationBin> &list)
{
  for (size_t i = 0; i < list.size(); i++) {
    if (proto.binSpec == list[i].binSpec) {
      bin = list[i];
      return true;
    }
  }
  return false;
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
    string newsys, newsysval;
    string relDifAna1, relDifAna2, relDifSys;
    string outputFilename;

    for (int i = 1; i < argc; i++) {
      string a(argv[i]);
      if (a == "outputAna") {
	outputAna = eatArg(argv, i, argc);
      } else if (a == "outputFlavor") {
	outputFlavor = eatArg(argv, i, argc);
      } else if (a == "addSysError") {
	newsys = eatArg(argv, i, argc);
	newsysval = eatArg(argv, i, argc);
      } else if (a == "calcRelDiff") {
	relDifAna1 = eatArg(argv, i, argc);
	relDifAna2 = eatArg(argv, i, argc);
	relDifSys = eatArg(argv, i, argc);
      } else if (a == "output") {
	outputFilename = eatArg(argv, i, argc);
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

    if (newsysval == "" && relDifAna1 == "") {
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
    // Next, do the manipulation
    //

    vector<CalibrationAnalysis> results;

    // New systematic that is added flat out.

    if (newsys.size() != 0) {
      // Parse the error size
      istringstream inParam (newsysval);
      double amount;
      char c;
      inParam >> amount >> c;
      bool isPercent = c == '%';

      // Do all the analyses
      for (size_t i = 0; i < info.Analyses.size(); i++) {
	CalibrationAnalysis newAna (info.Analyses[i]);
      
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
      }

    } else if (relDifAna1.size() > 0) {
      // Taking two guys, and in each common bin, using the difference as a new sys error
      map<string, vector<CalibrationAnalysis> > splitAnas (BinAnalysesByJetTagFlavOp(info.Analyses));
      for (map<string, vector<CalibrationAnalysis> >::const_iterator i_alist = splitAnas.begin(); i_alist != splitAnas.end(); i_alist++) {
	CalibrationAnalysis a1, a2;
	if (getAnalysis(a1, relDifAna1, i_alist->second) && getAnalysis(a2, relDifAna2, i_alist->second)) {
	  bool good = true;
	  for (size_t ib = 0; ib < a1.bins.size(); ib++) {
	    CalibrationBin otherBin;
	    if (!getBin(otherBin, a1.bins[ib], a2.bins)) {
	      good = false;
	    } else {
	      SystematicError err;
	      err.name = relDifSys;
	      err.value = fabs(a1.bins[ib].centralValue - otherBin.centralValue);
	      a1.bins[ib].systematicErrors.push_back(err);
	    }
	  }
	  if (good)
	    results.push_back(a1);
	}
      }
      
    }
    
    //
    // Get the results out
    //

    ostream *output (&cout);
    ofstream *outputFile = 0;
    if (outputFilename.size() > 0) {
      outputFile = new ofstream(outputFilename.c_str());
      output = outputFile;
    }

    for (size_t i = 0; i < results.size(); i++) {
      if (outputAna.size() > 0)
	results[i].name = outputAna;
      if (outputFlavor.size() > 0)
	results[i].flavor = outputFlavor;
      (*output) << results[i];
    }

    if (outputFile != 0) {
      outputFile->close();
    }

  } catch (exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }

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
  cout << "  output <name>                       - Write the result to the output filename instead of stdout" << endl;
  cout << endl;
  cout << " Command fails if duplicate analyses would be created by the operations" << endl;
  cout << " Only will combine common taggers, operating points, and jet alg, etc." << endl;
}
