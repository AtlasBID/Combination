//
// A diagnostics program that will dump
// out and check the input files
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/BinBoundaryUtils.h"

#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace BTagCombination;

// Helper routines forward defined.
void Usage(void);
void DumpEverything (vector<CalibrationAnalysis> &calibs);
void CheckEverything (vector<CalibrationAnalysis> &calibs);
void PrintNames (vector<CalibrationAnalysis> &calibs);

// Main program - run & control everything.
int main (int argc, char **argv)
{
  if (argc <= 1) {
    Usage();
    return 1;
  }

  try {
    // Parse the input arguments
    vector<CalibrationAnalysis> calibs;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, calibs, otherFlags);

    bool doCheck = false;
    bool doDump = true;
    bool doNames = false;

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "check") {
	doCheck = true;
	doDump = false;
      } else if (otherFlags[i] == "names") {
	doDump = false;
	doNames = true;
      } else {
	cerr << "Unknown command line option --" << otherFlags[i] << endl;
	Usage();
	return 1;
      }
    }

    // Dump out a list of comma seperated values
    if (doDump)
      DumpEverything (calibs);

    // Check to see if there are overlapping bins
    if (doCheck)
      CheckEverything(calibs);

    if (doNames)
      PrintNames(calibs);

    // Check to see if the bin specifications are consistent.
    return 0;

  } catch (exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }

  return 0;
}

//
// Hold onto a single ana/bin - makes sorting and otherwise
// dealing with this a bit simpler in the code.
//
class holder
{
public:
  holder (CalibrationAnalysis &ana, CalibrationBin &bin)
    : _ana(ana), _bin(bin) {}
  
  inline string name() const {return OPFullName(_ana);}
  inline string binName() const {return OPBinName(_bin);}
  inline vector<string> sysErrorNames() const
  {
    vector<string> r;
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      r.push_back(_bin.systematicErrors[i].name);
    return r;
  }
  inline bool hasSysError(const string &name) const
  {
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      if (_bin.systematicErrors[i].name == name)
	return true;
    return false;
  }
  inline double sysError(const string &name) const
  {
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      if (_bin.systematicErrors[i].name == name)
	return _bin.systematicErrors[i].value;
    throw runtime_error (string("sys error '") + name + "' not known!");
  }
private:
  CalibrationAnalysis _ana;
  CalibrationBin _bin;
};

//
// Generate a comma seperated list of csv values.
//
void DumpEverything (vector<CalibrationAnalysis> &calibs)
{
    vector<holder> held;
    for (unsigned int i = 0; i < calibs.size(); i++)
      for (unsigned int b = 0; b < calibs[i].bins.size(); b++)
	held.push_back(holder(calibs[i], calibs[i].bins[b]));

    // Get a complete list of all systematic errors!
    set<string> allsyserrors;
    for (unsigned int i = 0; i < held.size(); i++) {
      vector<string> binerr (held[i].sysErrorNames());
      allsyserrors.insert(binerr.begin(), binerr.end());
    }

    // Now that we are parsed, dump a comma seperated output to stdout...
    // hopefully this can be c/p into a excel file for nicer formatting.

    // Line1: analysis name headers
    for (unsigned int i = 0; i < held.size(); i++) {
      cout << "," << held[i].name() << " " << held[i].binName();
    }
    cout << endl;

    // Do a line for everything now...
    for (set<string>::const_iterator i = allsyserrors.begin(); i != allsyserrors.end(); i++) {
      cout << *i;
      for (unsigned int h = 0; h < held.size(); h++) {
	cout << ",";
	if (held[h].hasSysError(*i))
	  cout << held[h].sysError(*i);
      }
      cout << endl;
    }
}

//
// Make sure there are no overlapping bins, etc. for each
// analysis.
//
void CheckEverything (vector<CalibrationAnalysis> &calibs)
{
  //
  // Calculating boundaries will make sure each analysis
  // has a fully consitent set of bin boundaries
  //

  vector<bin_boundaries> bb;
  for (unsigned int i = 0; i < calibs.size(); i++) {
    bb.push_back(calcBoundaries(calibs[i]));
  }
  
  //
  // Next we check different analysis have consistent bins.
  //
  
  checkForConsitentBoundaries(bb);
}

void PrintNames (vector<CalibrationAnalysis> &calibs)
{
  for (unsigned int i = 0; i < calibs.size(); i++) {
    for (unsigned int b = 0; b < calibs[i].bins.size(); b++) {
      cout << OPIgnoreFormat(calibs[i], calibs[i].bins[b]) << endl;
    }
  }
}

void Usage(void)
{
  cout << "FTDump <file-list-and-options>" << endl;
  cout << "  --check - check if the binning of the input is self consistent" << endl;
  cout << "  --names - print out the names used for the --ignore command of everything" << endl;
}
