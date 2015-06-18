//
// Code to convert from the parser format into a data container.
//

#include "Combination/CDIConverter.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/CalibrationDataModelStreams.h"
#include "Combination/FitLinage.h"

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
using Analysis::CalibrationDataMappedHistogramContainer;
using Analysis::CalibrationDataContainer;

namespace ph = std::placeholders;

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
		       const Pred &getter,
		       bool extendedOK = false)
  {
    try {
      // Create the histo
      TH2 *values = bins.create_histo(hist_name);

      // Now loop over all bins in the analysis.
      for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++) {
	const CalibrationBin &bin (ana.bins[ibin]);
	if (extendedOK || !bin.isExtended) {
	  pair<double, double> val_and_error (getter(bin));
	  bins.set_bin_contents (values, bin.binSpec, val_and_error.first, val_and_error.second);
	}
      }

      return values;
    } catch (bad_cdi_config_exception &e) {
      ostringstream msg;
      msg << "Error while processing analysis: " << e.what() << endl
	  << ana;
      throw  bad_cdi_config_exception(msg.str().c_str());
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

  // Get the extrapolation error - which is whatever the sys error is.
  pair<double, double> get_extrapolation_error (const CalibrationBin &bin)
  {
    if (bin.isExtended) {
      if (bin.systematicErrors.size() != 1) {
	throw bad_cdi_config_exception ("Extrapolation bin does not have exactly one systematic error");
      }
      double v = bin.systematicErrors[0].value;
      return make_pair(v, v);
    } else {
      return make_pair((double)0.0, (double)0.0);
    }
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

      // Ignore extended bins.
      if (bin.isExtended)
	continue;

      transform (bin.systematicErrors.begin(), bin.systematicErrors.end(),
		 inserter(result, result.begin()),
		 bind<const string&>(&SystematicError::name, ph::_1));
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

  CalibrationAnalysis addTotalSysError (const CalibrationAnalysis &eff)
  {
    CalibrationAnalysis ana (eff);
    for (vector<CalibrationBin>::iterator itr = ana.bins.begin(); itr != ana.bins.end(); itr++) {
      // Ignore extended guys. They should already be converted
      if (itr->isExtended)
	continue;

      // Total up all systematics
      double totS = 0.0;
      for (vector<SystematicError>::const_iterator i_s = itr->systematicErrors.begin(); i_s != itr->systematicErrors.end(); i_s++) {
	totS += i_s->value*i_s->value;
      }
      SystematicError s;
      s.value = sqrt(totS);
      s.name = "systematics";
      itr->systematicErrors.push_back(s);
    }
    return ana;
  }

  //
  // Convert to the CDI format using regular bins - that is, everythign is layed out as a simple
  // grid.
  //
  CalibrationDataContainer *ConvertToCDIRegularBins (const CalibrationAnalysis &eff, const std::string &name)
  {
    CalibrationDataHistogramContainer *result = new CalibrationDataHistogramContainer(name.c_str());

    // grab the hadronization setting...
    map<string,string>::const_iterator hadFind = eff.metadata_s.find("Hadronization");
    string hadronization ("unknown");
    if (hadFind == eff.metadata_s.end()) {
      cout << "Warning: Analysis " << eff.name << " has no hadronization setting." << endl;
      cout << "  -> Hadronization has been left unset for this calibration!" << endl;
    } else {
      hadronization = hadFind->second;
    }
    result->setHadronisation(hadronization);

    // For the comment, set it to the complete history of how it was built.

    result->setComment(Linage(eff));

    //
    // We need to have a total systematic uncertianty in the CDI. So we need to tally it up.
    //

    CalibrationAnalysis ana(addTotalSysError(eff));

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

    // If there are any extrapolation bins, then set that too.
    bin_boundaries_hist ebins = calcBoundaries(ana, false);
    TH2 *extrap_errors = set_bin_values(ebins, ana, "extrap", get_extrapolation_error, true);
    if (extrap_errors->GetMaximum() > 0.0) {
      result->setUncertainty("extrapolation", extrap_errors);
    } else {
      delete extrap_errors;
    }

    return result;
  }

  //
  // Track the bin mappings.
  //
  class binMapper {
  public:
    // Get everything setup for inserting this analysis.
    binMapper (const CalibrationAnalysis &eff, CalibrationDataMappedHistogramContainer *c)
      : _container(c)
    {
      _nbin = eff.bins.size();

      // Fetch out all the variable names that we are going to be usig
      set<string> bin_names;
      for (size_t ib = 0; ib < eff.bins.size(); ib++) {
	for (size_t ibb = 0; ibb < eff.bins[ib].binSpec.size(); ibb++) {
	  bin_names.insert(eff.bins[ib].binSpec[ibb].variable);
	}
      }
      vector<string> vbin_names(bin_names.begin(), bin_names.end());
      for (size_t ib = 0; ib < vbin_names.size(); ib++) {
	_bin_lookup[vbin_names[ib]] = int(ib);
      }

      c->setMappedVariables(vbin_names);
    }

    // Return a bin index for this bin coordinate.
    int getBin(const vector<CalibrationBinBoundary> &binCoord)
    {
      // Simple check.
      const size_t maxCoord (10);
      if (binCoord.size() > maxCoord)
	throw runtime_error ("More coordinates that we can deal with - rebuild for more than 10!");

      // Put them in a set so we can sort them (they are sorted first by variable, then coordinates, see
      // operator< in Parser.h

      set<CalibrationBinBoundary> items(binCoord.begin(), binCoord.end());
      int index = 0;
      double low[maxCoord];
      double high[maxCoord];
      for (size_t i = 0; i < maxCoord; i++)
	low[i] = high[i] = 0.0;

      for (set<CalibrationBinBoundary>::const_iterator itr = items.begin(); itr != items.end(); itr++) {
	low[_bin_lookup[itr->variable]] = itr->lowvalue;
	high[_bin_lookup[itr->variable]] = itr->highvalue;
	index++;
      }

      CalibrationDataMappedHistogramContainer::Bin b(items.size(), low, high);
      return _container->addBin(b);
    }

    int numberBins() const {
      return _nbin;
    }

  private:
    int _nbin;
    CalibrationDataMappedHistogramContainer *_container;
    map<string, int> _bin_lookup;
  };

  // Simple TH1F bin holder just makes the code below make more "sense".
  class TH1FHolder {
  public:
    TH1FHolder() 
      : _h(0)
    {}

    class Binner {
    public:
      Binner (TH1F *h)
	: _h(h)
      {}

      void set (int bin, double val, double err = 0.0) {
	_h->SetBinContent(bin, val);
	_h->SetBinError(bin, err);
      }

    private:
      TH1F *_h;
    };

    Binner operator() (binMapper &bm) {
      if (_h == 0) {
	_h = new TH1F("values", "some values;mapped", bm.numberBins(), 0.0, double(bm.numberBins()));
      }
      return Binner(_h);
    }

    operator TH1F*() const {
      if (_h == 0)
	throw runtime_error ("Attempt to get histo pointer before we have created it!");
      return _h;
    }

  private:
    TH1F* _h;
  };

  //
  // Call this when we should expect irregular bins, so we need to do the binning
  // using the new Bin constructs.
  //
  CalibrationDataContainer *ConvertToCDIIrregularBins (const CalibrationAnalysis &eff, const std::string &name)
  {
    // We can't currently deal with extension bins, so fail.
    for (vector<CalibrationBin>::const_iterator itr = eff.bins.begin(); itr != eff.bins.end(); itr++) {
      if (itr->isExtended) {
	ostringstream err;
	err << "The CDI does not support extrapolation bins when irregular binning exists (analysis "
	    << eff.name 
	    << " - " 
	    << name 
	    << ")";
	throw bad_cdi_config_exception(err.str());
      }
    }

    //
    // We need to have a total systematic uncertianty in the CDI. So we need to tally it up.
    //

    CalibrationAnalysis ana(addTotalSysError(eff));

    //
    // Setup.
    //

    CalibrationDataMappedHistogramContainer *result = new CalibrationDataMappedHistogramContainer(name.c_str());
    binMapper mapper (ana, result);

    //
    // Next, loop through each bin of this analysis and extract all the info.
    //

    map<string, TH1FHolder> mapped_histograms;
    for (size_t ibin = 0; ibin < ana.bins.size(); ibin++) {
      const CalibrationBin &b(ana.bins[ibin]);

      int binIndex = mapper.getBin(b.binSpec);

      mapped_histograms["central"](mapper).set(binIndex, b.centralValue, b.centralValueStatisticalError);

      for (size_t isys = 0; isys < b.systematicErrors.size(); isys++) {
	const SystematicError &e(b.systematicErrors[isys]);
	mapped_histograms[e.name](mapper).set(binIndex, e.value);
      }
    }

    //
    // Finally, stuff this back into the main container.
    //

    for (map<string, TH1FHolder>::const_iterator itr = mapped_histograms.begin(); itr != mapped_histograms.end(); itr++) {
      if (itr->first == "central") {
	result->setResult(itr->second);
      } else {
	result->setUncertainty(itr->first, itr->second);
	if (is_uncorrelated(ana, itr->first))
	  result->setUncorrelated(itr->first.c_str());
      }
    }

    return result;
  }
}

namespace BTagCombination {

  //
  // Master converter. Returns a data container with a set of calibrations in it.
  // Use the most efficient method of conversion we can.
  //
  CalibrationDataContainer *ConvertToCDI (const CalibrationAnalysis &eff, const std::string &name)
  {
    // Try the regular flat/grid binning first.

    string binError;
    try {
      return ConvertToCDIRegularBins(eff, name);
    } catch (bin_boundary_error &e) {
      binError = e.what();
    }
      
    // If we are here, then the irregular binning is the only hope.

    try {
      return ConvertToCDIIrregularBins(eff, name);
    } catch (exception &e) {
      ostringstream err;
      err << "Error while trying to convert to CDI analysis " << eff.name << "." << endl
	  << "  Failed to do a regular size bins: " << binError << endl
	  <<  " Failed to do an irregular bin size: " << e.what() << endl;
      cerr << err.str() << endl;
      throw;
    }
  }
}
