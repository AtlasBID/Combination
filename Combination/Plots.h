///
/// Plots.h
///
///  Make plots of all of our results and inputs
///
#ifndef __COMBINATION_PLOTS__
#define __COMBINATION_PLOTS__

#include "Combination/Parser.h"

#include <TDirectory.h>
#include <vector>

namespace BTagCombination
{

  enum  GroupCriteria {
    gcByBin,
    gcByCalib,
    gcByCalibEff
  };

  enum PlotCollection {
    pcAll,
    pcEffOnly
  };

  /// Store plots in the directory given for all the analyses.
  void DumpPlots (TDirectory *outputDir, const std::vector<CalibrationAnalysis> &anas,
		  GroupCriteria gp, PlotCollection whatToPlot = pcAll);

}

#endif
