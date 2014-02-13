//
// There is a fairly special D* calc that must be done given D* inputs and a b-tagging scale factor.
//
// Given these, this program will perform that calculation. Fabrizio Parodi supplied the calculations
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

namespace {
  string eatArg (char **argv, int &index, const int maxArg)
  {
    if (index == (maxArg-1)) 
      throw runtime_error ("Not enough arguments.");
    index++;
    return argv[index];
  }

  // Return an analysis from a list.
  bool getAnalysis (vector<CalibrationAnalysis> &foundAna, const string &aname, const vector<CalibrationAnalysis> &list)
  {
    bool found = false;
    for (size_t i = 0; i < list.size(); i++) {
      if (list[i].name == aname) {
	foundAna.push_back(list[i]);
	found = true;
      }
    }
    return found;
  }

  double GetSysError (CalibrationBin &b, const string &name)
  {
    for (size_t e = 0; e < b.systematicErrors.size(); e++) {
      if (b.systematicErrors[e].name == name)
	return b.systematicErrors[e].value;
    }
    cerr << "Unable to find systematic error " << name << " in bin " << OPBinName(b) << endl;
    throw runtime_error ("Unable to find systematic error");
  }

  void UpdateSysError (CalibrationBin &b, const string &name, double newValue)
  {
    for (size_t e = 0; e < b.systematicErrors.size(); e++) {
      if (b.systematicErrors[e].name == name) {
	b.systematicErrors[e].value = newValue;
	return;
      }
    }

    // Not already there. Add.

    SystematicError err;
    err.name = name;
    err.value = newValue;
    b.systematicErrors.push_back(err);
    return;
  }

  double CalcFullError (const CalibrationBin &b)
  {
    double e2 = b.centralValueStatisticalError * b.centralValueStatisticalError;
    for (size_t e = 0; e < b.systematicErrors.size(); e++) {
      e2 += b.systematicErrors[e].value*b.systematicErrors[e].value;
    }
    return sqrt(e2);
  }

  // Do the calculation for one bin.
  void RescaleBin (CalibrationBin &dstar, const CalibrationBin &bSFb)
  {
    // Extract the info we need from the bSF bin.
    double bSF = bSFb.centralValue;
    double bSF_err = CalcFullError (bSFb);
  
    // And from the D* bin
    double cSF = dstar.centralValue;
    double errstat_cSF = dstar.centralValueStatisticalError;
    double syst_bSF = GetSysError(dstar, "b SF");

    // Do the calculation
    double deltaSF = (bSF-1.0)/0.05*syst_bSF;
    double cSF_new = cSF + deltaSF;
    double syst_bSF_new = syst_bSF/0.05 * bSF_err;

    // And update the d* bin. Note that the statistical error does not
    // change.
    dstar.centralValue = cSF_new;
    UpdateSysError(dstar, "b SF", syst_bSF_new);
  }

  // Rescale each D* bin, one at a time.
  void RescaleBins(vector<CalibrationBin> &dstarBins, const vector<CalibrationBin> &bSFBins)
  {
    // Build lookup table to help us with next step.

    map<set<CalibrationBinBoundary>, CalibrationBin> bSFBinLookup;
    for (size_t i = 0; i < bSFBins.size(); i++) {
      const vector<CalibrationBinBoundary> &bs(bSFBins[i].binSpec);
      bSFBinLookup[set<CalibrationBinBoundary>(bs.begin(), bs.end())] = bSFBins[i];
    }

    // Loop through the D* bins, rescaling one at a time.
  
    for (size_t i = 0; i < dstarBins.size(); i++) {
      set<CalibrationBinBoundary> key (dstarBins[i].binSpec.begin(), dstarBins[i].binSpec.end());
      map<set<CalibrationBinBoundary>, CalibrationBin>::const_iterator i_bsfBin = bSFBinLookup.find(key);
      if (i_bsfBin == bSFBinLookup.end()) {
	cerr << "For bin " << OPBinName(dstarBins[i]) << " in D* template could not find matching bin in bSF:" << endl;
	for (size_t ib = 0; ib < bSFBins.size(); ib++) {
	  cerr << "  -> " << OPBinName(bSFBins[ib]) << endl;
	}
	cerr << "  ** Skipping bin" << endl;
      } else {
	RescaleBin(dstarBins[i], i_bsfBin->second);
      }
    }
  }

  string stringReplace (const string &sourceString, const string &pattern, const string &replacement)
  {
    size_t index = sourceString.find(pattern);
    if (index == string::npos)
      return sourceString;

    string result(sourceString.substr(0, index));
    result += replacement;
    result += sourceString.substr(index + pattern.size());

    return result;
  }
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
    
    string outputAnaPattern;
    string dStarAna;
    string outputFilename;

    for (int i = 1; i < argc; i++) {
      string a(argv[i]);
      if (a == "outputAna") {
	outputAnaPattern = eatArg(argv, i, argc);
      } else if (a == "DStarAna") {
	dStarAna = eatArg(argv, i, argc);
      } else if (a == "output") {
	outputFilename = eatArg(argv, i, argc);
      } else {
	otherArgs.push_back(a);
      }
    }

    // Check the arguments
    if (outputAnaPattern == "" || dStarAna == "") {
      cout << "Both outputAna and DStarAna must be specified" << endl;
      Usage();
      return 1;
    }

    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs (otherArgs, info, otherFlags);

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      cerr << "Unknown command line option --" << otherFlags[i] << endl;
      Usage();
      return 1;
    }

    size_t outputPatternIndex = outputAnaPattern.find("<>");
    if (outputPatternIndex == string::npos) {
      cout << "outputAna '" << outputAnaPattern << "' must have a <> in it" << endl;
      Usage();
      return 1;
    }

    //
    // First, fetch the D* analysis out of the crowd.
    //

    vector<CalibrationAnalysis> dstarTemplateAna;
    if (!getAnalysis(dstarTemplateAna, dStarAna, info.Analyses)) {
      cerr << "Unable to find D* template analysis '" << dStarAna << "' in the list of input analyses." << endl;
      return 1;
    }

    //
    // Now, loop through all the bottom ones and use them to adjust the D* ones.
    //

    vector<CalibrationAnalysis> results;
    for (size_t i_ds = 0; i_ds < dstarTemplateAna.size(); i_ds++) {
      const CalibrationAnalysis &dstar(dstarTemplateAna[i_ds]);
      cout << "Template is " << OPFullName(dstar) << endl;

      for (size_t i = 0; i < info.Analyses.size(); i++) {
	const CalibrationAnalysis &a(info.Analyses[i]);
	if (a.name == dStarAna
	    || a.flavor != "bottom")
	  continue;

	if (a.tagger != dstar.tagger
	    || a.operatingPoint != dstar.operatingPoint
	    || a.jetAlgorithm != dstar.jetAlgorithm)
	  continue;

	// Create a new D* analysis that we will write out.

	CalibrationAnalysis r (dstar);
	r.name = stringReplace(outputAnaPattern, "<>", a.name);

	RescaleBins (r.bins, a.bins);

	cout << "  -> " << OPFullName(a) << endl;
	cout << "     " << OPFullName(r) << endl;

	results.push_back(r);
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
  cout << "  ouputAna <ana-pattern>              - The new analysis should be called this." << endl;
  cout << "                                        The pattern is 'Pre<>Post' where the <> will be replaced" << endl;
  cout << "                                        with the name of the bottom analysis." << endl;
  cout << "                                        Applied to the below guys" << endl;
  cout << "  DStarAna <ana>                      - The name of the raw D* analysis" << endl;
  cout << "  output <name>                       - Write the result to the output filename instead of stdout" << endl;
  cout << endl;
  cout << " outputAna and DStarAna must both be specified." << endl;
  cout << " Only new D* analyses are written out. Others along for the ride are never seen again." << endl;
  cout << " All bottom analyses will be used to create new D*. The flavor will be altered to whatever the D* analysis is." << endl;
}
