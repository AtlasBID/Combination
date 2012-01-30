///
/// Code to dump plots of the input and output analyses to a
/// root file.
///
#include "Combination/Plots.h"
#include "Combination/CommonCommandLineUtils.h"

#include "TGraphErrors.h"
#include "TCanvas.h"

#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;

namespace {
  using namespace BTagCombination;

  typedef map<string, vector<CalibrationAnalysis> > t_grouping;
  typedef set<CalibrationBinBoundary> t_BBSet;
  typedef map<string, t_BBSet> t_BinSet;

  ///
  /// Many of these routines could be moved into the global namespace if their
  /// utility was needed.
  ///

  // Find the bin results for a partiuclar bin. Return an empty if we
  // can't find it.
  CalibrationBin FindBin (const CalibrationAnalysis &ana, t_BBSet &bininfo)
  {
    for (unsigned int ib = 0; ib < ana.bins.size(); ib++) {
      const CalibrationBin &cb (ana.bins[ib]);
      t_BBSet t (cb.binSpec.begin(), cb.binSpec.end());
      if (t == bininfo)
	return cb;
    }
    return CalibrationBin();
  }

  ///
  /// Actually generate the plots for a particular bin
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas,
				    const t_BBSet &specifiedBins,
				    const t_BBSet &axisBins)
  {
    cout << "**Starting on bin " << axisBins.begin()->variable << "(# bins " << axisBins.size() << ")" << endl;
    // Extract all the results.

    typedef map<string, CalibrationBin> t_CBMap;
    typedef map<CalibrationBinBoundary, t_CBMap> t_BoundaryMap;
    t_BoundaryMap taggerResults;
    for (t_BBSet::const_iterator ib = axisBins.begin(); ib != axisBins.end(); ib++) {
      t_BBSet coordinate (specifiedBins);
      coordinate.insert(*ib);
      for(unsigned int ia = 0; ia < anas.size(); ia++) {
	taggerResults[*ib][anas[ia].name] = FindBin (anas[ia], coordinate);
      }
    }

    // For each analysis make a graf. We assume copy symantics for the values
    // we pass to the TGraph.
    vector<TGraphErrors*> plots;
    Double_t *v_bin = new Double_t[axisBins.size()];
    Double_t *v_binError = new Double_t[axisBins.size()];
    Double_t *v_central = new Double_t[axisBins.size()];
    Double_t *v_centralStatError = new Double_t[axisBins.size()];
    Double_t *v_centralTotError = new Double_t[axisBins.size()];
    for (unsigned int ia = 0; ia < anas.size(); ia++) {
      const string &anaName (anas[ia].name);
      cout << "** Doing analysis " << anaName << endl;
      int ibin = 0;
      for (t_BoundaryMap::const_iterator i_c = taggerResults.begin(); i_c != taggerResults.end(); i_c++) {
	v_bin[ibin] = 0.5 + ibin;
	v_binError[ibin] = 0; // No error along x-axis!!
	const CalibrationBin &cb(i_c->second.find(anaName)->second);
	v_central[ibin] = cb.centralValue;
	v_centralStatError[ibin] = cb.centralValueStatisticalError;
	v_centralTotError[ibin] = cb.centralValueStatisticalError;

	ibin++;
      }

      TGraphErrors *g = new TGraphErrors (axisBins.size(),
				    v_bin, v_central,
				    v_binError, v_centralStatError);
      g->SetName(anaName.c_str());
      g->SetTitle(anaName.c_str());
      plots.push_back(g);
    }

    delete[] v_bin;
    delete[] v_binError;
    delete[] v_central;
    delete[] v_centralStatError;
    delete[] v_centralTotError;

    // Now build the canvas that we are going to store.
    TCanvas *c = new TCanvas ("EffInfo");
    for (unsigned int i_p = 0; i_p < plots.size(); i_p++) {
      plots[i_p]->Draw("A*");
    }
    out->Add(c);
    out->Write();
  }

  ///
  /// Recursive routien to look at all the variables and walk down until there
  /// is just one left for the axis.
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas,
				    const string &binName,
				    const t_BinSet &binSet,
				    const t_BBSet &otherBins = t_BBSet())
  {
    cout << "Looking to do bin " << binName << "(already done " << otherBins.size() << ")" << endl;

    //
    // Are we at the last variable - which will be the axis of a plot?
    //

    if (otherBins.size() + 1 == binSet.size()) {
      cout << "  -> time to plot" << endl;
      GenerateCommonAnalysisPlots (out, anas,
				   otherBins,
				   binSet.find(binName)->second);
      return;
    }

    //
    // Not at the last variable, so now we pick off the next variable,
    // create directories, and then find the next variable.
    //

    set<string> allBins;
    set<string> alreadyBins;
    for (t_BinSet::const_iterator i = binSet.begin(); i != binSet.end(); i++) {
      allBins.insert(i->first);
    }
    for (t_BBSet::const_iterator i = otherBins.begin(); i != otherBins.end(); i++) {
      alreadyBins.insert(i->variable);
    }
    alreadyBins.insert(binName);
    vector<string> leftToDoBins;
    set_difference(allBins.begin(), allBins.end(),
		   alreadyBins.begin(), alreadyBins.end(),
		   back_inserter(leftToDoBins));
    string nextBin (leftToDoBins[0]);

    t_BBSet bounds(binSet.find(binName)->second);
    for (t_BBSet::const_iterator i = bounds.begin(); i != bounds.end(); i++) {
      ostringstream buf;
      buf << *i;
      TDirectory *r = out->mkdir(buf.str().c_str());

      t_BBSet otherBinsNext(otherBins);
      otherBinsNext.insert(*i);
      GenerateCommonAnalysisPlots (r, anas,
				   nextBin, binSet,
				   otherBinsNext);
    }
  }

  ///
  /// Split plots up by binning
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas)
  {
    // Get a list of all the axes that we have for bins, and the binning.
    t_BinSet binAxes;
    for (unsigned int i = 0; i < anas.size(); i++) {
      for (unsigned int i_b = 0; i_b < anas[i].bins.size(); i_b++) {
	const CalibrationBin &bin(anas[i].bins[i_b]);
	for (unsigned int i_c = 0; i_c < bin.binSpec.size(); i_c++) {
	  const CalibrationBinBoundary bb (bin.binSpec[i_c]);
	  binAxes[bb.variable].insert(bb);
	}
      }
    }

    //
    // Now, choose an axes to "freeze" and go down a level...
    //
    
    for (t_BinSet::const_iterator i = binAxes.begin(); i != binAxes.end(); i++) {
      TDirectory *r = out->mkdir(i->first.c_str());
      GenerateCommonAnalysisPlots (r, anas, i->first, binAxes);
    }
  }
}

namespace BTagCombination {

  ///
  /// Dump all the plots to an output directory.
  ///
  void DumpPlots (TDirectory *output, const vector<CalibrationAnalysis> &anas)
  {
    //
    // Group the analyses together so we put the proper things on
    // each plot.
    //

    t_grouping grouping;
    for (unsigned int i = 0; i < anas.size(); i++) {
      grouping[OPIndependentName(anas[i])].push_back(anas[i]);
    }

    //
    // Dump each
    //

    for (t_grouping::const_iterator i = grouping.begin(); i != grouping.end(); i++) {
      TDirectory *r = output->mkdir(i->first.c_str());
      GenerateCommonAnalysisPlots (r, i->second);
    }
  }

}
