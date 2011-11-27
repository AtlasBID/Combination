//
// Code to implement the combination of bins and analyses.
//

#include "Combination/Combiner.h"

#include <stdexcept>

namespace BTagCombination
{
  using namespace std;

  CalibrationBin CombineBin (vector<CalibrationBin> &bins)
  {
    // Simple checks to make sure we aren't bent out of shape

    if (bins.size() == 0) {
      throw runtime_error ("Unable to combine zero bins");
    }

    // If there is 1 bin we might as well short-circuit everything
    if (bins.size() == 1)
      return bins[0];

    throw runtime_error ("Not implemented yet");
  }
}

