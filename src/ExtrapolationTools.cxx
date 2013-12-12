// Extrapolation tools

#include "Combination/ExtrapolationTools.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/BinUtils.h"
#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"

#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

namespace {
  using namespace BTagCombination;

  // Helper function that will create a new extended analysis bin.
  CalibrationBin create_extrapolated_bin (double newsys, const CalibrationBin &bin)
  {
    CalibrationBin b(bin);
    b.systematicErrors.clear();
    SystematicError e;
    e.name = "extrapolated";
    e.value = newsys;
    b.systematicErrors.push_back(e);
    b.isExtended = true;
    return b;
  }

  // Helper function to return bin coordinates minus a particular item.
  set<CalibrationBinBoundary> boundary_set_without(const string &axis, const vector<CalibrationBinBoundary> &coords) {
    set<CalibrationBinBoundary> r;
    for (vector<CalibrationBinBoundary>::const_iterator itr = coords.begin(); itr != coords.end(); itr++) {
      if (itr->variable != axis)
	r.insert(*itr);
    }
    return r;
  }

  // Helper function to return bin coordinates
  set<CalibrationBinBoundary> boundary_set (const vector<CalibrationBinBoundary> &coords) {
    set<CalibrationBinBoundary> r;
    for (vector<CalibrationBinBoundary>::const_iterator itr = coords.begin(); itr != coords.end(); itr++) {
      r.insert(*itr);
    }
    return r;
  }

  // Helper function that will catalog the bins by coordinates other than the axis
  map<set<CalibrationBinBoundary>, CalibrationBin> bin_dict (const string &axis, const vector<CalibrationBin> &bins)
  {
    map<set<CalibrationBinBoundary>, CalibrationBin> r;
    for (vector<CalibrationBin>::const_iterator itr = bins.begin(); itr != bins.end(); itr++) {
      r[boundary_set_without(axis, itr->binSpec)] = *itr;
    }
    return r;
  }
}

namespace BTagCombination {
  ///
  /// Add the extrapolated data, after rescaling, to the current analysis.
  ///
  CalibrationAnalysis addExtrapolation (const CalibrationAnalysis &extrapolated,
					const CalibrationAnalysis &ana)
  {
    // Make sure we aren't extrapolating twice!
    for (vector<CalibrationBin>::const_iterator itr = ana.bins.begin(); itr != ana.bins.end(); itr++) {
      if (itr->isExtended)
	throw runtime_error ("Can't extrapolate an analysis with extrapolated bins");
    }

    // Get the bin boundaries of everything
    bin_boundaries ana_bounds (calcBoundaries(ana));
    bin_boundaries extr_bounds (calcBoundaries(extrapolated));

    // Make sure the bin boundaries are consistent with each other
    vector<bin_boundaries> bbs;
    bbs.push_back(ana_bounds);
    bbs.push_back(extr_bounds);
    checkForConsitentBoundaries(bbs);

    // To do the extrapolation, we have to figure out what axis the extension is going on (and make sure
    // we are only doing it in one!).

    vector<string> axis_names (ana_bounds.axis_names());
    string extrapolated_axis("");
    vector<double> bin_edges_ana;
    vector<double> bin_edges_ext;
    for (vector<string>::const_iterator itr = axis_names.begin(); itr != axis_names.end(); itr++) {
      bin_edges_ana = ana_bounds.get_axis_bins(*itr);
      bin_edges_ext = extr_bounds.get_axis_bins(*itr);

      sort(bin_edges_ana.begin(), bin_edges_ana.end());
      sort(bin_edges_ext.begin(), bin_edges_ext.end());

      if (bin_edges_ana[0] > bin_edges_ext[0]
	  || *(bin_edges_ana.end()-1) < *(bin_edges_ext.end()-1)) {
	if (extrapolated_axis.size() > 0) {
	  ostringstream err;
	  err << "At least axis " << extrapolated_axis << " and " << *itr << " are extrapolated. Can only deal with extrapolations along a single axis.";
	  throw runtime_error (err.str());
	}
	extrapolated_axis = *itr;
      }
    }

    // Get the last extrapolated bin, and normalize the size of the error there, and then
    // start looping through the extrapolated bins adding them in as extrapolated bins. These
    // bins must match exactly or we are in a bad way!

    double lowedge = *(bin_edges_ana.end()-2);
    vector<CalibrationBin> ana_bins_ledge (find_bins_with_low_edge(extrapolated_axis, lowedge, ana.bins));
    vector<CalibrationBin> ext_bins_ledge (find_bins_with_low_edge(extrapolated_axis, lowedge, extrapolated.bins));

    map<set<CalibrationBinBoundary>, CalibrationBin> ana_bin_info (bin_dict(extrapolated_axis, ana_bins_ledge));
    map<set<CalibrationBinBoundary>, CalibrationBin> ext_bin_info (bin_dict(extrapolated_axis, ext_bins_ledge));
    map<set<CalibrationBinBoundary>, double> ana_sys;
    map<set<CalibrationBinBoundary>, double> ext_sys;
    for (map<set<CalibrationBinBoundary>,CalibrationBin>::const_iterator a_itr = ana_bin_info.begin(); a_itr != ana_bin_info.end(); a_itr++) {
      map<set<CalibrationBinBoundary>,CalibrationBin>::const_iterator e_itr = ext_bin_info.find(a_itr->first);
      if (e_itr == ext_bin_info.end()) {
	ostringstream err;
	err << "Unable to find a bin that matches " << OPBinName(a_itr->first) << " in the extrapolation.";
	throw runtime_error(err.str());
      }

      CalibrationBin ana_bin (a_itr->second);
      CalibrationBin ext_bin (e_itr->second);

      ana_sys[a_itr->first] = bin_sys (ana_bin);
      ext_sys[a_itr->first] = bin_sys (ext_bin);
    }

    // Go through each of the extrapolated bins, and add a bin into the analysis.
    // Skip extrapolated bins that are already inside the analysis.
    
    CalibrationAnalysis r(ana);

    set<set<CalibrationBinBoundary> > all_analysis_bin_boundaries(listAnalysisBins(ana));
    for (vector<CalibrationBin>::const_iterator e_itr = extrapolated.bins.begin(); e_itr != extrapolated.bins.end(); e_itr++) {
      set<CalibrationBinBoundary> all_bounds(boundary_set(e_itr->binSpec));
      if (all_analysis_bin_boundaries.find(all_bounds) == all_analysis_bin_boundaries.end()) {
	set<CalibrationBinBoundary> bounds (boundary_set_without(extrapolated_axis, e_itr->binSpec));
	if (ana_sys.find(bounds) != ana_sys.end()) {
	  double ext_sys_current = bin_sys(*e_itr);
	  double ext_sys_base = ext_sys[bounds];
	  if (ext_sys_current < ext_sys_base) {
	    ostringstream err;
	    err << "Extrapolated bin's error (" << ext_sys_current << ")"
		<< " is smaller than the last bin matching the analysis (" << ext_sys_base << ")."
		<< " "
		<< "Analysis: " << OPFullName(r) << "  "
		<< "Extrapolation: " << OPFullName(extrapolated) << " (" << OPBinName(e_itr->binSpec) << ")";
	    throw runtime_error(err.str());
	  }
	  double ana_sys_base (ana_sys[bounds]);
	  double ext_sys_new = ext_sys_current/ext_sys_base * ana_sys_base;
	  // Do the quad calc to figure out what this component should be.
	  ext_sys_new = sqrt(ext_sys_new*ext_sys_new - ana_sys_base*ana_sys_base);
	  r.bins.push_back(create_extrapolated_bin (ext_sys_new, *e_itr));
	}
      }
    }

    return r;
  }
}
