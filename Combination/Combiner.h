///
/// Combiner.h
///
///  Code to combine results
///

#ifndef COMBINATION_COMBINER
#define COMBINATION_COMBINER

#include "Combination/Parser.h"
#include <set>

namespace BTagCombination
{

  // Given a list of single bins, return a combined single bin
  // Reuses much of internal infrastructure, so good for testing, but
  // not likley to be used for real.
  CalibrationBin CombineBin (std::vector<CalibrationBin> &bins, const std::string &fitName = "");

  // Given a list of analyses, combine them all at once!
  CalibrationAnalysis CombineSimilarAnalyses (std::vector<CalibrationAnalysis> &anas);

  // Type of combination
  enum CombinationType {
    kCombineByFullAnalysis, // Combine the whole analysis, doing cross-bin correlations
    kCombineBySingleBin // Combine each bin seperately from all other bins
  };

  // Given a list of analses (diff jet alg, dif tags, dif, etc.), with bins all equal on boundaries,
  // combine them and return the total new combined analysis.
  std::vector<CalibrationAnalysis> CombineAnalyses (const CalibrationInfo &info, bool verbose = true,
						    CombinationType combineType = kCombineByFullAnalysis);

  // Given a set of template bins, force the analysis into those bins. Bins are combined - they can't
  // be split. Further source bins must fully cover the template bins - no gaps. runtime_error is
  // thrown if any of this dosen't work.
  // Fit is done seperately in each template bin.
  CalibrationAnalysis RebinAnalysis (const std::set<std::set<CalibrationBinBoundary> > &templateBinning,
				     const CalibrationAnalysis &ana);
}

#endif
