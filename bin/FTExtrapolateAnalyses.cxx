//
// Extrapolate all analyses given a set of extrapolation analyses names.
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/ExtrapolationTools.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace BTagCombination;

namespace {
  string eatArg (char **argv, int &index, const int maxArg)
  {
    if (index == (maxArg-1)) 
      throw runtime_error ("Not enough arguments.");
    index++;
    return argv[index];
  }

  void usage()
  {
    cout << "FTExtrapolateAnalysis --output <fname> --extrapolation <ana> <normal-inputs>" << endl;
    cout << "  output        File where the results will be written. Default to stdout." << endl;
    cout << "  extrapolation Name of an extrapolation analysis. Multiple can be specified. Required." << endl;
  }

  // Helper function to sort by flavor, op, tagger, jet type
  map<string, vector<CalibrationAnalysis> > groupAnaByType (const vector<CalibrationAnalysis> &anas)
  {
    map<string, vector<CalibrationAnalysis> > results;
    for (vector<CalibrationAnalysis>::const_iterator itr = anas.begin(); itr != anas.end(); itr++) {
      string name (OPByFlavorTaggerOp(*itr));
      cout << "Partitioning by " << name << endl;
      results[name].push_back(*itr);
    }
    return results;
  }
}

int main(int argc, char **argv)
{
  vector<string> otherArgs;
    
  // Parse the input args for commands
  string outputFile ("");
  vector<string> extrapolationAnalyses;

  for (int i = 1; i < argc; i++) {
    string a(argv[i]);
    if (a == "--output") {
      outputFile = eatArg(argv, i, argc);
    } else if (a == "--extrapolation") {
      extrapolationAnalyses.push_back(eatArg(argv, i, argc));
    } else {
      otherArgs.push_back(a);
    }
  }

  CalibrationInfo info;
  try {
    vector<string> otherFlags;
    ParseOPInputArgs (otherArgs, info, otherFlags);
  } catch (exception &e) {
    cerr << "Error parsing input files: " << e.what() << endl;
    usage();
    return 1;
  }

  // Find the extrapolation analyses and seperate those out.
  vector<CalibrationAnalysis> extrapolationAnas;
  vector<CalibrationAnalysis> anas;
  for (vector<CalibrationAnalysis>::const_iterator itr = info.Analyses.begin(); itr != info.Analyses.end(); itr++) {
    if (find(extrapolationAnalyses.begin(), extrapolationAnalyses.end(), itr->name) != extrapolationAnalyses.end()) {
      extrapolationAnas.push_back(*itr);
    } else {
      anas.push_back(*itr);
    }
  }

  // Check inputs to make sure that we have the right size.
  if (extrapolationAnalyses.size() == 0
      && extrapolationAnas.size() > 0) {
    cout << "Unable to find all the extrapolation analyses" << endl;
    for (size_t i = 0; i < extrapolationAnas.size(); i++) {
      cout << "  Found in input " << extrapolationAnas[i].name << endl;
    }
    for (size_t i = 0; i < extrapolationAnalyses.size(); i++) {
      cout << "  Expected in input " << extrapolationAnalyses[i] << endl;
    }
    usage();
    return 1;
  }

  // Now do the extrapolation...
  map<string, vector<CalibrationAnalysis> > groupedExtrap (groupAnaByType(extrapolationAnas));
  map<string, vector<CalibrationAnalysis> > groupedAna (groupAnaByType(anas));
  vector<CalibrationAnalysis> results;

  for (map<string, vector<CalibrationAnalysis> >::const_iterator itr = groupedAna.begin(); itr != groupedAna.end(); itr++) {
    map<string, vector<CalibrationAnalysis> >::const_iterator e_itr = groupedExtrap.find(itr->first);
    if (e_itr == groupedExtrap.end()) {
      // Easy, no extrapolation, just copy.
      cout << "Didn't find it: " << itr->first << endl;
      for (vector<CalibrationAnalysis>::const_iterator a_itr = itr->second.begin(); a_itr != itr->second.end(); a_itr++) {
	results.push_back(*a_itr);
      }
    } else {
      // Do the extrapolation
      if (e_itr->second.size() > 1) {
	cout << "More than one extrapolated analysis to apply!" << endl;
	usage();
	return 1;
      }
      for (vector<CalibrationAnalysis>::const_iterator a_itr = itr->second.begin(); a_itr != itr->second.end(); a_itr++) {
	results.push_back(addExtrapolation(e_itr->second[0], *a_itr));
      }
    }
  }

  // Write otu the results.
  ostream *output (&cout);
  if (outputFile.size() > 0) {
    output = new ofstream(outputFile.c_str());
  }
  
  for (unsigned int i = 0; i < results.size(); i++) {
    (*output) << results[i] << endl;
  }

}
