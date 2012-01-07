//
// Code to implement some bin boundary calculatiosn starting from
// analyses and bins.
//
#include "Combination/BinBoundaryUtils.h"
#include "Combination/CommonCommandLineUtils.h"

#include <sstream>
#include <algorithm>

using namespace std;

namespace {
  using namespace BTagCombination;

  typedef map<string, vector<pair<double,double> > > t_bin_list;

  // Error class used to move info between layers when there is a failure.
  class binning_error : public runtime_error {
  public:
    inline binning_error (double lowbinHigh, double highbinLow, const string &message)
      : runtime_error (message.c_str()), _lowbinH (lowbinHigh), _highbinL (highbinLow)
    {}

    double _lowbinH, _highbinL;
  };

  // Go through all the analysis and extract all the bins that are
  // being used.
  //
  // We remove duplicates. We could use a set or similar to do that, but there are so
  // few using the find algoritm is short hand...
  t_bin_list extract_bins (const CalibrationAnalysis &ana)
  {
    t_bin_list result;

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
      const CalibrationBin &bin (ana.bins[ibin]);
      for (unsigned int iboundary = 0; iboundary < bin.binSpec.size(); iboundary++) {
	const CalibrationBinBoundary &bound (bin.binSpec[iboundary]);

	pair<double,double> bp = make_pair(bound.lowvalue, bound.highvalue);
	if (find (result[bound.variable].begin(), result[bound.variable].end(), bp)
	    == result[bound.variable].end())
	  result[bound.variable].push_back(bp);
      }
    }

    return result;
  }

  // Helper function to order bins by their lower boundary in the "sort" algorithm.
  bool compare_first_of_pair (const pair<double, double> &i1, const pair<double, double> &i2)
  {
    return i1.first < i2.first;
  }

  // Given all bins make sure the are adjacent and create just a list of bin boundaries much
  // like what you would put in a histogram.
  vector<double> get_bin_boundaries_hist (const vector<pair<double, double> > &allbins)
  {
    // Put them in order
    vector<pair<double, double> > acopy (allbins);
    sort(acopy.begin(), acopy.end(), compare_first_of_pair);

    // Now, extract the boundaries
    vector<double> result;
    pair<double, double> last;
    for (unsigned int i = 0; i < acopy.size(); i++) {
      if (result.size() == 0) {
	result.push_back(acopy[i].first);
      } else {
	if (acopy[i] == last) {
	  throw runtime_error ("Duplicate bins found!");
	}
	if (last.second != acopy[i].first) {
	  ostringstream errtxt;
	  errtxt << "Bins must be adjacent and exclusive - "
		 << " lower bin's upper boundary (" << last.second << ")"
		 << " and upper bins' lower boundary (" << acopy[i].first << ") need to be the same";
	  throw binning_error (last.second, acopy[i].first, errtxt.str());
	}
	result.push_back(acopy[i].first);
      }
      last = acopy[i];
    }
    result.push_back(last.second);
    return result;
  }

  CalibrationBinBoundary get_bin_boundary_for (const CalibrationBin &bin, const string &boundaryname)
  {
    for (unsigned int i = 0; i < bin.binSpec.size(); i++) {
      if (bin.binSpec[i].variable == boundaryname)
	return bin.binSpec[i];
    }
    return CalibrationBinBoundary();
  }

  // Get a list of all bins that match a criteria
  vector<CalibrationBin> get_all_bins_matching (const CalibrationAnalysis &ana, const string &binname,
						double lowbinH, double highbinL)
  {
    vector<CalibrationBin> result;

    for (unsigned int i = 0; i < ana.bins.size(); i++) {
      CalibrationBinBoundary b (get_bin_boundary_for(ana.bins[i], binname));
      if (b.lowvalue == highbinL || b.highvalue == lowbinH)
	result.push_back(ana.bins[i]);
    }

    return result;
  }
}

namespace BTagCombination {

  ///
  /// The bin_boudnaries object
  ///

  int bin_boundaries::get_xaxis_bin (const vector<CalibrationBinBoundary> &bin_spec) const
  {
    return find_bin (get_xaxis(), bin_spec);
  }

  int bin_boundaries::get_yaxis_bin (const vector<CalibrationBinBoundary> &bin_spec) const
  {
    return find_bin (get_yaxis(), bin_spec);
  }

  int bin_boundaries::find_bin (const pair<string, vector<double> > &axis_info,
			 const vector<CalibrationBinBoundary> &bin_spec) const
  {
    CalibrationBinBoundary spec = BinBoundaryUtils::find_spec(bin_spec, axis_info.first);
    for (unsigned int ibin = 0; ibin < axis_info.second.size(); ibin++) {
      if (spec.lowvalue == axis_info.second[ibin]) {
	return ibin + 1; // Remamer, root is offset by 1 its bin numbers!!
      }
    }
    ostringstream error;
    error << "Unable to find bin wiht lower boundary '" << spec.lowvalue << "' in axis '" << axis_info.first << "'.";
    throw runtime_error (error.str().c_str());
  }

  // Grab the boundaries from the analysis, and put them into
  // an object that knows how to create historams, etc., for teh CDI.
  bin_boundaries calcBoundaries (const CalibrationAnalysis &ana)
  {
    // Find all the bins that are in the analysis
    t_bin_list raw_bins = extract_bins(ana);
    if (raw_bins.size() > 2) {
      throw runtime_error(("Analysis '" + ana.name + "' has more than 2 bins!").c_str());
    }
    if (raw_bins.size() == 0) {
      throw runtime_error(("Analysis '" + ana.name + "' has no bins!").c_str());
    }

    // For each variable, get a set of bin boundaries

    bin_boundaries result;
    bool errorSeen = false;
    ostringstream errmsg;
    errmsg << "Found problems with binning: ";
    for (t_bin_list::const_iterator ibin = raw_bins.begin(); ibin != raw_bins.end(); ibin++) {
      try {
	result.add_axis(ibin->first, get_bin_boundaries_hist(ibin->second));
      } catch (binning_error &e){
	errmsg << endl;
	errmsg << "    " << e.what() << endl;

	// Now we have to find all the possibly offending bins, and then
	// print them out in the standard printing format that can be used as part
	// of our --ignore command line.

	errmsg << "      One of the following analysis/bins is in error - please use --ignore" << endl;

	vector<CalibrationBin> badbins(get_all_bins_matching(ana, ibin->first, e._lowbinH, e._highbinL));
	for (unsigned int i = 0; i < badbins.size(); i++) {
	  errmsg << "      -> " << OPIgnoreFormat(ana, badbins[i]) << endl;
	}

	errorSeen = true;
      }
    }

    if (errorSeen) {
      throw runtime_error (errmsg.str().c_str());
    }

    return result;
  }
}