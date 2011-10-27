//
// Code to convert from the parser format into a data container.
//

#include "Combination/CDIConverter.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

using Analysis::CalibrationDataHistogramContainer;
using Analysis::CalibrationDataContainer;

namespace {
  using namespace BTagCombination;
  using namespace std;

  // helper functions and objects for below

  class bin_boundaries2D {
  };

  typedef map<string, vector<pair<double,double> > > t_bin_list;

  // Go through all the analysis and extract all the bins that are
  // being used.
  t_bin_list extract_bins (const CalibrationAnalysis &ana)
  {
    t_bin_list result;

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
      const CalibrationBin &bin (ana.bins[ibin]);
      for (unsigned int iboundary = 0; iboundary < bin.binSpec.size(); iboundary++) {
	const CalibrationBinBoundary &bound (bin.binSpec[iboundary]);
	pair<double,double> bp = make_pair(bound.lowvalue, bound.highvalue);
	result[bound.variable].push_back(bp);
      }
    }

    return result;
  }

  bin_boundaries2D calcBoundaries (const CalibrationAnalysis &ana)
  {
    // Find all the bins that are in the analysis
    t_bin_list raw_bins = extract_bins(ana);
    if (raw_bins.size() > 2) {
      throw runtime_error(("Analysis '" + ana.name + "' has more than 2 bins!").c_str());
    }
    if (raw_bins.size() == 0) {
      throw runtime_error(("Analysis '" + ana.name + "' has no bins!").c_str());
    }

    cout << "Number of bins is " << raw_bins.size() << endl;

    bin_boundaries2D stuff;
    return stuff;
  }
}

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


    calcBoundaries(eff);
    // get bin boundaries
    // get the values 
    // create a histogram

    //
    // now, get the systeamtic histogram for each histogram.
    //

    // get the list of systematics
    // loop, use bin boundaries from above, etc.

    return result;

  }

}
