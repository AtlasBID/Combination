//
// Code to convert from the parser format into a data container.
//

#include "Combination/CDIConverter.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/CommonCommandLineUtils.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"

#include "TH2F.h"

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cmath>

using Analysis::CalibrationDataHistogramContainer;
using Analysis::CalibrationDataContainer;

namespace {
  using namespace BTagCombination;
  using namespace std;
  using boost::lambda::_1;
  using boost::lambda::_2;
  using boost::lambda::bind;

  // helper functions and objects for below

  // Helper class to generate historams, etc., for the bin boundaies we fine.
  class bin_boundaries_hist : public bin_boundaries {
  public:
    inline bin_boundaries_hist (const bin_boundaries &start)
      : bin_boundaries(start)
    {}

    // Create a 2D histogram using the two axes we know about.
    TH2 *create_histo (const string &name) const
    {
      if (_axes.size() != 2) {
	ostringstream msg;
	msg << "There must be exactly two axes (found: ";
	if (_axes.size() == 0) {
	  msg << "none";
	} else {
	  for (map<string,vector<double> >::const_iterator i = _axes.begin(); i != _axes.end(); i++) {
	    if (i != _axes.begin())
	      msg << ", ";
	    msg << i->first;
	  }
	}
	msg << ")!";
	throw runtime_error (msg.str().c_str());
      }

      // x,y axes
      const pair<string,vector<double> > xaxis (get_xaxis());
      const pair<string,vector<double> > yaxis (get_yaxis());

      // The title, which must include axes names
      ostringstream title;
      title << name << ";" << xaxis.first << ";" << yaxis.first;
      
      return new TH2F(name.c_str(), title.str().c_str(),
		      xaxis.second.size()-1, &(xaxis.second[0]),
		      yaxis.second.size()-1, &(yaxis.second[0]));
    }

    // Set the central value and error of a histogram with a particualr bin setting.
    // Trust that lower bin boundary is all that counds for lookup!!
    void set_bin_contents(TH2 *histo,
			  const vector<CalibrationBinBoundary> &bin_spec,
			  double central, double error) const
    {
      int xbin = get_xaxis_bin (bin_spec);
      int ybin = get_yaxis_bin (bin_spec);

      histo->SetBinContent (xbin, ybin, central);
      histo->SetBinError (xbin, ybin, error);
    }
  };

  // Using a functor to extract the name/value pairs for all the bins, set them.
  // Use the call-back pattern since it "should" make code easier to understand.
  // *cough* *cough*
  template<typename Pred>
  TH2 *set_bin_values (const bin_boundaries_hist &bins,
		       const CalibrationAnalysis &ana,
		       const string &hist_name,
		       const Pred &getter)
  {
    try {
      // Create the histo
      TH2 *values = bins.create_histo(hist_name);

      // Now loop over all bins in the analysis.
      for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
	const CalibrationBin &bin (ana.bins[ibin]);
	pair<double, double> val_and_error (getter(bin));
	bins.set_bin_contents (values, bin.binSpec, val_and_error.first, val_and_error.second);
      }

      return values;
    } catch (runtime_error e) {
      ostringstream msg;
      msg << "Error while processing analysis: " << e.what() << endl
	  << ana;
      throw runtime_error(msg.str().c_str());
    }

  }

  // Extract central value and statistical error from a bin
  pair<double, double> get_central_value (const CalibrationBin &bin)
  {
    return make_pair(bin.centralValue, bin.centralValueStatisticalError);
  }

  // Helper to find errors. Lord, what-for to have lambda functions enabled in ATLAS already!
  class find_error
  {
  public:
    inline find_error (const string &name)
      : _name(name)
    {}
    inline bool operator() (const SystematicError &err)
    { return err.name == _name;}
  private:
    const string &_name;
  };

  // Extract a systematic error
  class get_sys_error
  {
  public:
    inline get_sys_error (const string &sys_name)
      : _name (sys_name)
    {}
    pair<double, double> operator() (const CalibrationBin &bin) const
    {
      vector<SystematicError>::const_iterator e = find_if (bin.systematicErrors.begin(),
							   bin.systematicErrors.end(),
							   find_error(_name));
      if (e == bin.systematicErrors.end())
	return make_pair(0.0, 0.0);
      return make_pair(e->value, e->value);
    }
  private:
    const string &_name;
  };


  // Returns a list of all systematic errors used in this analysis
  set<string> sys_error_names (const CalibrationAnalysis &ana)
  {
    set<string> result;
    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
      const CalibrationBin &bin (ana.bins[ibin]);
      transform (bin.systematicErrors.begin(), bin.systematicErrors.end(),
		 inserter(result, result.begin()),
		 bind<const string&>(&SystematicError::name, _1));
    }
    return result;
  }

  // sees if this error is uncorrelated or not.
  bool is_uncorrelated (const CalibrationAnalysis &ana, const string &ename)
  {
    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
      const CalibrationBin &bin (ana.bins[ibin]);
      for (size_t i_sys = 0; i_sys < bin.systematicErrors.size(); i_sys++) {
	const SystematicError &e(bin.systematicErrors[i_sys]);
	if (e.name == ename) {
	  return e.uncorrelated;
	}
      }
    }
    return false;
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
    // We need to have a total systematic uncertianty in the CDI. So we need to tally it up.
    //

    CalibrationAnalysis ana (eff);
    for (vector<CalibrationBin>::iterator itr = ana.bins.begin(); itr != ana.bins.end(); itr++) {
      double totS = 0.0;
      for (vector<SystematicError>::const_iterator i_s = itr->systematicErrors.begin(); i_s != itr->systematicErrors.end(); i_s++) {
	totS += i_s->value*i_s->value;
      }
      SystematicError s;
      s.value = sqrt(totS);
      s.name = "systematics";
      itr->systematicErrors.push_back(s);
    }

    //
    // First, convert this analysis to a histogram, and extract the values with errors
    // being the systematic errors for each value.
    //

    bin_boundaries_hist bins = calcBoundaries(ana);
    TH2 *central_value = set_bin_values(bins, ana, "central", get_central_value);
    result->setResult(central_value);

    // Get all systematic errors
    set<string> error_names (sys_error_names(ana));

    // Get the full set of values for each.
    for (set<string>::const_iterator e_name = error_names.begin(); e_name != error_names.end(); e_name++) {
      TH2 *errors = set_bin_values(bins, ana, *e_name, get_sys_error(*e_name));
      result->setUncertainty(e_name->c_str(), errors);

      if (is_uncorrelated(ana, *e_name))
	result->setUncorrelated(e_name->c_str());

    }

    return result;
  }

}
