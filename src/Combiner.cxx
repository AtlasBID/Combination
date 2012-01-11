//
// Code to implement the combination of bins and analyses, and the direct interface to the
// actual combination code.
//

#include "Combination/Combiner.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/CombinationContext.h"
#include "Combination/CommonCommandLineUtils.h"

#include <RooRealVar.h>

#include <stdexcept>

using namespace std;

namespace {
  using namespace BTagCombination;

  //
  // Add all the measurements for a particular bin into the context.
  //  - Assume everythign in the bins vector is the same bin! (no x-checking).
  //  - Variable name is based on the bin name - mapping should be "obvious".
  //  - Sys errors are added as well.
  //
  void FillContextWithBinInfo (CombinationContext &ctx, const vector<CalibrationBin> &bins)
  {
    // Simple x-checks and setup
    if (bins.size() == 0)
      return;

    string binName (OPBinName(bins[0]));

    // For each bin, add the info
    for (unsigned int i = 0; i < bins.size(); i++) {
      const CalibrationBin &b(bins[i]);
      Measurement *m = ctx.AddMeasurement (binName, -1.0, 2.0, b.centralValue, b.centralValueStatisticalError);
    }
  }

  //
  // Extract the complete result - with sys errors - from the calibratoin bin
  //  - Context has already had the fit run.
  //
  CalibrationBin ExtractBinResult (CombinationContext::FitResult binResult, CalibrationBin &forThisBin)
  {
    CalibrationBin result;
    result.binSpec = forThisBin.binSpec;

    result.centralValue = binResult.centralValue;
    result.centralValueStatisticalError = binResult.statisticalError;

    return result;
  }
}

namespace BTagCombination
{

  CalibrationBin CombineBin (vector<CalibrationBin> &bins)
  {
    // Simple checks to make sure we aren't bent out of shape

    if (bins.size() == 0) {
      throw runtime_error ("Unable to combine zero bins");
    }

    // If there is 1 bin we might as well short-circuit everything
    if (bins.size() == 1)
      return bins[0];

    // If there are more than 1 bins, then they all need to be identical!
    vector<CalibrationBinBoundary> specs (bins[0].binSpec);
    for (unsigned int ibin = 1; ibin < bins.size(); ibin++)
      if (!BinBoundaryUtils::compare_spec(specs, bins[ibin].binSpec))
	throw runtime_error ("Attempt to combine two bins with different boundaries");

    // Now we are ready to build the combination. Do a single fit of everything.
    CombinationContext ctx;
    FillContextWithBinInfo (ctx, bins);
    map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

    // Now that we have the result, we need to extract the numbers and build the resulting bin
    string binName (OPBinName(bins[0]));    

    if (fitResult.find(binName) == fitResult.end())
      throw runtime_error ("Unable to find bin '" + binName + "' in the fit results");

    CalibrationBin result (ExtractBinResult (fitResult[binName], bins[0]));
    result.binSpec = specs;

    return result;
  }
}

