///
/// FTPlot
///
///  Read in the combo files and make a bunch of plots
///

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Plots.h"
#include "Combination/AtlasStyle.h"

#include "TApplication.h"

#include <TFile.h>

#include <iostream>

using namespace std;
using namespace BTagCombination;

void usage(void);

#ifdef _MSC_VER
#define UNUSED
#else
#define UNUSED __attribute__((unused))
#endif

int main (int argc, char **argv)
{
  try {
    // Init the application so that all plotting infrastructure is initialized.
    // On Windows if this is missing it will cause a rather ugly crash.
    int myArgc = 1;
    char* myArgv[1];
    myArgv[0] = argv[0];
    TApplication *a UNUSED = new TApplication("FTPlot", &myArgc, myArgv);

    // Parse the input arguments
    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);
    vector<CalibrationAnalysis> &calibs(info.Analyses);

    GroupCriteria grouping = gcByBin;
    PlotCollection whatToPlot = pcAll;
    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      string arg(otherFlags[i]);
      if (arg == "ByBin") {
	grouping = gcByBin;
      } else if (arg == "ByCalib") {
	grouping = gcByCalib;
      } else if (arg == "ByCalibEff") {
	grouping = gcByCalibEff;
      } else if (arg == "ByCalibTaggerJet") {
	grouping = gcByCalibTaggerJet;
      } else if (arg == "EffOnly") {
	whatToPlot = pcEffOnly;
      } else {
	cout << "Unknown option '" << arg << "'." << endl;
	usage();
	return 1;
      }
    }

    //
    // Now generate the plots in the output file.
    //

    TFile *f = new TFile ("plots.root", "RECREATE");

    SetAtlasStyle();
    DumpPlots (f, calibs, grouping, whatToPlot);

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
