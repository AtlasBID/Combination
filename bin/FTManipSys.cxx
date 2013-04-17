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

// Main program - run & control everything.
int main (int argc, char **argv)
{
  if (argc <= 1) {
    Usage();
    return 1;
  }

  try {
    // Parse the input arguments
    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);

    bool sawFlag = false;

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "check") {
      } else {
	cerr << "Unknown command line option --" << otherFlags[i] << endl;
	Usage();
	return 1;
      }
    }

    if (!sawFlag) {
      Usage();
      return 1;
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
  cout << "FTDump <file-list-and-options>" << endl;
  cout << "  ouputAna <ana>                      - The new analysis should be called this." << endl;
  cout << "                                        Applied to the below guys" << endl;
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
