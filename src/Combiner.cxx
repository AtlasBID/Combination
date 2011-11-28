//
// Code to implement the combination of bins and analyses.
//

#include "Combination/Combiner.h"

#include <stdexcept>

using namespace std;

namespace {
  using namespace BTagCombination;

  // Check to see if the two bins are the same or not.
  bool sameBin (const vector<CalibrationBinBoundary> &b1, const vector<CalibrationBinBoundary> &b2)
  {
    return false;
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
      if (!sameBin(specs, bins[ibin].binSpec))
	throw runtime_error ("Attempt to combine two bins with different boundaries");

    // Now we are ready to build the combination!

    // Now that we have the result, we need to extract the numbers and build the resulting bin
    CalibrationBin result;
    result.binSpec = specs;

    return CalibrationBin();
  }
}

