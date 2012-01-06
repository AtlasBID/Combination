//
// Code to convert from the parser format into a data container.
//

#include "Combination/CDIConverter.h"
#include "Combination/BinBoundaryUtils.h"

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
      if (_axes.size() != 2)
	throw runtime_error ("Only two axes are allowed!");
      
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

  private:
    // Axis names and bin boundaries
    map<string, vector<double> > _axes;
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
    // Create the histo
    TH2 *values = bins.create_histo(hist_name);

    // Now loop over all bins in the analysis.
    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
      const CalibrationBin &bin (ana.bins[ibin]);
      pair<double, double> val_and_error (getter(bin));
      bins.set_bin_contents (values, bin.binSpec, val_and_error.first, val_and_error.second);
    }

    return values;
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
}

namespace BTagCombination {

  //
  // Master converter. Returns a data container with a set of calibrations in it.
  //

  CalibrationDataContainer *ConvertToCDI (const CalibrationAnalysis &eff, const std::string &name)
  {
    CalibrationDataHistogramContainer *result = new CalibrationDataHistogramContainer(name.c_str());

    //
    // First, convert this analysis to a histogram, and extract the values with errors
    // being the systematic errors for each value.
    //

    bin_boundaries_hist bins =  calcBoundaries(eff);
    TH2 *central_value = set_bin_values(bins, eff, "central", get_central_value);
    result->setResult(central_value);

    // Get all systematic errors
    set<string> error_names (sys_error_names(eff));

    // Get the full set of values for each.
    for (set<string>::const_iterator e_name = error_names.begin(); e_name != error_names.end(); e_name++) {
      TH2 *errors = set_bin_values(bins, eff, *e_name, get_sys_error(*e_name));
      result->setUncertainty(e_name->c_str(), errors);
    }

    return result;
  }

}
