///
/// BinUtils.h
///
/// Some utilities to deal with bins
///
#ifndef __BTagCombination__BinUtils__
#define __BTagCombination__BinUtils__


#include "Combination/Parser.h"

#include <set>
#include <vector>

namespace BTagCombination {

  // Get the list of bins from a single analysis
  std::set<std::set<CalibrationBinBoundary> > listAnalysisBins(const CalibrationAnalysis &ana);

  // Get a list of all bins in an analysis.
  std::set<std::set<CalibrationBinBoundary> > listAllBins (const std::vector<CalibrationAnalysis> &analyses);

  // Remove a bin from all analyses...
  std::vector<CalibrationAnalysis> removeBin (const std::vector<CalibrationAnalysis> &analyses, const std::set<CalibrationBinBoundary> &binToRemove);

  // Remove all but this bin
  std::vector<CalibrationAnalysis> removeAllBinsButBin (const std::vector<CalibrationAnalysis> &analyses, const std::set<CalibrationBinBoundary> &binToNotRemove);

  // Get a list of all systematic errors
  std::set<std::string> listAllSysErrors(const std::vector<CalibrationAnalysis> &analyses);

  // Remove a sys error from all analyses...
  std::vector<CalibrationAnalysis> removeSysError(const std::vector<CalibrationAnalysis> &analyses, const std::string &sysErrorName);

  // Alter a sys error to be correlated
  std::vector<CalibrationAnalysis> makeSysErrorUncorrelated(const std::vector<CalibrationAnalysis> &analyses, const std::string &sysErrorName);
  
  // Find all bins in the list that contain a specified low edge value
  std::vector<CalibrationBin> find_bins_with_low_edge(const std::string &axis_name, const double axis_low_val, const std::vector<CalibrationBin> allbins);

  // Return the total systematic error (added in quad) that this bin has.
  double bin_sys (const CalibrationBin &bin);
}

#endif
