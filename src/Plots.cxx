//
// Code to dump plots of the input and output analyses to a
// root file.
//

#include "Combination/Plots.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/AtlasLabels.h"

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
#include <stdexcept>
#include <iomanip>

using namespace std;

namespace {
  using namespace BTagCombination;

  typedef map<string, vector<CalibrationAnalysis> > t_grouping;
  typedef set<CalibrationBinBoundary> t_BBSet;
  typedef map<string, t_BBSet> t_BinSet;

  // A few global constants...
  const double c_legendXStart = 0.55;
  const double c_legendYStart = 0.90;
  const double c_legendYDelta = 0.037;

  const double c_binXStart = 0.20;
  const double c_binYStart = 0.85;
  const double c_binYDelta = 0.05;
  

  ///
  /// Many of these routines could be moved into the global namespace if their
  /// utility was needed.
  ///

  // Find the bin results for a partiuclar bin. Return an empty if we
  // can't find it.
  CalibrationBin FindBin (const CalibrationAnalysis &ana, t_BBSet &bininfo, bool &found)
  {
    found = false;
    for (unsigned int ib = 0; ib < ana.bins.size(); ib++) {
      const CalibrationBin &cb (ana.bins[ib]);
      t_BBSet t (cb.binSpec.begin(), cb.binSpec.end());
      if (t == bininfo) {
	//cout << "found bin: " << ana.name << "(" << ana.flavor << "," << ana.tagger << "," << ana.operatingPoint << ") ";
	//for (t_BBSet::const_iterator i = t.begin(); i != t.end(); i++) {
	//cout << *i << " ";
	//}
	//cout << endl;
	found = true;
	return cb;
      }
    }

    // This line will cause an analysis that isn't present to appear as a "ZERO"
    // in the plot. We should fix this.
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
  /// Declare a single histogram
  ///

  TH1F *DeclareSingleHist(const string &ananame, const string &namePostFix, const string &titlePrefix, int nbins,
			  TDirectory *out)
  {
    TH1F *t = new TH1F((ananame + namePostFix).c_str(), (titlePrefix + ananame).c_str(), nbins, 0.0, nbins);
    t->SetDirectory(out);
    return t;
  }

  ///
  /// Actually generate the plots for a particular bin
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas,
				    const t_BBSet &specifiedBins,
				    const t_BBSet &axisBins)
  {
    out -> cd();

    int markerID[] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};
    int colorID[] =  { 1,  2,  3,  4,  6,  7,  8,  9, 40, 41, 42, 43, 44, 45};

    // Some setup and defs that make life simpler below.
    string binName (axisBins.begin()->variable);

    double legendYPos = c_legendYStart;
    double legendYDelta = c_legendYDelta;
    double legendXPos = c_legendXStart;

    string flavorName (anas[0].flavor);
    string taggerName (anas[0].tagger);
    string opName (anas[0].operatingPoint);

    // Extract all the results.

    typedef map<string, CalibrationBin> t_CBMap;
    typedef map<CalibrationBinBoundary, t_CBMap> t_BoundaryMap;
    t_BoundaryMap taggerResults;

    for (t_BBSet::const_iterator ib = axisBins.begin(); ib != axisBins.end(); ib++) {
      t_BBSet coordinate (specifiedBins);
      coordinate.insert(*ib);
      for(unsigned int ia = 0; ia < anas.size(); ia++) {
	bool found;
	CalibrationBin fb = FindBin (anas[ia], coordinate, found);
	if (!found) {
	  if (taggerResults[*ib].find(anas[ia].name) == taggerResults[*ib].end()) {
	    taggerResults[*ib][anas[ia].name] = fb;
	  }
	} else {
	  taggerResults[*ib][anas[ia].name] = fb;
	}
      }
    }

    //
    // A little tricky here. The person providing input can split the analysis up into multiple
    // partitiions. So the same analysis can appear twice in the list. No good talking about it that
    // way on the plots. So, sort that out. and maintain order.
    //

    set<string> seenAnaNames;
    vector<string> anaNames;
    vector<double> anaChi2;
    for (unsigned int ia = 0; ia < anas.size(); ia++) {
      string anaName (anas[ia].name);
      if (seenAnaNames.find(anaName) == seenAnaNames.end()) {
	seenAnaNames.insert(anaName);
	anaNames.push_back(anaName);
	map<string, double>::const_iterator i_chi2 = anas[ia].metadata.find("gchi2");
	map<string, double>::const_iterator i_ndof = anas[ia].metadata.find("gndof");
	if (i_chi2 != anas[ia].metadata.end()) {
	  anaChi2.push_back((i_chi2->second)/(i_ndof->second));
	} else {
	  anaChi2.push_back(0.0);
	}
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
    vector<string> plotAnalysisName;
    vector<string> binlabels;
    map<string, vector<pair<string, TH1F*> > > sysErrorPlots;
    Double_t *v_bin = new Double_t[axisBins.size()];
    Double_t *v_binError = new Double_t[axisBins.size()];
    Double_t *v_central = new Double_t[axisBins.size()];
    Double_t *v_centralStatError = new Double_t[axisBins.size()];
    Double_t *v_centralTotError = new Double_t[axisBins.size()];

    double yCentralTotMin = 10.0;
    double yCentralTotMax = 0.0;

    //
    // Our first job is to build the bin lables. We already have the set
    // that is the bin lables. So all we now have to do is get a mapping
    // between the labels.
    //

    map<CalibrationBinBoundary, int> bbBinNumber;
    int index = 0;
    for (t_BBSet::const_iterator itr_b = axisBins.begin(); itr_b != axisBins.end(); itr_b++) {
      ostringstream buf;
      buf << CalibrationBinBoundaryFormat(CalibrationBinBoundary::kROOTFormatted)
	  << *itr_b;
      binlabels.push_back(buf.str());

      bbBinNumber[*itr_b] = index;
      index++;
    }

    //
    // Extract the data for plotting, and make the plots. We actually make the plots
    // for the single values here.
    //

    for (unsigned int ia = 0; ia < anaNames.size(); ia++) {
      const string &anaName (anaNames[ia]);

      // Clear out the arrays...
      for (size_t ib = 0; ib < axisBins.size(); ib++) {
	v_bin[ib] = x_InitialCoordinate + ib;
	v_binError[ib] = 0; // No error along x-axis!!

	v_binError[ib] = 0.0;
	v_central[ib] = -1000.0;
	v_centralStatError[ib] = 0.0;
	v_centralTotError[ib] = 0.0;
      }

      // Create the plots we are going to be filling as we go
      map<string, TH1F*> singlePlots;
      singlePlots["central"] = DeclareSingleHist(anaName,  "_cv", "Central values for ", axisBins.size(), out);
      singlePlots["statistical"] = DeclareSingleHist(anaName,  "_stat", "Statistical errors for ", axisBins.size(), out);
      singlePlots["total"] = DeclareSingleHist(anaName,  "_totalerror", "Total errors for ", axisBins.size(), out);

      set<string> allSys;
      for (t_BoundaryMap::const_iterator i_c = taggerResults.begin(); i_c != taggerResults.end(); i_c++) {
	const CalibrationBin &cb(i_c->second.find(anaName)->second);
	for (unsigned int i = 0; i < cb.systematicErrors.size(); i++) {
	  allSys.insert(cb.systematicErrors[i].name);
	}
      }

      for (set<string>::const_iterator i = allSys.begin(); i != allSys.end(); i++) {
	TH1F *h = DeclareSingleHist(anaName, string("_sys_") + *i, string ("Systematic errors for ") + *i + " ", axisBins.size(), out);;
	singlePlots[*i] = h;
	sysErrorPlots[*i].push_back(make_pair(anaName,h));
      }

      // Now, loop over all the bins filling everything in
      for (t_BoundaryMap::const_iterator i_c = taggerResults.begin(); i_c != taggerResults.end(); i_c++) {
	// Get the bin number
	if (bbBinNumber.find(i_c->first) == bbBinNumber.end()) {
	  ostringstream err;
	  err << "Internal error - bin boundary " << i_c->first << " is not known as a bin boundary!";
	  throw runtime_error(err.str().c_str());
	}
	int ibin = bbBinNumber[i_c->first];

	const CalibrationBin &cb(i_c->second.find(anaName)->second);
	v_central[ibin] = cb.centralValue;
	v_centralStatError[ibin] = cb.centralValueStatisticalError;
	v_centralTotError[ibin] = CalcTotalError(cb);

	// Fill in the single plots now
	singlePlots["central"]->SetBinContent(ibin+1, cb.centralValue);
	singlePlots["statistical"]->SetBinContent(ibin+1, cb.centralValueStatisticalError);
	singlePlots["total"]->SetBinContent(ibin+1, v_centralTotError[ibin]);
	for (unsigned int i = 0; i < cb.systematicErrors.size(); i++) {
	  singlePlots[cb.systematicErrors[i].name]->SetBinContent(ibin+1, cb.systematicErrors[i].value);
	}

	// Record min and max for later use with limit setting
	double t = v_central[ibin] - v_centralTotError[ibin];
	if (t != 0.0 && yCentralTotMin > t)
	  yCentralTotMin = t;
	t = v_central[ibin] + v_centralTotError[ibin];
	if (t != 0.0 && yCentralTotMax < t)
	  yCentralTotMax = t;

	ibin++;
      }

      //
      // Build the main comparison plot
      //

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

      // The name should have a chi2 if we can

      if (anaChi2[ia] != 0.0) {
	ostringstream msg;
	msg << anaName << " (\\chi^{2}/N_{DOF}=" <<setprecision(3) << anaChi2[ia] << ")";
	plotAnalysisName.push_back(msg.str());
      } else {
	plotAnalysisName.push_back(anaName);
      }

      // And the place we will be putting stuff

      x_InitialCoordinate += x_DeltaCoordinate;
    }

    delete[] v_bin;
    delete[] v_binError;
    delete[] v_central;
    delete[] v_centralStatError;
    delete[] v_centralTotError;

    // Build a canvas that will store each systematic error, all plotted on top of each other.

    for (map<string, vector<pair<string, TH1F*> > >::const_iterator itr = sysErrorPlots.begin(); itr != sysErrorPlots.end(); itr++) {
      string name = "sys_" + itr->first;
      TCanvas *c = new TCanvas (name.c_str());

      const vector<pair<string, TH1F*> > &hlist (itr->second);
      double maxV = -1000.0;
      double minV = 1000.0;
      for (vector<pair<string, TH1F*> >::const_iterator h_itr = hlist.begin(); h_itr != hlist.end(); h_itr++) {
	if (maxV < h_itr->second->GetMaximum())
	  maxV = h_itr->second->GetMaximum();
	if (minV > h_itr->second->GetMinimum())
	  minV = h_itr->second->GetMinimum();
      }

      if (minV > 0.0)
	minV = 0.0;

      // Plot header

      TH1F *h = new TH1F("sys", "",
			 binlabels.size(), 0.0, binlabels.size());
      h->SetMaximum(maxV*1.10);
      h->SetMinimum(minV);
      h->SetStats(false);
      {
	ostringstream buf;
	buf << "Systematic Error " << itr->first;
	h->GetYaxis()->SetTitle(buf.str().c_str());

	TAxis *a = h->GetXaxis();
	for (size_t i = 0; i < binlabels.size(); i++) {
	  a->SetBinLabel(i+1, binlabels[i].c_str());
	}
      }

      h->Draw();
      h->SetDirectory(0);

      // Plot the actual guys

      size_t m_index = 0;
      double lYPos = c_legendYStart;
      double lYDelta = c_legendYDelta;
      double lXPos = c_legendXStart;

      for (vector<pair<string, TH1F*> >::const_iterator h_itr = hlist.begin(); h_itr != hlist.end(); h_itr++) {
	
	h_itr->second->SetMaximum (maxV*1.10);
	h_itr->second->SetMinimum (minV);

	h_itr->second->SetMarkerStyle(markerID[m_index]);
	h_itr->second->SetMarkerColor(colorID[m_index]);
	h_itr->second->SetLineColor(colorID[m_index]);

	h_itr->second->Draw("SAMEP");

	myMarkerText (lXPos, lYPos,
		      colorID[m_index], markerID[m_index],
		      h_itr->first.c_str());

	m_index += 1;
	m_index = m_index % sizeof(markerID);
	lYPos -= lYDelta;
      }      

      c->Write();
      delete c;
    }

    // Build a canvas that will contain the final plot of the fit results and the inputs and systematic
    // and statistical errors. We will store this in the output directory.
    // Now build the canvas that we are going to store. Do do special axis labels we have
    // to fake the whole TGraph guy out.

    TCanvas *c = new TCanvas ("EffInfo");
    TH1F *h = new TH1F("SF", "",
		       binlabels.size(), 0.0, binlabels.size());

    h->SetMinimum (yCentralTotMin * 0.9);
    h->SetMaximum (yCentralTotMax * 1.1);
    h->SetStats(false);

    {
      ostringstream buf;
      buf << "SF for " << flavorName << " jets (" << taggerName << " " << opName << ")";
      h->GetYaxis()->SetTitle(buf.str().c_str());
    }

    TAxis *a = h->GetXaxis();
    for (unsigned int i = 0; i < binlabels.size(); i++) {
      a->SetBinLabel(i+1, binlabels[i].c_str());
    }

    h->Draw();
    h->SetDirectory(0);

    for (unsigned int i_p = 0; i_p < plots.size(); i_p++) {
      int markerIndex = i_p % sizeof(markerID);
      plots[i_p]->SetMarkerStyle(markerID[markerIndex]);
      plots[i_p]->SetMarkerColor(colorID[markerIndex]);
      plots[i_p]->SetLineColor(colorID[markerIndex]);

      plots[i_p]->Draw("P");

      plotsSys[i_p]->SetLineColor(colorID[markerIndex]);
      plotsSys[i_p]->Draw("[]");

      myMarkerText (legendXPos, legendYPos,
		    colorID[markerIndex], markerID[markerIndex],
		    plotAnalysisName[i_p].c_str());
      legendYPos -= legendYDelta;
    }

    // Last thing to do is add to the legend all the bin coordinates that we have
    // fixed for this plot.

    double binYPos = c_binYStart;
    double binXPos = c_binXStart;
    for (t_BBSet::const_iterator i = specifiedBins.begin(); i != specifiedBins.end(); i++) {
      ostringstream buf;
      buf << CalibrationBinBoundaryFormat(CalibrationBinBoundary::kROOTFormatted)
	  << *i;
      myText(binXPos, binYPos, 1, buf.str().c_str());
      binYPos -= c_binYDelta;
    }

    //
    // Write it out in that directory that we are using so it can be found
    // by the user. We have to actually write it b/c root really doesn't like
    // more than one canvas with the same name in memory at a time. Writing it
    // out makes sure that it stays around, and actually deleting it means we
    // don't get a pesky warning from ROOT about a name collision. :-)
    // Do you love root? I do!
    //

    c->Write();
    delete c;
  }

  ///
  /// Recursive routine to look at all the variables and walk down until there
  /// is just one left for the axis.
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas,
				    const string &binName,
				    const t_BinSet &binSet,
				    const t_BBSet &otherBins = t_BBSet())
  {
    //
    // Are we at the last variable - which will be the axis of a plot?
    //

    if (otherBins.size() + 1 == binSet.size()) {
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
