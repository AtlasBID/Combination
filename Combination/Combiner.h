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
  CalibrationBin CombineBin (std::vector<CalibrationBin> &bins);

  // Given a list of analyses, combine them all at once!
  CalibrationAnalysis CombineAnalyses (std::vector<CalibrationAnalysis> &anas);
}

#endif
