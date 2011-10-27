//
// Code to convert from the parser format into a data container.
//

#include "Combination/CDIConverter.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"

using Analysis::CalibrationDataHistogramContainer;
using Analysis::CalibrationDataContainer;

namespace BTagCombination {

  //
  // Master converter. Returns a data container with a set of calibrations in it.
  //

  CalibrationDataContainer *ConvertToCDI (const CalibrationAnalysis &eff, const std::string &name)
  {
    CalibrationDataHistogramContainer *result = new CalibrationDataHistogramContainer(name.c_str());

    //
    // First, convert this analysis to a histogram.
    //

    //
    // now, get the systeamtic histogram for each histogram.
    //

    return result;

  }

}
