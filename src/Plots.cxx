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
#include "THStack.h"
#include "TObject.h"
#include "TStyle.h"
#include "TLegend.h"

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
  
  const int colorID[] =  { 1,  2,  3,  4,  6,  7,  8,  9, 20, 30, 40, 21, 31, 41,
			   12, 22, 32, 42, 13, 23, 33, 43, 14, 24, 34, 44,
			   15, 25, 35, 45, 16, 26, 36, 46, 17, 27, 37, 47,
			   18, 28, 38, 48, 19, 29, 39, 49
  };
  int markerID[] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33};



  // Track the sys error to color mapping so we can make them all the same.
  int sysErrorColor (const string &sysErrorName)
  {
    static map<string, int> sysError_colorID;
    static int sysError_colorID_index = 0;

    if (sysError_colorID.find(sysErrorName) == sysError_colorID.end()) {
      sysError_colorID[sysErrorName] = colorID[sysError_colorID_index];
      sysError_colorID_index = (sysError_colorID_index+1) % (sizeof(colorID)/sizeof(int));
    }
    return sysError_colorID[sysErrorName];
  }

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
	//cout << "   found bin: " << ana.name << "(" << ana.flavor << "," << ana.tagger << "," << ana.operatingPoint << ") ";
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

  // Ordering for historams based on the size in their first bin.
  bool CompareHistoPairs (const pair<string, TH1F*> &left, const pair<string, TH1F*> &right)
  {
    for (int i = 1; i <= left.second->GetNbinsX(); i++) {
      if (fabs(left.second->GetBinContent(i)) != fabs(right.second->GetBinContent(i)))
	return fabs(left.second->GetBinContent(i)) < fabs(right.second->GetBinContent(i));
    }
    return false;
  }

  // Plot the plots.
  void stack_sys_error_plots (vector<pair<string,TH1F*> > &plots, const string &fitname,
			      const string &name_modifier = "",
			      const string &title_modifier = "",
			      const string &base_name = "ana_sys_",
			      const string &base_title = "Systematic Error (\\sigma^{2})",
			      const string &yaxis_title = "\\sigma^{2}")
  {
      ostringstream name, title;
      name << base_name << fitname << name_modifier;
      title << base_title << title_modifier << " for " << fitname << " SFs; ; " << yaxis_title;


    // Now that they are sorted, color them for "easy" viewing.

    int index = 0;
    for(vector<pair<string,TH1F*> >::const_iterator ip = plots.begin(); ip != plots.end(); ip++) {
      int col = sysErrorColor(ip->first);
      ip->second->SetLineColor(col);
      ip->second->SetFillColor(col);
      index++;
    }

    //
    // Build the stack. We have to build a postiive and negative stack. The ROOT code
    // can't deal, correctly, with histograms that have both postivie and negative components
    // in them.
    //

    THStack *hpos = new THStack (name.str().c_str(), title.str().c_str());
    THStack *hneg = new THStack (name.str().c_str(), title.str().c_str());

    double maxV = 0;
    double minV = 0;

    vector<TH1F*> temp;
    for(vector<pair<string,TH1F*> >::const_iterator ip = plots.begin(); ip != plots.end(); ip++) {
      double lmax = ip->second->GetMaximum();
      double lmin = ip->second->GetMinimum();
      if (lmax > 0)
	maxV += lmax;
      if (lmin < 0)
	minV += lmin;

      // Clone the histogram, and zero out everything that is postiive or negative
      TH1F* hp = static_cast<TH1F*>(ip->second->Clone());
      TH1F* hn = static_cast<TH1F*>(ip->second->Clone());
      temp.push_back(hp);
      temp.push_back(hn);

      for (int ib = 1; ib <= hp->GetNbinsX(); ib++) {
	double bc = hp->GetBinContent(ib);
	if (bc > 0) {
	  hn->SetBinContent(ib, 0.0);
	} else {
	  hp->SetBinContent(ib, 0.0);
	}
      }
      hpos->Add(hp);
      hneg->Add(hn);
    }

    hpos->SetMaximum(maxV+fabs(maxV)*0.1);
    hpos->SetMinimum(minV-fabs(minV)*0.1);
    hneg->SetMaximum(maxV+fabs(maxV)*0.1);
    hneg->SetMinimum(minV-fabs(minV)*0.1);

    //
    // The legends. So many sys errors, split them up in three.
    //

    vector<TLegend*> legends;
    float xmin = 0.19;
    float xmax = 0.39;
    float ymin = 0.45;
    float ymax = 0.92;

    int nLegends = (int) (plots.size()/15.0);
    if (nLegends > 3)
      nLegends = 3;
    if (nLegends == 0)
      nLegends = 1;

    for (int il = 0; il < nLegends; il++) {
      legends.push_back(new TLegend(xmin+0.2*il, ymin, xmax+0.2*il, ymax));
    }

    float divisor = plots.size() / (float)legends.size();

    int counter = 0;
    for(vector<pair<string,TH1F*> >::const_reverse_iterator ip = plots.rbegin(); ip != plots.rend(); ip++) {
      legends[(int)(counter/divisor)]->AddEntry(ip->second, ip->first.c_str(), "f");
      counter++;
    }

    TCanvas *c = new TCanvas(name.str().c_str(), title.str().c_str());
    hpos->Draw();
    hneg->Draw("SAME");
    for(vector<TLegend*>::const_iterator l = legends.begin(); l != legends.end(); l++)
      (*l)->Draw();

    //
    // Write everything out and clean it up.
    //

    c->Write();
    delete c;

    for (size_t i_p = 0; i_p < temp.size(); i_p++) {
      delete temp[i_p];
    }

    for (size_t i_l = 0; i_l < legends.size(); i_l++) {
      delete legends[i_l];
    }
  }

  //
  // Given a set of named plots organized by analysis, produce a stacked histogram of each one.
  //
  void plot_stacked_by_ana(map<string, vector<pair<string, TH1F*> > > &plotsByAna, const vector<string> &binlabels,
			   bool squareit, const string &base_name, const string &base_title, const string &yaxis_title)
  {
    for (map<string, vector<pair<string, TH1F*> > >::const_iterator itr = plotsByAna.begin(); itr != plotsByAna.end(); itr++) {

      // Square them and sort by size.
      vector<pair<string,TH1F*> > plots;
      map<int,double> total_error2;
      for(vector<pair<string,TH1F*> >::const_iterator ip = itr->second.begin(); ip != itr->second.end(); ip++) {
	TH1F *h2 = static_cast<TH1F*>(ip->second->Clone());
	for (int i = 1; i <= h2->GetNbinsX(); i++) {
	  double bc = h2->GetBinContent(i);
	  if (squareit) {
	    bc = bc*bc;
	    h2->SetBinContent(i, bc);
	  }
	  total_error2[i] += fabs(bc);
	}

	TAxis *a = h2->GetXaxis();
	for (size_t ib = 0; ib < binlabels.size(); ib++) {
	  a->SetBinLabel(ib+1, binlabels[ib].c_str());
	}

	plots.push_back(make_pair(ip->first, h2));
      }

      sort (plots.begin(), plots.end(), CompareHistoPairs);

      // Filter out a second list that contains only plots that make up
      // 5% or more of the total error squared.

      vector<pair<string,TH1F*> > plots_filtered_by_size;
      for(vector<pair<string,TH1F*> >::const_iterator ip = plots.begin(); ip != plots.end(); ip++) {
	bool above_threshold = false;
	for (int i = 1; i <= ip->second->GetNbinsX(); i++) {
	  if (fabs(ip->second->GetBinContent(i)) > 0.05*total_error2[i]) {
	    above_threshold = true;
	    plots_filtered_by_size.push_back(*ip);
	    break;
	  }
	}
      }      

      stack_sys_error_plots (plots, itr->first, "", "", base_name, base_title, yaxis_title);
      stack_sys_error_plots (plots_filtered_by_size, itr->first,
			     "_5p", "> 5% of Total ", base_name, base_title, yaxis_title);

      for(vector<pair<string,TH1F*> >::const_iterator ip = plots.begin(); ip != plots.end(); ip++) {
	delete ip->second;
      }
    }

  }

  ///
  /// Actually generate the plots for a particular bin
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas,
				    const t_BBSet &specifiedBins,
				    const t_BBSet &allAxisBins)
  {
    out -> cd();

    // Some setup and defs that make life simpler below.
    string binName (allAxisBins.begin()->variable);

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

    for (t_BBSet::const_iterator ib = allAxisBins.begin(); ib != allAxisBins.end(); ib++) {
      t_BBSet coordinate (specifiedBins);
      coordinate.insert(*ib);
      for(unsigned int ia = 0; ia < anas.size(); ia++) {
	bool found;
	CalibrationBin fb = FindBin (anas[ia], coordinate, found);
	if (found)
	  taggerResults[*ib][anas[ia].name] = fb;
      }
    }

    //
    // Some of the axis bins may not be filled by this combination of analyses. No need to plot those, so
    // we will remove them.
    //

    t_BBSet axisBins;
    for (t_BBSet::const_iterator ib = allAxisBins.begin(); ib != allAxisBins.end(); ib++) {
      if (taggerResults.find(*ib) != taggerResults.end()) {
	axisBins.insert(*ib);
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
	map<string, vector<double> >::const_iterator i_chi2 = anas[ia].metadata.find("gchi2");
	map<string, vector<double> >::const_iterator i_ndof = anas[ia].metadata.find("gndof");
	if (i_chi2 != anas[ia].metadata.end()) {
	  anaChi2.push_back((i_chi2->second[0])/(i_ndof->second[0]));
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
    map<string, vector<pair<string, TH1F*> > > sysErrorPlotsByAna;
    map<string, vector<pair<string, TH1F*> > > cvShiftPlotsByAna;
    map<string, TH1F*> cvShiftPlotsSingle;
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
	v_central[ib] = 0.0;
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
	if (i_c->second.find(anaName) != i_c->second.end()) {
	  const CalibrationBin &cb(i_c->second.find(anaName)->second);
	  for (unsigned int i = 0; i < cb.systematicErrors.size(); i++) {
	    allSys.insert(cb.systematicErrors[i].name);
	  }
	}
      }

      for (set<string>::const_iterator i = allSys.begin(); i != allSys.end(); i++) {
	TH1F *h = DeclareSingleHist(anaName, string("_sys_") + *i, string ("Systematic errors for ") + *i + " ", axisBins.size(), out);
	singlePlots[*i] = h;
	sysErrorPlots[*i].push_back(make_pair(anaName,h));
	sysErrorPlotsByAna[anaName].push_back(make_pair(*i,h));

	h = DeclareSingleHist(anaName, string("_cvShift_") + *i, string ("Central Value shifts caused by ") + *i + " ", axisBins.size(), 0);
	cvShiftPlotsSingle[*i] = h;
	cvShiftPlotsByAna[anaName].push_back(make_pair(*i,h));
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

	if (i_c->second.find(anaName) != i_c->second.end()) {
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

	  for (map<string, pair<double, double> >::const_iterator i_meta = cb.metadata.begin(); i_meta != cb.metadata.end(); i_meta++) {
	    if (i_meta->first.find("CV Shift ") == 0) {
	      string name(i_meta->first.substr(9));
	      if (cvShiftPlotsSingle.find(name) == cvShiftPlotsSingle.end())
		throw runtime_error (("Unexpected systematic error " + name).c_str());
	      cvShiftPlotsSingle[name]->SetBinContent(ibin+1, i_meta->second.first);
	    }
	  }
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

    //
    // Build a set of stacked histograms of the systeamtic errors^2 contribution in each plot.
    // Also, window out only those that contribute at least 5%.
    // This would be a lot simpler with C++011!!!
    //

    plot_stacked_by_ana(sysErrorPlotsByAna, binlabels, true, "ana_sys_", "Systematic Error (\\sigma^{2})",  "\\sigma^{2}");
    plot_stacked_by_ana(cvShiftPlotsByAna, binlabels, false, "cv_shift_", "Central Value Shift", "CV Shift");

    //
    // Get rid fo the cv shift guy
    //

    for (map<string, TH1F*>::const_iterator i_p = cvShiftPlotsSingle.begin(); i_p != cvShiftPlotsSingle.end(); i_p++) {
      delete i_p->second;
    }
    cvShiftPlotsSingle.clear();

    //
    // Plot the systematic errors for each bin for all contributing analyses on one plot. This way
    // one can easily compare the contributions of one systeamtic error to all different analyses, and
    // see how fitting controls them.
    //

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
      h->SetMaximum(maxV+fabs(maxV)*0.10);
      h->SetMinimum(minV-fabs(minV)*0.10);
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
	
	h_itr->second->SetMaximum (maxV+fabs(maxV)*0.10);
	h_itr->second->SetMinimum (minV-fabs(minV)*0.10);

	h_itr->second->SetMarkerStyle(markerID[m_index]);
	int col = sysErrorColor(h_itr->first);
	h_itr->second->SetMarkerColor(col);
	h_itr->second->SetLineColor(col);

	h_itr->second->Draw("SAMEP");

	myMarkerText (lXPos, lYPos,
		      col, markerID[m_index],
		      h_itr->first.c_str());

	m_index += 1;
	m_index = m_index % sizeof(markerID);
	lYPos -= lYDelta;
      }      

      c->Write();
      delete c;
      delete h;

      for (vector<pair<string, TH1F*> >::const_iterator h_itr = hlist.begin(); h_itr != hlist.end(); h_itr++) {
	delete h_itr->second;
      }
      

    }

    // Build a canvas that will contain the final plot of the fit results and the inputs and systematic
    // and statistical errors. We will store this in the output directory.
    // Now build the canvas that we are going to store. Do do special axis labels we have
    // to fake the whole TGraph guy out.

    TCanvas *c = new TCanvas ("EffInfo");
    TH1F *h = new TH1F("SF", "", binlabels.size(), 0.0, binlabels.size());

    h->SetMinimum (yCentralTotMin-fabs(yCentralTotMin)*0.1);
    h->SetMaximum (yCentralTotMax+fabs(yCentralTotMax)*0.1);
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
    myText(binXPos, binYPos, 1, anas[0].jetAlgorithm.c_str());

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
    delete h;
    for (size_t i_p = 0; i_p < plotsSys.size(); i_p++) {
      delete plotsSys[i_p];
    }
    for (size_t i_p = 0; i_p < plots.size(); i_p++) {
      delete plots[i_p];
    }
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

  pair<int, vector<double> > inc_average_counter (pair<int, vector<double> > v, vector<double> inc)
  {
    if (v.second.size() != inc.size()) {
      cout << "Internal error. Attempt to average vectors with size " << v.second.size() << " with " << inc.size() << endl;
      throw runtime_error ("Averaging can't happen on things of different size, inconsitent pull or similar!");
    }

    vector<double> initial (v.second);
    for (size_t i = 0; i < inc.size(); i++)
      initial[i] += inc[i];
    return make_pair (v.first + 1, initial);
  }

  ///
  /// Put a pull plot into the output
  ///
  void GenerateMetaDataListPlot (TDirectory *out, const map<string, vector<double> > &metadata,
				 const string &meta_name_prefix,
				 const string &th1_name,
				 const string &th1_title,
				 bool plotSigmaBands = false)
  {
    map<string, pair<int, vector<double> > > pulls;
    for(map<string, vector<double> >::const_iterator i = metadata.begin(); i != metadata.end(); i++) {
      string name (i->first);
      if (name.find(meta_name_prefix) == 0) {
	name = name.substr(meta_name_prefix.size());
	if (name.find("Correlated") != 0) {

	  // These are split, so we need to deal with getting the proper name.
	  if (name.find("UNCORBIN-") == 0) {
	    name = name.substr(9);
	    size_t e = name.find("-**");
	    name = name.substr(0, e);
	  }

	  if (pulls.find(name) == pulls.end()) {
	    pulls[name] = make_pair(0, vector<double>(i->second.size(), 0.0));
	  }
	  pulls[name] = inc_average_counter (pulls[name], i->second);
	}
      }
    }

    // Now, build the histo

    if (pulls.size() > 0) {

      TH1F *h = new TH1F (th1_name.c_str(), th1_title.c_str(),
			  pulls.size(), 0.0, pulls.size());
      h->SetDirectory(0);
    
      TAxis *a = h->GetXaxis();
      int ibin = 1;
      for (map<string, pair<int, vector<double> > >::const_iterator i = pulls.begin(); i != pulls.end(); i++, ibin++) {
	a->SetBinLabel(ibin, i->first.c_str());
	double value = i->second.second[0] / ((double)i->second.first);
	h->SetBinContent(ibin, value);
	if (i->second.second.size() > 1) {
	  double error = i->second.second[1] / ((double)i->second.first);
	  h->SetBinError(ibin, error);
	}
      }
      a->LabelsOption("v");

      h->SetFillColor(4);
      h->SetStats(0);
      h->SetMinimum(-2.5);
      h->SetMaximum(2.5);

      TCanvas *c = new TCanvas(th1_name.c_str(), th1_title.c_str());
      c->SetBottomMargin(0.55);

      h->Draw(); // We really want hbar here, but it doesn't quite work when filling in a histo

      if (plotSigmaBands) {
	// Draw the yellow bands now
	TBox *b1 = new TBox (0.0, -2.0, h->GetNbinsX(), 2.0);
	b1->SetFillColor(kYellow);
	b1->Draw();

	TBox *b2 = new TBox (0.0, -1.0, h->GetNbinsX(), 1.0);
	b2->SetFillColor(kGreen);
	b2->Draw();


	h->Draw("AXISSAME");
	h->Draw("SAME");

      } else {
	c->SetGrid();
      }

      out->cd();
      c->Write();
      delete c;
      delete h;
    }
  }


  //
  // Extract all the from claues from the list of 
  set<string> ExtractAllFromClauses(const map<string,vector<double> > &meta)
  {
    set<string> result;
    for (map<string,vector<double> >::const_iterator itr = meta.begin(); itr != meta.end(); itr++) {
      size_t itx = itr->first.find("[from");
      if (itx != string::npos) {
	string fromClause = itr->first.substr(itx+1);
	fromClause = fromClause.substr(5, fromClause.size()-1-5);
	result.insert(fromClause);
      }
    }

    return result;
  }

  ///
  /// Generate the plots that are global to a single analysis
  ///
  void GenerateAnalysisPlots (TDirectory *out, const CalibrationAnalysis &ana)
  {
    GenerateMetaDataListPlot(out, ana.metadata, "Pull ", "pull_" + ana.name, "Pulls for fit " + ana.name);
    GenerateMetaDataListPlot(out, ana.metadata, "Nuisance ", "nuisance_" + ana.name, "Nuisance values for fit " + ana.name, true);

    // Pull out the systematic errors so we can see what happens bin-by-bin for pulls and nuisance parameters

    set<string> fromClauses (ExtractAllFromClauses(ana.metadata));
    for (set<string>::const_iterator fc = fromClauses.begin(); fc != fromClauses.end(); fc++) {
      string fcSearch (string("[from ") + *fc + "]");
      map<string, vector<double> > skimmed_metadata;
      for(map<string,vector<double> >::const_iterator md = ana.metadata.begin(); md != ana.metadata.end(); md++) {
	size_t index = md->first.find(fcSearch);
	if (index != string::npos) {
	  string name(md->first.substr(0, index-1));
	  skimmed_metadata[name] = md->second;
	}
      }

      GenerateMetaDataListPlot(out, skimmed_metadata, "Pull ", "pull_" + ana.name + *fc, "Pulls for fit " + ana.name + " [" + *fc + "]");
      GenerateMetaDataListPlot(out, skimmed_metadata, "Nuisance ", "nuisance_" + ana.name + *fc, "Nuisance values for fit " + ana.name + " [" + *fc + "]", true);
    }

  }

  ///
  /// Split plots up by binning
  ///
  void GenerateCommonAnalysisPlots (TDirectory *out,
				    const vector<CalibrationAnalysis> &anas)
  {
    //
    // Do the plots that are for a single analysis
    //

    for (size_t i = 0; i < anas.size(); i++) {
      GenerateAnalysisPlots (out, anas[i]);
    }

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
