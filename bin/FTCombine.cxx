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

    for (int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "verbose") {
	verbose = true;
      } else {
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
    vector<CalibrationAnalysis> result (CombineAnalyses(info));
    
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
  cerr << "Usage: FTCombine <files, --ignore> --verbose" << endl;
}
