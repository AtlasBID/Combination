///
/// Combiner.h
///
///  Code to combine results
///

#ifndef COMBINATION_COMBINER
#define COMBINATION_COMBINER

#include "Combination/Parser.h"

namespace BTagCombination
{

  // Given a list of single bins, return a combined single bin
  // Reuses much of internal infrastructure, so good for testing, but
  // not likley to be used for real.
  CalibrationBin CombineBin (std::vector<CalibrationBin> &bins, const std::string &fitName = "");

  // Given a list of analyses, combine them all at once!
  CalibrationAnalysis CombineSimilarAnalyses (std::vector<CalibrationAnalysis> &anas);

  // Given a list of analses (diff jet alg, dif tags, dif, etc.), with bins all equal on boundaries,
  // combine them and return the total new combined analysis.
  std::vector<CalibrationAnalysis> CombineAnalyses (const CalibrationInfo &info, bool verbose = true);
}

#endif
