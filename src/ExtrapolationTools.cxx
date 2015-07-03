// Extrapolation tools

#include "Combination/ExtrapolationTools.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/BinUtils.h"
#include "Combination/BinNameUtils.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/FitLinage.h"

#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

namespace {
	using namespace BTagCombination;

	// Helper function that will create a new extended analysis bin based on an old bin.
	// All old sys errors will be cleared, but other than that, it remains the same.
	CalibrationBin create_extrapolated_bin(double newsys, const CalibrationBin &templateBin)
	{
		CalibrationBin b(templateBin);
		b.systematicErrors.clear();
		SystematicError e;
		e.name = "extrapolated";
		e.value = newsys;
		b.systematicErrors.push_back(e);
		b.isExtended = true;
		return b;
	}

	// Helper function to return bin coordinates minus a particular axis cordinate.
	set<CalibrationBinBoundary> boundary_set_without(const string &axis, const vector<CalibrationBinBoundary> &coords) {
		set<CalibrationBinBoundary> r;
		for (vector<CalibrationBinBoundary>::const_iterator itr = coords.begin(); itr != coords.end(); itr++) {
			if (itr->variable != axis)
				r.insert(*itr);
		}
		return r;
	}

	// Helper function to return bin coordinates
	set<CalibrationBinBoundary> boundary_set(const vector<CalibrationBinBoundary> &coords) {
		return set<CalibrationBinBoundary>(coords.begin(), coords.end());
	}

	// Helper function that will catalog the bins by coordinates other than the axis
	map<set<CalibrationBinBoundary>, CalibrationBin> bin_dict(const string &axis, const vector<CalibrationBin> &bins)
	{
		map<set<CalibrationBinBoundary>, CalibrationBin> r;
		for (vector<CalibrationBin>::const_iterator itr = bins.begin(); itr != bins.end(); itr++) {
			r[boundary_set_without(axis, itr->binSpec)] = *itr;
		}
		return r;
	}

        // Helper class for a pair of numbers that are zero'd automatically.
        class zero_pair {
        public:
          double _p1;
          double _p2;

          zero_pair()
            : _p1(0), _p2(0)
          {}
        };

        // What is the sign of a number?
        int sign(double n)
        {
          return n < 0 ? -1 : 1;
        }

        // Subtract two systematic errors (e1 - e2). Special Cases:
        //   - e1 or e2 is missing the sytematic error
        //   - e1 is anti-correlated to e2.
        //   - e2 > e1 (after anti-correlation accounted for), sign flip is ignored.
        vector<SystematicError> subtract_sys_errors(const vector<SystematicError> &e1, const vector<SystematicError> &e2)
        {
          map<string, zero_pair> sys_pairs;
          for_each(e1.begin(), e1.end(), [&](const SystematicError &e) { sys_pairs[e.name]._p1 = e.value; });
          for_each(e2.begin(), e2.end(), [&](const SystematicError &e) { sys_pairs[e.name]._p2 = e.value; });

          vector<SystematicError> result;
          for (auto ep : sys_pairs)
          {
            SystematicError new_e;
            new_e.name = ep.first;
            new_e.uncorrelated = true;

            bool bothNeg = sign(ep.second._p1) == -1 && sign(ep.second._p2) == -1;
            bool opSigned = false;
            if (sign(ep.second._p1) != +1) {
              opSigned = !opSigned;
              ep.second._p1 = -ep.second._p1;
            }
            if (sign(ep.second._p2) != +1) {
              opSigned = !opSigned;
              ep.second._p2 = -ep.second._p2;
            }
            new_e.value = fabs(ep.second._p1 - ep.second._p2);
            if (opSigned || bothNeg) {
              new_e.value = -new_e.value;
            }
            result.push_back(new_e);
          }
          return result;
        }
}

namespace BTagCombination {
	///
	/// Add the extrapolated data, after rescaling, to the current analysis.
	///
	CalibrationAnalysis addExtrapolation(const CalibrationAnalysis &extrapolated,
		const CalibrationAnalysis &ana)
	{
		// Make sure we aren't extrapolating twice!
		for (vector<CalibrationBin>::const_iterator itr = ana.bins.begin(); itr != ana.bins.end(); itr++) {
			if (itr->isExtended)
				throw runtime_error("Can't extrapolate an analysis with extrapolated bins");
		}

		// Get the bin boundaries of everything
		bin_boundaries ana_bounds(calcBoundaries(ana));
		bin_boundaries extr_bounds(calcBoundaries(extrapolated));

		// Make sure the bin boundaries are consistent with each other
		vector<bin_boundaries> bbs;
		bbs.push_back(ana_bounds);
		bbs.push_back(extr_bounds);

		try {
			checkForConsitentBoundaries(bbs);
		}
		catch (runtime_error &excp) {
			ostringstream err;
			err << "Extrapolation failed because bins were inconsistent" << endl
				<< "  Analysis: " << OPFullName(ana) << endl;
			for (unsigned int i = 0; i < ana.bins.size(); i++) {
				err << "    " << OPBinName(ana.bins[i]) << endl;
			}
			err << "  Extrapolation: " << OPFullName(extrapolated) << endl
				<< "  Error: " << excp.what();

			throw runtime_error(err.str());
		}

		// To do the extrapolation, we have to figure out what axis the extension is going on (and make sure
		// we are only doing it in one!).

		vector<string> axis_names(ana_bounds.axis_names());
		string extrapolated_axis("");
		vector<double> bin_edges_ana;
		vector<double> bin_edges_ext;
		for (vector<string>::const_iterator itr = axis_names.begin(); itr != axis_names.end(); itr++) {
			bin_edges_ana = ana_bounds.get_axis_bins(*itr);
			bin_edges_ext = extr_bounds.get_axis_bins(*itr);

			sort(bin_edges_ana.begin(), bin_edges_ana.end());
			sort(bin_edges_ext.begin(), bin_edges_ext.end());

			if (bin_edges_ana[0] > bin_edges_ext[0]
				|| *(bin_edges_ana.end() - 1) < *(bin_edges_ext.end() - 1)) {
				if (extrapolated_axis.size() > 0) {
					ostringstream err;
					err << "At least axis " << extrapolated_axis << " and " << *itr << " are extrapolated. Can only deal with extrapolations along a single axis " 
						<< "(analysis: " << OPFullName(ana) << " extrap ana: " << OPFullName(extrapolated) << ").";
					throw runtime_error(err.str());
				}
				extrapolated_axis = *itr;
			}
		}

#define ERROR_PROPAGATION
#ifdef ERROR_PROPAGATION
                // The error propagation method, which is used in Run 2.
                // The size of the error is just the difference in error between the reference bin and the
                // extrapolated bin. This has to be done on a sys-error by sys-error basis.

                double lowedge = *(bin_edges_ana.end() - 2); // Low edge of last bin in data
                auto ana_bins_ledge(find_bins_with_low_edge(extrapolated_axis, lowedge, ana.bins));
                auto ext_bins_ledge(find_bins_with_low_edge(extrapolated_axis, lowedge, extrapolated.bins));

                // Cache the systematic errors for the reference extrapolation bin.
                map<set<CalibrationBinBoundary>, CalibrationBin> ana_bin_info(bin_dict(extrapolated_axis, ana_bins_ledge));
                map<set<CalibrationBinBoundary>, CalibrationBin> ext_bin_info(bin_dict(extrapolated_axis, ext_bins_ledge));
                map<set<CalibrationBinBoundary>, vector<SystematicError>> ext_sys;
                map<set<CalibrationBinBoundary>, CalibrationBin> ana_bin_cache;
                for (auto a_itr = ana_bin_info.begin(); a_itr != ana_bin_info.end(); a_itr++) {
                  auto e_itr = ext_bin_info.find(a_itr->first);
                  if (e_itr == ext_bin_info.end()) {
                    ostringstream err;
                    err << "Unable to find a bin that matches " << OPBinName(a_itr->first) << " in the extrapolation.";
                    throw runtime_error(err.str());
                  }

                  CalibrationBin ana_bin(a_itr->second);
                  CalibrationBin ext_bin(e_itr->second);

                  ext_sys[a_itr->first] = ext_bin.systematicErrors;
                  ana_bin_cache[a_itr->first] = ana_bin;
                }

                // Go through each of the extrapolated bins, and add a bin into the analysis.
                // Skip extrapolated bins that are already inside the analysis.

                CalibrationAnalysis r(ana);

                set<set<CalibrationBinBoundary> > all_analysis_bin_boundaries(listAnalysisBins(ana));
                for (auto e_itr = extrapolated.bins.begin(); e_itr != extrapolated.bins.end(); e_itr++) {
                  set<CalibrationBinBoundary> all_bounds(boundary_set(e_itr->binSpec));
                  if (all_analysis_bin_boundaries.find(all_bounds) == all_analysis_bin_boundaries.end()) {
                    set<CalibrationBinBoundary> bounds(boundary_set_without(extrapolated_axis, e_itr->binSpec));
                    if (ext_sys.find(bounds) != ext_sys.end()) {

                      // Make sure that extrapolated bin is reasonable (this is a dataquality test
                      // that is in here because we've seen this not to be the case due to poor checking
                      // of inputs.

                      double ext_sys_current = bin_sys(*e_itr);
                      if (ext_sys_current == 0.0) {
                        ostringstream err;
                        err << "Extrapolated bin's is error zero " << endl
                          << "  Analysis: " << OPFullName(r) << endl
                          << "  Extrapolation: " << OPFullName(extrapolated) << " (" << OPBinName(e_itr->binSpec) << ")";
                        cerr << err.str() << endl;
                        throw runtime_error(err.str());
                      }

                      // Calculate the difference in systematic errors

                      auto deltaSys = subtract_sys_errors(e_itr->systematicErrors, ext_sys[bounds]);
                      auto ext_sys_new = bin_sys(deltaSys);

                      CalibrationBin newb(create_extrapolated_bin(ext_sys_new, *e_itr));
                      newb.centralValue = ana_bin_cache[bounds].centralValue;
                      newb.centralValueStatisticalError = ana_bin_cache[bounds].centralValueStatisticalError;
                      r.bins.push_back(newb);
                    }
                  }
                }
#endif

#ifdef SCALING_METHOD
                // This is the scaling method, which was used in Run 1.

		// Get the last extrapolated bin, and normalize the size of the error there, and then
		// start looping through the extrapolated bins adding them in as extrapolated bins. These
		// bins must match exactly or we are in a bad way!

		double lowedge = *(bin_edges_ana.end() - 2);
		vector<CalibrationBin> ana_bins_ledge(find_bins_with_low_edge(extrapolated_axis, lowedge, ana.bins));
		vector<CalibrationBin> ext_bins_ledge(find_bins_with_low_edge(extrapolated_axis, lowedge, extrapolated.bins));

		map<set<CalibrationBinBoundary>, CalibrationBin> ana_bin_info(bin_dict(extrapolated_axis, ana_bins_ledge));
		map<set<CalibrationBinBoundary>, CalibrationBin> ext_bin_info(bin_dict(extrapolated_axis, ext_bins_ledge));
		map<set<CalibrationBinBoundary>, double> ana_sys;
		map<set<CalibrationBinBoundary>, double> ext_sys;
		map<set<CalibrationBinBoundary>, CalibrationBin> ana_bin_cache;
		for (map<set<CalibrationBinBoundary>, CalibrationBin>::const_iterator a_itr = ana_bin_info.begin(); a_itr != ana_bin_info.end(); a_itr++) {
			map<set<CalibrationBinBoundary>, CalibrationBin>::const_iterator e_itr = ext_bin_info.find(a_itr->first);
			if (e_itr == ext_bin_info.end()) {
				ostringstream err;
				err << "Unable to find a bin that matches " << OPBinName(a_itr->first) << " in the extrapolation.";
				throw runtime_error(err.str());
			}

			CalibrationBin ana_bin(a_itr->second);
			CalibrationBin ext_bin(e_itr->second);

			ana_sys[a_itr->first] = bin_sys(ana_bin);
			ext_sys[a_itr->first] = bin_sys(ext_bin);
			ana_bin_cache[a_itr->first] = ana_bin;
		}

		// Go through each of the extrapolated bins, and add a bin into the analysis.
		// Skip extrapolated bins that are already inside the analysis.

		CalibrationAnalysis r(ana);

		set<set<CalibrationBinBoundary> > all_analysis_bin_boundaries(listAnalysisBins(ana));
		for (vector<CalibrationBin>::const_iterator e_itr = extrapolated.bins.begin(); e_itr != extrapolated.bins.end(); e_itr++) {
			set<CalibrationBinBoundary> all_bounds(boundary_set(e_itr->binSpec));
			if (all_analysis_bin_boundaries.find(all_bounds) == all_analysis_bin_boundaries.end()) {
				set<CalibrationBinBoundary> bounds(boundary_set_without(extrapolated_axis, e_itr->binSpec));
				if (ana_sys.find(bounds) != ana_sys.end()) {
					double ext_sys_current = bin_sys(*e_itr);
					double ext_sys_base = ext_sys[bounds];
					if (ext_sys_current == 0.0) {
						ostringstream err;
						err << "Extrapolated bin's is error zero " << endl
							<< "  Analysis: " << OPFullName(r) << endl
							<< "  Extrapolation: " << OPFullName(extrapolated) << " (" << OPBinName(e_itr->binSpec) << ")";
						cerr << err.str() << endl;
						throw runtime_error(err.str());
					}
					if (ext_sys_current < ext_sys_base) {
						ostringstream err;
						err << "Extrapolated bin's error (" << ext_sys_current << ")"
							<< " is smaller than the last bin matching the analysis (" << ext_sys_base << ")." << endl
							<< "  Analysis: " << OPFullName(r) << endl
							<< "  Extrapolation: " << OPFullName(extrapolated) << " (" << OPBinName(e_itr->binSpec) << ")"
							<< "  --> Reset to be identical";
						cerr << err.str() << endl;
						ext_sys_current = ext_sys_base;
					}
					double ana_sys_base(ana_sys[bounds]);
					double ext_sys_new = ext_sys_current / ext_sys_base * ana_sys_base;
					// Do the quad calc to figure out what this component should be.
					double ext_sys_new_delta = sqrt(ext_sys_new*ext_sys_new - ana_sys_base*ana_sys_base);
					CalibrationBin newb(create_extrapolated_bin(ext_sys_new_delta, *e_itr));
					newb.centralValue = ana_bin_cache[bounds].centralValue;
					newb.centralValueStatisticalError = ana_bin_cache[bounds].centralValueStatisticalError;
					r.bins.push_back(newb);
				}
			}
		}
#endif
		// Update the linage.

		r.metadata_s["Linage"] = BinaryLinageOp(r, extrapolated, LBExtrapolate);

		return r;
	}
}
