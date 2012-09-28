//
//  FTExploreFit
//
// Explore a fit - see if we can figure out why it behaves the way
// it does. Usually run on a bad fit.
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Combiner.h"

#include <RooMsgService.h>

using namespace std;
using namespace BTagCombination;

void usage(void);

int main (int argc, char **argv)
{
  // Parse input arguments
  CalibrationInfo allInfo;
  try {
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, allInfo, otherFlags);
  } catch (exception &e) {
    cerr << "Error parsing command line: " << e.what() << endl;
    usage();
    return 1;
  }

  //
  // Simple setups
  //

  RooMsgService::instance().setSilentMode(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

  // What we do can only be done by looking at one fit at a time. So we
  // have to split everything by flavor, tagger, op, and jet algorithm.

  typedef map<string, vector<CalibrationAnalysis> > t_anaMap;
  t_anaMap binnedAnalyses (BinAnalysesByJetTagFlavOp(allInfo.Analyses));

  for(t_anaMap::const_iterator i_ana = binnedAnalyses.begin(); i_ana != binnedAnalyses.end(); i_ana++) {

    // Make sure this is worth our time...

    if (i_ana->second.size() == 1) {
      cout << "Can't explore fits that have only one input" << endl;
      continue;
    }

    // Next, build up a new Calibration Info as the central guy - this will have just one fit. And then
    // get the complete results of the fit.

    CalibrationInfo centralInfo (allInfo);
    centralInfo.Analyses = i_ana->second;
    cout << "Doing central fit..." << endl;
    vector<CalibrationAnalysis> centralResult (CombineAnalyses(centralInfo, false));

    // Now, if we've been asked to remove a bin at a time.

    

  }

  return 0;
}

void usage(void)
{
  cout << "FTExploreFit <std-cmd-line-argsw>" << endl;
}
