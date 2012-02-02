///
/// FTPlot
///
///  Read in the combo files and make a bunch of plots
///

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Plots.h"
#include "Combination/AtlasStyle.h"

#include <TFile.h>

#include <iostream>

using namespace std;
using namespace BTagCombination;

void usage(void);

int main (int argc, char **argv)
{
  try {
    // Parse the input arguments
    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);
    vector<CalibrationAnalysis> &calibs(info.Analyses);

    if (otherFlags.size() != 0) {
      usage();
      return 1;
    }

    //
    // Now generate the plots in the output file.
    //

    TFile *f = new TFile ("plots.root", "RECREATE");

    SetAtlasStyle();
    DumpPlots (f, calibs);

    f->Write();
    f->Close();

  } catch (exception &e) {
    cerr << "Error while doing the combination: " << e.what() << endl;
    return 1;
  }
  return 0;
}

void usage(void)
{
  cout << "FTPlot <std-cmd-line-argsw>" << endl;
}
