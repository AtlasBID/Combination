//
// Code to implement the combination of bins and analyses, and the direct interface to the
// actual combination code.
//

#include "Combination/Combiner.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"
#include "Combination/CommonCommandLineUtils.h"

#include <RooRealVar.h>

#include <stdexcept>
#include <sstream>

using namespace std;

namespace {
  using namespace BTagCombination;

  // Fill the context info for a single bin.
  void FillContextWithBinInfo (CombinationContext &ctx, const CalibrationBin &b, const string &prefix = "", const string &mname = "") {
    
    string binName (prefix + OPBinName(b));

    Measurement *m;
    if (mname.size() == 0) {
      m = ctx.AddMeasurement (binName, -1.0, 2.0, b.centralValue, b.centralValueStatisticalError);
    } else {
      m = ctx.AddMeasurement (mname, binName, -1.0, 2.0, b.centralValue, b.centralValueStatisticalError);
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
  map<string, vector<CalibrationBin> > FillContextWithCommonAnaInfo (CombinationContext &ctx, const vector<CalibrationAnalysis> &ana, const string &prefix = "")
  {
    // Sort the bins all together.
    map<string, vector<CalibrationBin> > bybins;
    for (unsigned int i_ana = 0; i_ana < ana.size(); i_ana++) {
      const CalibrationAnalysis &a(ana[i_ana]);
      for (unsigned int i_bin = 0; i_bin < a.bins.size(); i_bin++) {
	const CalibrationBin &b(a.bins[i_bin]);
	string binName(OPBinName(b));
	bybins[prefix + binName].push_back(b);
	FillContextWithBinInfo(ctx, b, prefix, OPIgnoreFormat(a, b));
      }
    }

    return bybins;
  }

  //
  // Extract the complete result - with sys errors - from the calibratoin bin
  //  - Context has already had the fit run.
  //
  CalibrationBin ExtractBinResult (CombinationContext::FitResult binResult, const CalibrationBin &forThisBin)
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
    }

    return result;
  }

  // Given a mapping of bins to analysis bins, and a set of fit results, extract the mapping
  // and return a list of combined fits.
  vector<CalibrationBin> ExtractBinsResult (const map<string, vector<CalibrationBin> > &bybins,
					    map<string, CombinationContext::FitResult> &fitResult)
  {
    vector<CalibrationBin> result;
    for (map<string, vector<CalibrationBin> >::const_iterator i_b = bybins.begin(); i_b != bybins.end(); i_b++) {
      string binName = i_b->first;
      CalibrationBin thisBin (ExtractBinResult (fitResult[binName], i_b->second[0]));
      result.push_back(thisBin);
    }
    return result;
  }

}

namespace BTagCombination
{

  CalibrationBin CombineBin (vector<CalibrationBin> &bins)
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
    map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

    // Now that we have the result, we need to extract the numbers and build the resulting bin
    string binName (OPBinName(bins[0]));    

    if (fitResult.find(binName) == fitResult.end())
      throw runtime_error ("Unable to find bin '" + binName + "' in the fit results");

    CalibrationBin result (ExtractBinResult (fitResult[binName], bins[0]));

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
    map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

    //
    // Finally, go through and extract the fit results.
    //

    result.bins = ExtractBinsResult (bybins, fitResult);

    return result;
  }

  // Given a list of analyses, sort them, and combine everything, and return
  // all the new ones.
  vector<CalibrationAnalysis> CombineAnalyses (const CalibrationInfo &info)
  {
    //
    // First step, divide up the analyses by fittable catagory
    //

    typedef map<string, vector<CalibrationAnalysis> > t_anaMap;
    t_anaMap binnedAnalyses (BinAnalysesByJetTagFlavOp(info.Analyses));

    //
    // in each bin, fit everything. one odd thing is we have to loop through all
    // the correlations and extract any we need.
    //

    vector<CalibrationAnalysis> result;
    for(t_anaMap::const_iterator i_ana = binnedAnalyses.begin(); i_ana != binnedAnalyses.end(); i_ana++) {
      CombinationContext ctx;
      map<string, vector<CalibrationBin> > bins =  FillContextWithCommonAnaInfo(ctx, i_ana->second);
      
      // Now, go look for any correlations that might apply here.
      for (size_t i_cor = 0; i_cor < info.Correlations.size(); i_cor++) {
	for (size_t i_cbin = 0; i_cbin < info.Correlations[i_cor].bins.size(); i_cbin++) {
	  const BinCorrelation &bin(info.Correlations[i_cor].bins[i_cbin]);
	  pair<string, string> aNames (OPIgnoreCorrelatedFormat(info.Correlations[i_cor],
								bin));

	  cout << "Looking at correlation " << OPIgnoreFormat(info.Correlations[i_cor], bin)
	       << std::endl;
	  cout << "  name1 = " << aNames.first << endl;
	  cout << "  name2 = " << aNames.second << endl;	  

	  Measurement *m1 = ctx.FindMeasurement(aNames.first);
	  Measurement *m2 = ctx.FindMeasurement(aNames.second);

	  cout << "  m1 = " << m1 << " m2 = " << m2 << endl;

	  if (m1 == 0 || m2 == 0) {
	    if (!(m1 == 0 && m2 == 0)) {
	      ostringstream out;
	      out << "Both analyses not present for correlation " << OPFullName(info.Correlations[i_cor]) << " - but at least one is!";
	      throw runtime_error (out.str());
	    }
	    continue;
	  }

	  //
	  // Now, do the correlations
	  //

	  if (bin.hasStatCorrelation) {
	    cout << "  Stat Correlation is " << bin.statCorrelation << endl;
	    ctx.AddCorrelation("statistical", m1, m2, bin.statCorrelation);
	  }
	}
      }
      
      // Do the fit.
      map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

      // Dummy analysis that we will fill in with the results.
      CalibrationAnalysis r(i_ana->second[0]);
      r.name = "combined";
      r.bins = ExtractBinsResult(bins, fitResult);
      result.push_back(r);
    }

#ifdef notyet

    // First by jet algorithm
    typedef map<string, vector<CalibrationAnalysis> > CalibMap;
    map<string, map<string, vector<CalibrationBin> > > binByBinMap;
    CalibMap byJetAlg;
    for (unsigned int i = 0; i < ana.size(); i++) {
      byJetAlg[ana[i].jetAlgorithm].push_back(ana[i]);
    }

    for (CalibMap::const_iterator i_jet = byJetAlg.begin(); i_jet != byJetAlg.end(); i_jet++) {
      string n_jetalg = i_jet->first;
      // Next, by flavor
      CalibMap byFlavor;
      for (unsigned int i = 0; i < i_jet->second.size(); i++) {
	byFlavor[i_jet->second[i].flavor].push_back(i_jet->second[i]);
      }
      
      for (CalibMap::const_iterator i_jet = byFlavor.begin(); i_jet != byFlavor.end(); i_jet++) {
	string n_flavor = i_jet->first;
	// Next, by tagger
	CalibMap byTagger;
	for (unsigned int i = 0; i < i_jet->second.size(); i++) {
	  byTagger[i_jet->second[i].tagger].push_back(i_jet->second[i]);
	}
      
	for (CalibMap::const_iterator i_jet = byTagger.begin(); i_jet != byTagger.end(); i_jet++) {
	  string n_tagger = i_jet->first;
	  // Next, by cut point
	  CalibMap byOP;
	  for (unsigned int i = 0; i < i_jet->second.size(); i++) {
	    byOP[i_jet->second[i].operatingPoint].push_back(i_jet->second[i]);
	  }
      
	  for (CalibMap::const_iterator i_jet = byOP.begin(); i_jet != byOP.end(); i_jet++) {
	    string n_OP = i_jet->first;
	    ostringstream prefix;
	    prefix << n_jetalg
		   << "-" << n_flavor
		   << "-" << n_tagger
		   << "-" << n_OP;
	    // Now we can actually build the fit context
	    cout << "Fitting " << prefix.str() << endl;
	  }
	}
      }
    }

    //
    // Do the fit
    //

    map<string, CombinationContext::FitResult> fitResult = ctx.Fit();

    //
    // Extract the results... Should be one per group of analyses we inserted.
    //

    for (CalibMap::const_iterator i_jet = byJetAlg.begin(); i_jet != byJetAlg.end(); i_jet++) {
      string n_jetalg = i_jet->first;
      // Next, by flavor
      CalibMap byFlavor;
      for (unsigned int i = 0; i < i_jet->second.size(); i++) {
	byFlavor[i_jet->second[i].flavor].push_back(i_jet->second[i]);
      }
      
      for (CalibMap::const_iterator i_jet = byFlavor.begin(); i_jet != byFlavor.end(); i_jet++) {
	string n_flavor = i_jet->first;
	// Next, by tagger
	CalibMap byTagger;
	for (unsigned int i = 0; i < i_jet->second.size(); i++) {
	  byTagger[i_jet->second[i].tagger].push_back(i_jet->second[i]);
	}
      
	for (CalibMap::const_iterator i_jet = byTagger.begin(); i_jet != byTagger.end(); i_jet++) {
	  string n_tagger = i_jet->first;
	  // Next, by cut point
	  CalibMap byOP;
	  for (unsigned int i = 0; i < i_jet->second.size(); i++) {
	    byOP[i_jet->second[i].operatingPoint].push_back(i_jet->second[i]);
	  }
      
	  for (CalibMap::const_iterator i_jet = byOP.begin(); i_jet != byOP.end(); i_jet++) {
	    string n_OP = i_jet->first;
	    ostringstream prefix;
	    prefix << n_jetalg
		   << "-" << n_flavor
		   << "-" << n_tagger
		   << "-" << n_OP;

	    // Extract the result
	    CalibrationAnalysis example (i_jet->second[0]);
	    map<string, vector<CalibrationBin> > &byBin (binByBinMap[prefix.str()]);
	    example.name = "combined";
	    example.bins = ExtractBinsResult(byBin, fitResult);
	    result.push_back(example);
	  }
	}
      }
    }
#endif

    return result;
  }

}

