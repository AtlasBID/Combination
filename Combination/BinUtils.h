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

  // Get a list of all bins in an analysis.
  std::set<std::set<CalibrationBinBoundary> > listAllBins (const std::vector<CalibrationAnalysis> &analyses);

  // Remove a bin from all analyses...
  std::vector<CalibrationAnalysis> removeBin (const std::vector<CalibrationAnalysis> &analyses, const std::set<CalibrationBinBoundary> &binToRemove);
}

#endif
