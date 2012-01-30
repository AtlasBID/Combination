///
/// Code to dump plots of the input and output analyses to a
/// root file.
///
#include "Combination/Plots.h"
#include "Combination/CommonCommandLineUtils.h"

#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TH1F.h"

#include <map>
#include <cmath>
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

  //
  // Calc the total error for this bin.
  //
  double CalcTotalError (const CalibrationBin &b)
  {
    double acc = b.centralValueStatisticalError;
    acc = acc*acc;

    for (unsigned int i = 0; i < b.systematicErrors.size(); i++) {
      double s = b.systematicErrors[i].value;
      acc += s*s;
    }

    return sqrt(acc);
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

    //
    // We need to put each analysis at a point along the x-axis, and have it at
    // different places along the x-axis. This calc is a bit messy, as a result.
    //

    double x_InitialCoordinate = 0.5;
    double x_DeltaCoordinate = 0.0;
    if (anas.size() > 1) {
      x_DeltaCoordinate = 0.7 / ((double)anas.size());
      if (x_DeltaCoordinate > 0.1)
	x_DeltaCoordinate = 0.1;
      x_InitialCoordinate = 0.5 - x_DeltaCoordinate * anas.size() / 2.0;
    }

    // For each analysis make a graf. We assume copy symantics for the values
    // we pass to the TGraph.

    vector<TGraphErrors*> plots, plotsSys;
    vector<string> binlabels;
    Double_t *v_bin = new Double_t[axisBins.size()];
    Double_t *v_binError = new Double_t[axisBins.size()];
    Double_t *v_central = new Double_t[axisBins.size()];
    Double_t *v_centralStatError = new Double_t[axisBins.size()];
    Double_t *v_centralTotError = new Double_t[axisBins.size()];

    double yCentralTotMin = 10.0;
    double yCentralTotMax = 0.0;
    for (unsigned int ia = 0; ia < anas.size(); ia++) {
      const string &anaName (anas[ia].name);
      cout << "** Doing analysis " << anaName << endl;
      cout << "   Initial coordinate: " << x_InitialCoordinate << endl;
      int ibin = 0;

      for (t_BoundaryMap::const_iterator i_c = taggerResults.begin(); i_c != taggerResults.end(); i_c++) {
	v_bin[ibin] = x_InitialCoordinate + ibin;
	v_binError[ibin] = 0; // No error along x-axis!!
	const CalibrationBin &cb(i_c->second.find(anaName)->second);
	v_central[ibin] = cb.centralValue;
	v_centralStatError[ibin] = cb.centralValueStatisticalError;
	v_centralTotError[ibin] = CalcTotalError(cb);

	// Record min and max for later use with limit setting
	double t = v_central[ibin] - v_centralTotError[ibin];
	if (t != 0.0 && yCentralTotMin > t)
	  yCentralTotMin = t;
	t = v_central[ibin] + v_centralTotError[ibin];
	if (t != 0.0 && yCentralTotMax < t)
	  yCentralTotMax = t;

	// Fill in the axis labels if this is our first time through.
	if (ia == 0) {
	  ostringstream buf;
	  buf << cb;
	  binlabels.push_back(buf.str());
	}

	ibin++;
      }

      TGraphErrors *g = new TGraphErrors (axisBins.size(),
				    v_bin, v_central,
				    v_binError, v_centralStatError);
      g->SetName(anaName.c_str());
      g->SetTitle(anaName.c_str());
      plots.push_back(g);

      g = new TGraphErrors (axisBins.size(),
			    v_bin, v_central,
			    v_binError, v_centralTotError);
      g->SetName((anaName + "TotError").c_str());
      g->SetTitle((anaName + " Total Error").c_str());
      plotsSys.push_back(g);

      x_InitialCoordinate += x_DeltaCoordinate;
    }

    delete[] v_bin;
    delete[] v_binError;
    delete[] v_central;
    delete[] v_centralStatError;
    delete[] v_centralTotError;

    // Now build the canvas that we are going to store. Do do special axis labels we have
    // to fake the whole TGraph guy out.

    TCanvas *c = new TCanvas ("EffInfo");
    TH1F *h = new TH1F("SF", "",
		       binlabels.size(), 0.0, binlabels.size());

    h->SetMinimum (yCentralTotMin * 0.9);
    h->SetMaximum (yCentralTotMax * 1.1);
    h->SetStats(false);

    TAxis *a = h->GetXaxis();
    for (unsigned int i = 0; i < binlabels.size(); i++) {
      a->SetBinLabel(i+1, binlabels[i].c_str());
    }

    h->Draw();
    h->SetDirectory(0);

    int markerID[] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};
    int colorID[] =  { 1,  2,  3,  4,  5,  6,  7,  8,  9, 40, 41, 42, 43, 44, 45};
    for (unsigned int i_p = 0; i_p < plots.size(); i_p++) {
      int markerIndex = i_p % 15;
      plots[i_p]->SetMarkerStyle(markerID[markerIndex]);
      plots[i_p]->SetMarkerColor(colorID[markerIndex]);
      plots[i_p]->SetLineColor(colorID[markerIndex]);

      plots[i_p]->Draw("P");

      plotsSys[i_p]->SetLineColor(colorID[markerIndex]);
      plotsSys[i_p]->Draw("[]");
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
