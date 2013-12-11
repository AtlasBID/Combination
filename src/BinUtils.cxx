//
// BinUtils - utilities for dealing iwth bins in a "gross" manar.
//

#include "Combination/BinUtils.h"

using namespace std;

namespace {
  using namespace BTagCombination;

  // Helper function that returns the true if the coordinate matching the axis name has the value val.
  bool has_low_coord (const string &axis_name, const double val, const vector<CalibrationBinBoundary> &coords) {
    for (vector<CalibrationBinBoundary>::const_iterator itr = coords.begin(); itr != coords.end(); itr++) {
      if (itr->variable == axis_name
	  && itr->lowvalue == val)
	return true;
    }
    return false;
  }
}

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
  // Get a list of all bins in an analysis.
  //
  set<set<CalibrationBinBoundary> > listAnalysisBins (const CalibrationAnalysis &ana)
  {
    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    return listAllBins(anas);
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

  //
  // Return a list of analyses that are just like the orginal, with the specified bin removed.
  //
  vector<CalibrationAnalysis> removeAllBinsButBin (const vector<CalibrationAnalysis> &analyses, const set<CalibrationBinBoundary> &binToRemove)
  {
    vector<CalibrationAnalysis> result;
    for (vector<CalibrationAnalysis>::const_iterator i_ana = analyses.begin(); i_ana != analyses.end(); i_ana++) {
      CalibrationAnalysis rana (*i_ana);
      rana.bins.clear();

      for (vector<CalibrationBin>::const_iterator i_bin = i_ana->bins.begin(); i_bin != i_ana->bins.end(); i_bin++) {
	set<CalibrationBinBoundary> binspec (i_bin->binSpec.begin(), i_bin->binSpec.end());
	if (binspec == binToRemove) {
	  rana.bins.push_back(*i_bin);
	}
      }
      if (rana.bins.size() > 0)
	result.push_back (rana);
    }
    return result;
  }

  set<string> listAllSysErrors (const vector<CalibrationAnalysis> &analyses)
  {
    set<string> result;
    for(vector<CalibrationAnalysis>::const_iterator itr = analyses.begin(); itr != analyses.end(); itr++) {
      for(vector<CalibrationBin>::const_iterator i_bin = itr->bins.begin(); i_bin != itr->bins.end(); i_bin++) {
	for(vector<SystematicError>::const_iterator i_sys = i_bin->systematicErrors.begin(); i_sys != i_bin->systematicErrors.end(); i_sys++) {
	  result.insert(i_sys->name);
	}
      }
    }
    return result;
  }

  // Remove a sys error from our fits.
  vector<CalibrationAnalysis> removeSysError(const vector<CalibrationAnalysis> &analyses, const string &sysErrorName)
  {
    vector<CalibrationAnalysis> results;
    for(vector<CalibrationAnalysis>::const_iterator itr = analyses.begin(); itr != analyses.end(); itr++) {
      CalibrationAnalysis a (*itr);
      a.bins.clear();

      for(vector<CalibrationBin>::const_iterator i_bin = itr->bins.begin(); i_bin != itr->bins.end(); i_bin++) {
	CalibrationBin b(*i_bin);
	b.systematicErrors.clear();
	for(vector<SystematicError>::const_iterator i_sys = i_bin->systematicErrors.begin(); i_sys != i_bin->systematicErrors.end(); i_sys++) {
	  if (i_sys->name != sysErrorName) {
	    b.systematicErrors.push_back(*i_sys);
	  }
	}
	a.bins.push_back(b);
      }
      results.push_back(a);
    }
    return results;
  }

  // Make a systematic error uncorrelated
  vector<CalibrationAnalysis> makeSysErrorUncorrelated(const vector<CalibrationAnalysis> &analyses, const string &sysErrorName)
  {
    vector<CalibrationAnalysis> results;
    for(vector<CalibrationAnalysis>::const_iterator itr = analyses.begin(); itr != analyses.end(); itr++) {
      CalibrationAnalysis a (*itr);
      a.bins.clear();

      for(vector<CalibrationBin>::const_iterator i_bin = itr->bins.begin(); i_bin != itr->bins.end(); i_bin++) {
	CalibrationBin b(*i_bin);
	b.systematicErrors.clear();
	for(vector<SystematicError>::const_iterator i_sys = i_bin->systematicErrors.begin(); i_sys != i_bin->systematicErrors.end(); i_sys++) {
	  SystematicError t (*i_sys);
	  if (i_sys->name == sysErrorName) {
	    t.uncorrelated = true;
	  }
	  b.systematicErrors.push_back(t);
	}
	a.bins.push_back(b);
      }
      results.push_back(a);
    }
    return results;
  }

  // Calc the total bin systematic error
  double bin_sys (const CalibrationBin &b)
  {
    double total = 0.0;
    for (vector<SystematicError>::const_iterator itr = b.systematicErrors.begin(); itr != b.systematicErrors.end(); itr++) {
      double v = itr->value;
      total += v*v;
    }

    return sqrt(total);
  }

  // Find all bins with a common low edge for a partiuclar axis.
  vector<CalibrationBin> find_bins_with_low_edge(const string &axis_name, const double axis_low_val, const vector<CalibrationBin> allbins)
  {
    vector<CalibrationBin> r;

    for (vector<CalibrationBin>::const_iterator itr = allbins.begin(); itr != allbins.end(); itr++) {
      if (has_low_coord(axis_name, axis_low_val, itr->binSpec)) {
	r.push_back(*itr);
      }
    }

    return r;
  }

}
