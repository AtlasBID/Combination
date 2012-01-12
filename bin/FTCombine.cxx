///
/// FTCombine
///
///  Combine several different measurements
///

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Combiner.h"

#include <iostream>
using namespace std;
using namespace BTagCombination;

void usage (void);

int main (int argc, char **argv)
{
  try {
    // Parse the input arguments
    vector<CalibrationAnalysis> calibs;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, calibs, otherFlags);

    if (otherFlags.size() != 0) {
      usage();
      return 1;
    }

    // Now that we have the calibrations, just combine them!
    vector<CalibrationAnalysis> result (CombineAnalyses(calibs));
    
    // Dump them out - this will get fixed, obviously
    for (unsigned int i = 0; i < result.size(); i++) {
      cout << result[i] << endl;
    }

  } catch (exception &e) {
    cerr << "Error while doing the combination: " << e.what() << endl;
    return 1;
  }
  return 0;
}

void usage (void)
{
  cerr << "Usage: FTCombine <files, --ignore>" << endl;
}
