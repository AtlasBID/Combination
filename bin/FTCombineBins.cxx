//
// We want to rebin a current analysis. Use another analysis input file
// as the template for the rebinning.
//
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Combiner.h"

#include <RooMsgService.h>

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
    vector<string> otherArgs;
    
    // Parse the input args for commands
    string outputAna;
    string templateAna;
    string outputFilename;

    for (int i = 1; i < argc; i++) {
      string a(argv[i]);
      if (a == "outputAna") {
	outputAna = eatArg(argv, i, argc);
      } else if (a == "templateAna") {
	templateAna = eatArg(argv, i, argc);
      } else if (a == "output") {
	outputFilename = eatArg(argv, i, argc);
      } else {
	otherArgs.push_back(a);
      }
    }

    // Check the arguments
    if (outputAna == "" || templateAna == "") {
      cout << "Both the output analysis and template analysis must be specified" << endl;
      Usage();
      return 1;
    }

    // Now parse the rest of the command line arguments.

    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs (otherArgs, info, otherFlags);

    bool verbose = false;
    for (size_t i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "verbose") {
	verbose = true;
      } else {
	cout << "Unrecognized flag '" << otherFlags[i] << endl;
	Usage();
	return 1;
      }
    }

    // Turn off all those fitting messages!

    if (!verbose) {
      RooMsgService::instance().setSilentMode(true);
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    }

    //
    // Find the template analysis and split it out from the other analyses that we will be doing a refit on.
    //

    CalibrationAnalysis tAnalysis;
    if (!getAnalysis(tAnalysis, templateAna, info.Analyses)) {
      cout << "Unable to find analysis '" << templateAna << "' in the input list of analyses" << endl;
      return 1;
    }

    set<set<CalibrationBinBoundary> > templateBinning;
    for (size_t ib = 0; ib < tAnalysis.bins.size(); ib++) {
      const CalibrationBin &b(tAnalysis.bins[ib]);
      set<CalibrationBinBoundary> bbounds (b.binSpec.begin(), b.binSpec.end());
      templateBinning.insert(bbounds);
    }

    //
    // Now, rebin each analysis
    //

    vector<CalibrationAnalysis> results;
    set<string> rebinAnalysisNames;
    for (size_t i = 0; i < info.Analyses.size(); i++) {

      // Make sure we want to actuall refit this guy!

      if (info.Analyses[i].name == templateAna)
	continue;

      // Do the rebinning
      cout << "Rebinning analysis '" << info.Analyses[i].name << "'" << endl;
      CalibrationAnalysis r (RebinAnalysis(templateBinning, info.Analyses[i]));
      r.name = outputAna;

      // Is this a legal name - are we going to make a duplicate?
      string name = OPFullName(r);
      if (rebinAnalysisNames.find(name) != rebinAnalysisNames.end()) {
	cout << "Rebinning '" << info.Analyses[i].name << "' generated a duplicate analysis" << endl;
	cout << "  -> " << name << endl;
	return 1;
      }
      rebinAnalysisNames.insert(name);
      results.push_back(r);
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
  cout << "FTCombineBins <file-list-and-options>" << endl;
  cout << "  ouputAna <ana>                      - The rebined analysis should be called this. [required]" << endl;
  cout << "  templateAna <ana>                      - Name of the analysis to use as a template. There should be only one [required]" << endl;
  cout << "  output <fname>                      - Write results to an output file instead of stdout." << endl;
  cout << endl;
  cout << " All the other standard commands apply. Use them to window down to a particular analysis or flavor, etc." << endl;
  cout << " An attempt will be made to rebin all analyses except the template ones." << endl;
  cout << endl;
  cout << " Command fails if duplicate analyses would be created by the operations" << endl;
  cout << " Only will combine common taggers, operating points, and jet alg, etc." << endl;
}
