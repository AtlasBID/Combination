//
// Copy input CalibrationAnalyses to the output - but only the defaults.
//

#include "Combination/Parser.h"
#include "Combination/CDIConverter.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/CalibrationDataModelStreams.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

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

  // Very simple wild-card matching
  bool wCompare (const string &s1, const string &s2)
  {
    if (s1 == "*" || s2 == "*")
      return true;
    return s1 == s2;
  }

  // Match with some basic wildcard info
  bool isAMatch (const vector<DefaultAnalysis> &defaults, const CalibrationAnalysis &ana)
  {
    for (unsigned int i = 0; i < defaults.size(); i++) {
      const DefaultAnalysis &d(defaults[i]);
      if (wCompare(ana.jetAlgorithm, d.jetAlgorithm)
	  && wCompare(ana.flavor, d.flavor)
	  && wCompare(ana.tagger, d.tagger)
	  && wCompare(ana.operatingPoint, d.operatingPoint)
	  && wCompare(ana.name, d.name)
	  )
	return true;
    }
    return false;
  }

}


int main(int argc, char **argv)
{
  vector<string> otherArgs;
    
  // Parse the input args for commands
  string outputFile ("");

  for (int i = 1; i < argc; i++) {
    string a(argv[i]);
    if (a == "output") {
      outputFile = eatArg(argv, i, argc);
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
    return 1;
  }

  vector<CalibrationAnalysis> defaultCalibrations;
  const vector<CalibrationAnalysis> &calibs(info.Analyses);
  for (unsigned int i = 0; i < calibs.size(); i++) {
    if (isAMatch(info.Defaults, calibs[i])) {
      CalibrationAnalysis def (calibs[i]);
      def.name = "default";
      defaultCalibrations.push_back(def);
    }
  }

  ostream *output (&cout);
  if (outputFile.size() > 0) {
    output = new ofstream(outputFile.c_str());
  }
  
  for (unsigned int i = 0; i < defaultCalibrations.size(); i++) {
    (*output) << defaultCalibrations[i] << endl;
  }

}
