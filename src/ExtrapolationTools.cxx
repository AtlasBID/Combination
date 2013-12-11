// Extrapolation tools

#include "Combination/ExtrapolationTools.h"

namespace BTagCombination {
  ///
  /// Add the extrapolated data, after rescaling, to the current analysis.
  ///
  CalibrationAnalysis addExtrapolation (const CalibrationAnalysis &extrapolated,
					const CalibrationAnalysis &ana)
  {
    CalibrationAnalysis r(ana);
    return r;
  }

}
