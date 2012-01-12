//
// Code to implement the combination of bins and analyses, and the direct interface to the
// actual combination code.
//

#include "Combination/Combiner.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"
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
      for (unsigned int i_sys = 0; i_sys < b.systematicErrors.size(); i_sys++) {
	const SystematicError &err(b.systematicErrors[i_sys]);
	m->addSystematicAbs(err.name, err.value);
      }
    }
  }

  //
  // Extract the complete result - with sys errors - from the calibratoin bin
  //  - Context has already had the fit run.
  //
  CalibrationBin ExtractBinResult (CombinationContext::FitResult binResult, const CalibrationBin &forThisBin)
  {
    CalibrationBin result;
    result.binSpec = forThisBin.binSpec;

    result.centralValue = binResult.centralValue;
    result.centralValueStatisticalError = binResult.statisticalError;

    for (map<string, double>::const_iterator i_sys = binResult.sysErrors.begin(); i_sys != binResult.sysErrors.end(); i_sys++) {
      SystematicError e;
      e.name = i_sys->first;
      e.value = i_sys->second;
      result.systematicErrors.push_back(e);
    }

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

    return result;
  }

  // Given a list of compatible analyses, combine them.
  // Compatible means matching bins and matching flavors.
  // Assume:
  //  - The bins do not overlap
  //  - The flavor is the same
  //  - The cut point is the same
  //  - The tagger is the same
  //  - The jet algorithm is the same
  CalibrationAnalysis CombineAnalyses (vector<CalibrationAnalysis> &ana)
  {
    //
    // Specal cases and input checks, and simple setup
    //

    if (ana.size() == 0)
      throw runtime_error ("Can't combine zero analyses!");

    CalibrationAnalysis result (ana[0]);
    result.name = "combined";

    if (ana.size() == 1) {
      return result;
    }
    result.bins.clear();

    //
    // Now that we know everything is the same, and we only differ by the analysis
    // we can sort by the bins.
    //

    map<string, vector<CalibrationBin> > bybins;
    for (unsigned int i_ana = 0; i_ana < ana.size(); i_ana++) {
      const CalibrationAnalysis &a(ana[i_ana]);
      for (unsigned int i_bin = 0; i_bin < a.bins.size(); i_bin++) {
	const CalibrationBin &b(a.bins[i_bin]);
	bybins[OPBinName(b)].push_back(b);
      }
    }

    //
    // Now, build the context for the fit, and do the fit.
    //

    CombinationContext ctx;
    for (map<string, vector<CalibrationBin> >::const_iterator i_b = bybins.begin(); i_b != bybins.end(); i_b++) {
      FillContextWithBinInfo (ctx, i_b->second);
    }

    map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

    //
    // Finally, go through and extract the fit results.
    //

    for (map<string, vector<CalibrationBin> >::const_iterator i_b = bybins.begin(); i_b != bybins.end(); i_b++) {
      string binName = i_b->first;
      CalibrationBin thisBin (ExtractBinResult (fitResult[binName], i_b->second[0]));
      result.bins.push_back(thisBin);
    }

    return result;
  }

}

