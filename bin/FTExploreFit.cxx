//
//  FTExploreFit
//
// Explore a fit - see if we can figure out why it behaves the way
// it does. Usually run on a bad fit.
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Combiner.h"
#include "Combination/BinUtils.h"
#include "Combination/Plots.h"

#include <RooMsgService.h>
#include <TFile.h>

#include <algorithm>

using namespace std;
using namespace BTagCombination;

void usage(void);

namespace {

  // 
  // Dump plots into an output file.
  //
  void DumpPlotResults (TDirectory *outDir,
			const CalibrationInfo &info,
			const vector<CalibrationAnalysis> &r)
  {
    vector<CalibrationAnalysis> anas (info.Analyses);
    anas.insert(anas.end(), r.begin(), r.end());
    DumpPlots (outDir, anas);
  }

  string Normalize (const string &name)
  {
    string r (name);

    replace(r.begin(), r.end(), '.', '-');
    replace(r.begin(), r.end(), ':', '-');
    replace(r.begin(), r.end(), '/', '-');

    return r;
  }

}

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

  //
  // We will store the outputs in file.
  //

  TFile *outputPlots = TFile::Open("explore.root", "RECREATE");

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

    // Put each set of analyses in a seperate guy.
    TDirectory *outDir = outputPlots->mkdir(Normalize(i_ana->first).c_str());

    // Next, build up a new Calibration Info as the central guy - this will have just one fit. And then
    // get the complete results of the fit.

    CalibrationInfo centralInfo (allInfo);
    centralInfo.Analyses = i_ana->second;
    cout << "Doing central fit..." << endl;
    vector<CalibrationAnalysis> centralResult (CombineAnalyses(centralInfo, false));

    // We use the global chi2/ndof as the figure of merit.

    double centralChi2 = centralResult[0].metadata["gchi2"]/centralResult[0].metadata["gndof"];
    cout << "  chi2/ndof = " << centralChi2 << endl;

    // Sore the default in a single one.

    DumpPlotResults (outDir->mkdir("Default"), centralInfo, centralResult);

    // Now, if we've been asked to remove a bin at a time.

    // Get a list of the bins in these analyses.
    set<set<CalibrationBinBoundary> > allBins (listAllBins(centralInfo.Analyses));

    // Now, remove the bins one at a time
    for (set<set<CalibrationBinBoundary> >::const_iterator itr = allBins.begin(); itr != allBins.end(); itr++) {
      CalibrationInfo missingBinInfo (allInfo);
      missingBinInfo.Analyses = removeBin (missingBinInfo.Analyses, *itr);
      vector<CalibrationBinBoundary> tempBinInfo (itr->begin(), itr->end());
      cout << "Doing fit without bin " << OPBinName(tempBinInfo) << endl;
      vector<CalibrationAnalysis> missingBinResult (CombineAnalyses(missingBinInfo, false));

      // Print out the chi2
      double missingChi2 = missingBinResult[0].metadata["gchi2"]/missingBinResult[0].metadata["gndof"];
      cout << "  Missing bin chi2/ndof = " << missingChi2 << endl;

      // Generate plots.
      DumpPlotResults (outDir->mkdir(Normalize(OPBinName(tempBinInfo)).c_str()), missingBinInfo, missingBinResult);
    }

    // Now, do it by removing one sys error at a time.

    set<string> allSysErrors (listAllSysErrors(centralInfo.Analyses));
    for (set<string>::const_iterator itr = allSysErrors.begin(); itr != allSysErrors.end(); itr++) {
      CalibrationInfo missingSysInfo (allInfo);
      missingSysInfo.Analyses = removeSysError (missingSysInfo.Analyses, *itr);

      cout << "Doing fit wihtout sys error " << *itr << endl;
      vector<CalibrationAnalysis> missingSysResult (CombineAnalyses(missingSysInfo, false));

      double missingChi2 = missingSysResult[0].metadata["gchi2"]/missingSysResult[0].metadata["gndof"];
      cout << "  Missing bin chi2/ndof = " << missingChi2 << endl;

      DumpPlotResults (outDir->mkdir(Normalize(*itr).c_str()), missingSysInfo, missingSysResult);
    }
  }

  //
  // Make sure all plot get written out.
  //

  outputPlots->Write();
  outputPlots->Close();

  return 0;
}

void usage(void)
{
  cout << "FTExploreFit <std-cmd-line-argsw>" << endl;
}
