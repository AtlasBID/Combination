//
// Tools to help take a MC analysis and SF and turn them into a calibration that has been
// extended.
//

#ifndef ExtrapolationTools_H
#define ExtrapolationTools_H

#include "Combination/Parser.h"

namespace BTagCombination {

  // Given an extrapolated analysis, apply it to ana in order to extend it.
  CalibrationAnalysis addExtrapolation (const CalibrationAnalysis &extrapolated,
					const CalibrationAnalysis &ana);
}

#endif
