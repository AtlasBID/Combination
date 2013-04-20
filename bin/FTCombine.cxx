///
/// FTCombine
///
///  Combine several different measurements
///

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Combiner.h"
#include <RooMsgService.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace BTagCombination;

void usage (void);

int main (int argc, char **argv)
{
  try {
    // Parse the input arguments
    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);

    bool verbose = false;
    string prefix = "";

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "verbose") {
	verbose = true;
      } else if (otherFlags[i].substr(0, 6) == "prefix") {
	prefix = otherFlags[i].substr(6);
      } else {
	cout << "Error: Unknnown flag: " << otherFlags[i] << endl;
	usage();
	return 1;
      }
    }

    // Turn off all those fitting messages!
    if (!verbose) {
      RooMsgService::instance().setSilentMode(true);
      RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    }

    // Now that we have the calibrations, just combine them!
    vector<CalibrationAnalysis> result;
    if (!info.BinByBin) {
      result = CombineAnalyses(info);
    } else {
      result = CombineAnalyses(info, true, kCombineBySingleBin);
    }
    
    if (prefix != "") {
      for (vector<CalibrationAnalysis>::iterator itr = result.begin(); itr != result.end(); itr++) {
	itr->name = prefix + itr->name;
      }
    }

    // Dump them out to an output file.
    ofstream out ("combined.txt");
    for (unsigned int i = 0; i < result.size(); i++) {
      out << result[i] << endl;
    }
    out.close();

  } catch (exception &e) {
    cerr << "Error while doing the combination: " << e.what() << endl;
    return 1;
  }
  return 0;
}

void usage (void)
{
  cerr << "Usage: FTCombine <files, --ignore> --verbose [--profile | --binbybin] --prefixXXX" << endl;
}
