//
// BinUtils - utilities for dealing iwth bins in a "gross" manar.
//

#include "Combination/BinUtils.h"

using namespace std;

namespace BTagCombination {

  //
  // Get a list of all bins in all analyses. Unique bins.
  //
  set<set<CalibrationBinBoundary> > listAllBins (const vector<CalibrationAnalysis> &analyses)
  {
    set<set<CalibrationBinBoundary> > result;

    for (vector<CalibrationAnalysis>::const_iterator i_ana = analyses.begin(); i_ana != analyses.end(); i_ana++) {
      for (vector<CalibrationBin>::const_iterator i_bin = i_ana->bins.begin(); i_bin != i_ana->bins.end(); i_bin++) {
	result.insert(set<CalibrationBinBoundary>(i_bin->binSpec.begin(), i_bin->binSpec.end()));
      }
    }

    return result;
  }

  //
  // Return a list of analyses that are just like the orginal, with the specified bin removed.
  //
  vector<CalibrationAnalysis> removeBin (const vector<CalibrationAnalysis> &analyses, const set<CalibrationBinBoundary> &binToRemove)
  {
    vector<CalibrationAnalysis> result;
    for (vector<CalibrationAnalysis>::const_iterator i_ana = analyses.begin(); i_ana != analyses.end(); i_ana++) {
      CalibrationAnalysis rana (*i_ana);
      rana.bins.clear();

      for (vector<CalibrationBin>::const_iterator i_bin = i_ana->bins.begin(); i_bin != i_ana->bins.end(); i_bin++) {
	set<CalibrationBinBoundary> binspec (i_bin->binSpec.begin(), i_bin->binSpec.end());
	if (binspec != binToRemove) {
	  rana.bins.push_back(*i_bin);
	}
      }
      if (rana.bins.size() > 0)
	result.push_back (rana);
    }
    return result;
  }

}
