//
// Code to convert a parsed data data structure into the
// calibration data interface format that can be stored in a root file.
//

#ifndef CDIConverter_H
#define CDIConverter_H

#include "Combination/Parser.h"
#include "CalibrationDataInterface/CalibrationDataContainer.h"

#include <string>


namespace BTagCombination {

  // Given the calibration analyses in all their glory, conver them!
  Analysis::CalibrationDataContainer *ConvertToCDI (const BTagCombination::CalibrationAnalysis &eff,
						    const std::string &name);

}

#endif
