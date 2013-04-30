//
// Code to implement the combination of bins and analyses, and the direct interface to the
// actual combination code.
//

#include "Combination/Combiner.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/BinUtils.h"
#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"
#include "Combination/CommonCommandLineUtils.h"

#include <RooRealVar.h>

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

namespace {
  using namespace BTagCombination;

  // Fill the context info for a single bin.
  void FillContextWithBinInfo (CombinationContext &ctx, const CalibrationBin &b, const string &prefix = "", const string &mname = "", bool verbose = true) {
    
    string binName (prefix + OPBinName(b));

    Measurement *m;
    if (mname.size() == 0) {
      m = ctx.AddMeasurement (binName, -10.0, 10.0, b.centralValue, b.centralValueStatisticalError);
    } else {
      m = ctx.AddMeasurement (mname, binName, -10.0, 10.0, b.centralValue, b.centralValueStatisticalError);
      if (verbose)
	cout << "--> Adding measurement called " << mname << endl;
    }

    for (unsigned int i_sys = 0; i_sys < b.systematicErrors.size(); i_sys++) {
      const SystematicError &err(b.systematicErrors[i_sys]);

      string ename(err.name);
      if (err.uncorrelated) {
	ename = string("UNCORBIN-") + ename + "-**" + binName;
      }

      m->addSystematicAbs(ename, err.value);
    }
  }

  //
  // Add all the measurements for a particular bin into the context.
  //  - Assume everythign in the bins vector is the same bin! (no x-checking).
  //  - Variable name is based on the bin name - mapping should be "obvious".
  //  - Sys errors are added as well.
  //
  void FillContextWithBinInfo (CombinationContext &ctx, const vector<CalibrationBin> &bins, const string &prefix = "")
  {
    // Simple x-checks and setup
    if (bins.size() == 0)
      return;

    // For each bin, add the info
    for (unsigned int i = 0; i < bins.size(); i++) {
      const CalibrationBin &b(bins[i]);
      FillContextWithBinInfo (ctx, b, prefix);
    }
  }

  // Fill the fitting context with a list of analyses info... it is assumed that common bins
  // in here can be fit together.
  map<string, vector<CalibrationBin> > FillContextWithCommonAnaInfo (CombinationContext &ctx, const vector<CalibrationAnalysis> &ana, const string &prefix = "", bool verbose = true)
  {
    // Sort the bins all together.
    map<string, vector<CalibrationBin> > bybins;
    for (unsigned int i_ana = 0; i_ana < ana.size(); i_ana++) {
      const CalibrationAnalysis &a(ana[i_ana]);
      for (unsigned int i_bin = 0; i_bin < a.bins.size(); i_bin++) {
	const CalibrationBin &b(a.bins[i_bin]);
	string binName(OPBinName(b));
	bybins[prefix + binName].push_back(b);
	FillContextWithBinInfo(ctx, b, prefix, OPIgnoreFormat(a, b), verbose);
      }
    }

    return bybins;
  }

  //
  // Extract the complete result - with sys errors - from the calibratoin bin
  //  - Context has already had the fit run.
  //
  CalibrationBin ExtractBinResult (const CombinationContext::FitResult &binResult, const CalibrationBin &forThisBin)
  {
    CalibrationBin result;
    result.binSpec = forThisBin.binSpec;

    result.centralValue = binResult.centralValue;
    result.centralValueStatisticalError = binResult.statisticalError;

    for (map<string, double>::const_iterator i_sys = binResult.sysErrors.begin(); i_sys != binResult.sysErrors.end(); i_sys++) {
      SystematicError e;
      e.name = i_sys->first;
      e.value = i_sys->second;
      e.uncorrelated = false;

      if (e.name.substr(0,8) == "UNCORBIN") {
	string name = e.name.substr(9);
	size_t nend = name.find("-**");
	if (nend != string::npos) {
	  e.name = name.substr(0, nend);
	  e.uncorrelated = true;
	}
      }

      if (e.value != 0.0)
	result.systematicErrors.push_back(e);

      // And the central value change associated with this systematic error
      map<string,double>::const_iterator cv_s = binResult.cvShifts.find(i_sys->first);
      if (cv_s != binResult.cvShifts.end())
	result.metadata["CV Shift " + e.name] = make_pair(cv_s->second, 0.0);
    }

    return result;
  }

  // Given a mapping of bins to analysis bins, and a set of fit results, extract the mapping
  // and return a list of combined fits.
  vector<CalibrationBin> ExtractBinsResult (const map<string, vector<CalibrationBin> > &bybins,
					    const map<string, CombinationContext::FitResult> &fitResult)
  {
    vector<CalibrationBin> result;
    for (map<string, vector<CalibrationBin> >::const_iterator i_b = bybins.begin(); i_b != bybins.end(); i_b++) {
      string binName = i_b->first;
      map<string, CombinationContext::FitResult>::const_iterator itr = fitResult.find(binName);
      if (itr == fitResult.end()) {
	ostringstream err;
	err << "Unable to recover bin " << binName << " in the output of the fit!";
	throw runtime_error (err.str().c_str());
      }

      CalibrationBin thisBin (ExtractBinResult (itr->second, i_b->second[0]));
      result.push_back(thisBin);
    }
    return result;
  }
  
  // Nice way of sorting analyses for fitting.
  typedef map<string, vector<CalibrationAnalysis> > t_anaMap;

  // Looking at sets of bin boundaries, return true if one of the sent in bins is a container of our
  // given bin.
  class ContainsBin {
  public:
    inline ContainsBin (const CalibrationBin &b)
    {
      for (size_t i = 0; i < b.binSpec.size(); i++) {
	_binB[b.binSpec[i].variable] = b.binSpec[i];
      }
    }

    bool operator() (const set<CalibrationBinBoundary> &bounds)
    {
      if (bounds.size() != _binB.size())
	return false;

      for (set<CalibrationBinBoundary>::const_iterator itr = bounds.begin(); itr != bounds.end(); itr++) {
	map<string, CalibrationBinBoundary>::const_iterator fb = _binB.find(itr->variable);
	if (fb == _binB.end())
	  return false;

	if (fb->second.lowvalue > itr->lowvalue
	    || fb->second.highvalue < itr->highvalue)
	  return false;
      }

      return true;
    }

  private:
    map<string, CalibrationBinBoundary> _binB;
  };
}

namespace BTagCombination
{

  CalibrationBin CombineBin (vector<CalibrationBin> &bins, const string &fitName)
  {
    // Simple checks to make sure we aren't bent out of shape

    if (bins.size() == 0) {
      throw runtime_error ("Unable to combine zero bins");
    }

    // If there is 1 bin we might as well short-circuit everything
    if (bins.size() == 1)
      return bins[0];

    // If there are more than 1 bins, then they all need to be identical!
    vector<CalibrationBinBoundary> specs (bins[0].binSpec);
    for (unsigned int ibin = 1; ibin < bins.size(); ibin++)
      if (!BinBoundaryUtils::compare_spec(specs, bins[ibin].binSpec))
	throw runtime_error ("Attempt to combine two bins with different boundaries");

    // Now we are ready to build the combination. Do a single fit of everything.
    CombinationContext ctx;
    FillContextWithBinInfo (ctx, bins);
    const map<string, CombinationContext::FitResult> fitResult = ctx.Fit(fitName);

    // Now that we have the result, we need to extract the numbers and build the resulting bin
    string binName (OPBinName(bins[0]));    

    
    map<string, CombinationContext::FitResult>::const_iterator ptr = fitResult.find(binName);
    if (ptr == fitResult.end())
      throw runtime_error ("Unable to find bin '" + binName + "' in the fit results");

    CalibrationBin result (ExtractBinResult (ptr->second, bins[0]));

    return result;
  }

  // Given a list of compatible analyses, combine them.
  // Compatible means matching bins and matching flavors.
  // Assume:
  //  - The bins do not overlap
  //  - The flavor is the same
  //  - The cut point is the same
  //  - The tagger is the same
  //  - The jet algorithm is the same
  CalibrationAnalysis CombineSimilarAnalyses (vector<CalibrationAnalysis> &ana)
  {
    //
    // Specal cases and input checks, and simple setup
    //

    if (ana.size() == 0)
      throw runtime_error ("Can't combine zero analyses!");

    CalibrationAnalysis result (ana[0]);
    result.name = "combined";

    if (ana.size() == 1) {
      return result;
    }
    result.bins.clear();

    //
    // Now that we know everything is the same, we can build the context
    // and do the fit.
    //

    CombinationContext ctx;
    map<string, vector<CalibrationBin> > bybins = FillContextWithCommonAnaInfo (ctx, ana);
    const map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

    //
    // Finally, go through and extract the fit results.
    //

    result.bins = ExtractBinsResult (bybins, fitResult);

    return result;
  }

  // Merge the meta data from all the analyses into the current meta data stream.
  void MergeMetadata (map<string,vector<double> > &meta, const vector<CalibrationAnalysis> &anas)
  {
    for(vector<CalibrationAnalysis>::const_iterator i_ana = anas.begin(); i_ana != anas.end(); i_ana++) {
      for(map<string,vector<double> >::const_iterator i_m = i_ana->metadata.begin(); i_m != i_ana->metadata.end(); i_m++) {
	string name (i_m->first);
	if (i_ana->name != "") {
	  name = name + " [from " + i_ana->name + "]";
	}
	string bname (name);

	int index = 0;
	while (meta.find(name) != meta.end()) {
	  if (meta[name] == i_m->second)
	    break;
	  index++;
	  ostringstream n;
	  n << bname << " " << index;
	  name = n.str();
	}
	meta[name] = i_m->second;
      }
    }
  }

  // We plunk everythign we are given here into a single context, and return the new
  // fit.
  CalibrationAnalysis CombineAnalysesInOneContext(const vector<CalibrationAnalysis> &anas,
				   const vector<AnalysisCorrelation> &correlations,
				   const string &resultFitName,
				   bool verbose)
  {
    CombinationContext ctx;
    ctx.SetVerbose(verbose);
    map<string, vector<CalibrationBin> > bins =  FillContextWithCommonAnaInfo(ctx, anas, "", verbose);
      
    // We make an assumption about the fit name here, and the way the fit is being done (const over flavor, tag, OP).
    string fitName = anas[0].flavor
      + ":" + anas[0].tagger
      + ":" + anas[0].operatingPoint;

    // Now, go look for any correlations that might apply here.
    for (size_t i_cor = 0; i_cor < correlations.size(); i_cor++) {
      for (size_t i_cbin = 0; i_cbin < correlations[i_cor].bins.size(); i_cbin++) {
	const BinCorrelation &bin(correlations[i_cor].bins[i_cbin]);
	pair<string, string> aNames (OPIgnoreCorrelatedFormat(correlations[i_cor], bin));

	Measurement *m1 = ctx.FindMeasurement(aNames.first);
	Measurement *m2 = ctx.FindMeasurement(aNames.second);

	if (m1 == 0 || m2 == 0) {
	  if (!(m1 == 0 && m2 == 0)) {
	    ostringstream out;
	    out << "Both analyses not present for correlation " << OPFullName(correlations[i_cor]) << " - but at least one is!";
	    throw runtime_error (out.str());
	  }
	  continue;
	}

	if (verbose)
	  cout << "--> Adding correlation " << OPIgnoreFormat(correlations[i_cor], bin)
	       << std::endl;

	//
	// Now, do the correlations
	//

	if (bin.hasStatCorrelation) {
	  ctx.AddCorrelation("statistical", m1, m2, bin.statCorrelation);
	}
      }
    }
      
    // Do the fit.
    map<string, CombinationContext::FitResult> fitResult = ctx.Fit(fitName);
    CombinationContext::ExtraFitInfo extraInfo = ctx.GetExtraFitInformation();

    // Dummy analysis that we will fill in with the results.
    CalibrationAnalysis r(anas[0]);
    r.name = resultFitName;

    r.bins = ExtractBinsResult(bins, fitResult);
    r.metadata.clear();
    r.metadata["gchi2"].push_back(extraInfo._globalChi2);
    r.metadata["gndof"].push_back(extraInfo._ndof);
    for (map<string,double>::const_iterator i_p = extraInfo._pulls.begin(); i_p != extraInfo._pulls.end(); i_p++) {
      r.metadata[string("Pull ") + i_p->first].push_back(i_p->second);
    }
    for (map<string,pair<double,double> >::const_iterator i_p = extraInfo._nuisance.begin(); i_p != extraInfo._nuisance.end(); i_p++) {
      r.metadata[string("Nuisance ") + i_p->first].push_back(i_p->second.first);
      r.metadata[string("Nuisance ") + i_p->first].push_back(i_p->second.second);
    }

    MergeMetadata (r.metadata, anas);
    return r;
  }

  // Do the combination, doing everythign accross bins.
  vector<CalibrationAnalysis> CombineAnalysesAllBins (const CalibrationInfo &info, bool verbose)
  {
    t_anaMap binnedAnalyses (BinAnalysesByJetTagFlavOp(info.Analyses));

    //
    // in each bin, fit everything. one odd thing is we have to loop through all
    // the correlations and extract any we need.
    //

    vector<CalibrationAnalysis> result;
    for(t_anaMap::const_iterator i_ana = binnedAnalyses.begin(); i_ana != binnedAnalyses.end(); i_ana++) {
      if (i_ana->second.size() > 1) {
	CalibrationAnalysis r (CombineAnalysesInOneContext(i_ana->second,
							   info.Correlations,
							   info.CombinationAnalysisName,
							   verbose));

	result.push_back(r);
      }
    }

    return result;
  }

  // Merge the resulting analyses. Assume all bins are mutually exclusive, undefine result
  // if that isn't the case!
  CalibrationAnalysis MergeAnalyses(const vector<CalibrationAnalysis> &anas, string anaName)
  {
    CalibrationAnalysis ana(anas[0]);
    ana.bins.clear();
    ana.metadata.clear();
    ana.name = anaName;

    for (vector<CalibrationAnalysis>::const_iterator itr = anas.begin(); itr != anas.end(); itr++) {
      for (vector<CalibrationBin>::const_iterator i_bin = itr->bins.begin(); i_bin != itr->bins.end(); i_bin++) {
	ana.bins.push_back(*i_bin);
      }
    }
    MergeMetadata(ana.metadata, anas);

    return ana;
  }

  // Do the fits bin-by-bin.
  vector<CalibrationAnalysis> CombineAnalysesByBin (const CalibrationInfo &info, bool verbose)
  {
    // Split this list of analyses by bin, do the fit, and then recombine.
    t_anaMap analysesInCommon (BinAnalysesByJetTagFlavOp(info.Analyses));
    vector<CalibrationAnalysis> result;
    for(t_anaMap::const_iterator i_ana = analysesInCommon.begin(); i_ana != analysesInCommon.end(); i_ana++) {
      if (i_ana->second.size() > 1) {
	set<set<CalibrationBinBoundary> > allBins (listAllBins(i_ana->second));
	vector<CalibrationAnalysis> binByBinFits;
	for (set<set<CalibrationBinBoundary> >::const_iterator i_bin = allBins.begin(); i_bin != allBins.end(); i_bin++) {
	  vector<CalibrationAnalysis> anaForBin (removeAllBinsButBin(i_ana->second, *i_bin));
	  
	  CalibrationAnalysis r (CombineAnalysesInOneContext(anaForBin,
							     info.Correlations,
							     OPBinName(*i_bin),
							     verbose));
	  binByBinFits.push_back(r);
	}
	result.push_back(MergeAnalyses(binByBinFits, info.CombinationAnalysisName));
      }
    }    

    return result;
  }

  //
  // Master entry to do the fitting. Shell routine that calls out depending on the type of fit
  // desired.
  //
  vector<CalibrationAnalysis> CombineAnalyses (const CalibrationInfo &info, bool verbose, CombinationType combineType)
  {
    switch(combineType) {
    case kCombineByFullAnalysis:
      return CombineAnalysesAllBins(info, verbose);

    case kCombineBySingleBin:
      return CombineAnalysesByBin (info, verbose);

    default:
      throw runtime_error ("Unknown combination type!");
      break;
    }
  }

  //
  // Combine bins in a single analysis to generate a new analysis.
  // - Can't split bins
  // - All template bins must be fully covered by the analysis bins.
  // - Fit is done seperately in each bin.
  //
  CalibrationAnalysis RebinAnalysis (const set<set<CalibrationBinBoundary> > &templateBinning,
				     const CalibrationAnalysis &ana)
  {
    // Do quick checks to make sure inputs look basically good.

    if (templateBinning.size() == 0)
      throw runtime_error ("Can't rebin analysis if there are not bins in the template!");

    if (ana.bins.size() == 0)
      throw runtime_error ("Unable to rebin an empty analysis!");

    // We need to associate bins in the source analysis with the targets. We create a map and look
    // for completely contained bins.

    map<set<CalibrationBinBoundary>, vector<CalibrationBin> > matchedBins;
    for (set<set<CalibrationBinBoundary> >::const_iterator itr = templateBinning.begin(); itr != templateBinning.end(); itr++) {
      matchedBins[*itr] = vector<CalibrationBin>();
    }

    for (size_t i_bin = 0; i_bin < ana.bins.size(); i_bin++) {
      set<set<CalibrationBinBoundary> >::const_iterator foundBin =
	find_if (templateBinning.begin(), templateBinning.end(), ContainsBin(ana.bins[i_bin]));

      if (foundBin == templateBinning.end()) {
	ostringstream err;
	err << "Bin " << ana.bins[i_bin] << "is not contained by any template bins";
	throw runtime_error (err.str().c_str());
      }

    }

    return ana;
  }  

}

