///
/// Implementation of the context for a combination of sevearl measurements.
///

#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"
#include "Combination/MeasurementUtils.h"

#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooGaussian.h>
#include <RooProdPdf.h>
#include <RooArgList.h>
#include <RooDataSet.h>
#include <RooProduct.h>
#include <RooAddition.h>
#include <RooPlot.h>
#include <RooFitResult.h>

#include <TFile.h>
#include <TH1F.h>
#include <TMatrixT.h>

#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <sstream>

using std::string;
using std::map;
using std::runtime_error;
using std::for_each;
using std::transform;
using std::back_inserter;
using std::ostringstream;

namespace {
  using namespace BTagCombination;

  // Max length of a parameter we allow into RooFit to prevent a crash.
  // It does change with RooFit version number...
  const size_t cMaxParameterNameLength = 90;
  const int cMINUITStrat = 1;

  // Helper function that will look at the over correlation of two results and if it finds the over
  // correlation it will then turn it off.

  void CheckForAndDisableOverCorrelation (Measurement *m1, Measurement *m2, bool verbose = true)
  {
    pair<double, double> split1 = m1->SharedError(m2);
    pair<double, double> split2 = m2->SharedError(m1);

    // Calculate the rho

    double s12 = split1.first*split1.first + split1.second*split1.second;
    double s22 = split2.first*split2.first + split2.second*split2.second;
    double s1 = m1->totalError();
    double s2 = m2->totalError();

    double rho = m1->Rho(m2);

    // And now the weight, assuming a straight combination.
    
    double wt = (s22 - rho*s1*s2)/(s12 + s22 - 2*rho*s1*s2);
    if (wt > 1.0 || wt < 0.0) {
      if (verbose)
	cout << "WARNING: Correlated and uncorrelated errors make it impossible to combine these measurements." << endl
	     << "  " << m1->What() << endl
	     << "  #1: " << m1->Name() << endl
	     << "  s1=" << s1 << " s1c=" << split1.second << " s1u=" << split1.first << endl
	     << "  #2: " << m2->Name() << endl
	     << "  s2=" << s2 << " s2c=" << split2.second << " s2u=" << split2.first << endl
	     << "  rho=" << rho << " wt=" << wt << endl;
      if (s1 > s2) {
	if (verbose)
	  cout << "  Keeping #2" << endl;
	m1->setDoNotUse(true);
      } else {
	if (verbose)
	  cout << "  Keeping #1" << endl;
	m2->setDoNotUse(true);
      }
    }
  }

  //
  // Given a variable (which has been fit and so has an error), generate a range
  // that is +- 5 sigma around the variable.
  //
  RooCmdArg SigmaRange (const RooRealVar &v, const double sigma = 5.0)
  {
    double low = v.getVal() - sigma*v.getError();
    double high = v.getVal() + sigma*v.getError();
    return RooFit::Range(low, high);
  }

}

namespace BTagCombination {

  ///
  /// Creates a new combination context.
  ///
  CombinationContext::CombinationContext(void)
    : _verbose(true), _doPlots (false)
  {
  }

  //
  // If a gaussian error is less than 1% then we must bump it up, unfortunately.
  //
  void CombinationContext::AdjustTooSmallGaussians()
  {
    vector<Measurement*> gmes (GoodMeasurements());
    for (vector<Measurement*>::iterator itr = gmes.begin(); itr != gmes.end(); itr++) {
      if ((*itr)->doNotUse())
	continue;

      (*itr)->CheckAndAdjustStatisticalError(0.01);
    }
  }

  //
  // Look through all the measurements to be combined and make sure they
  // aren't going to put us in a region that is "bad".
  //
  void CombinationContext::TurnOffOverCorrelations()
  {
    //
    // First we need to catalog all the data ponits by what they are measuring, as
    // that is where we have to do the testing.
    //

    typedef map<string, vector<Measurement*> > t_MeasureByWhat;
    t_MeasureByWhat mapper;
    vector<Measurement*> gmes (GoodMeasurements());
    for (vector<Measurement*>::const_iterator itr = gmes.begin(); itr != gmes.end(); itr++) {
      if ((*itr)->doNotUse())
	continue;
      mapper[(*itr)->What()].push_back(*itr);
    }

    //
    // For each one calculate the conditions for over correlations, and turn off one if
    // it occurs.
    //

    for (t_MeasureByWhat::iterator itr = mapper.begin(); itr != mapper.end(); itr++) {

      // Silly cases.

      if (itr->second.size() < 2)
	continue; // Nothing to combine here! :-)

      // Now, for each combination of two we have to look to check for over correlation. If any of them
      // are, we drop the one with the lowest error.

      for (size_t i_1 = 0; i_1 < itr->second.size(); i_1++) {
	for (size_t i_2 = i_1 + 1; i_2 < itr->second.size(); i_2++) {
	  if (!itr->second[i_2]->doNotUse() && !itr->second[i_1]->doNotUse()) {
	    CheckForAndDisableOverCorrelation (itr->second[i_1], itr->second[i_2], _verbose);
	  }
	}
      }
    }
  }

  ///
  /// Do the fit. We do all the building here, and then the fit, and then we extract
  /// all the results needed.
  ///
  map<string, CombinationContext::FitResult> CombinationContext::Fit(const std::string &name)
  {
    //
    // We do keep some state (I know, not perfect)
    //

    _extraInfo.clear();

    //
    // First thing to do is x-check the measurements to eliminate any combinations
    // that will lead to bad points in phase space (i.e. the correlated/uncorrelated
    // are nasty. Also look for gaussian errors that are too small for our fitter
    // to deal with.
    //

    TurnOffOverCorrelations();
    AdjustTooSmallGaussians();

    //
    // There are only a certian sub-set of the measruements that are "valid"
    // for use.
    //

    vector<Measurement*> gMeas (GoodMeasurements());

    ///
    /// Get all the systematic errors and create the variables we will need for them.
    ///

    for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++) {
      Measurement *m(*imeas);

      vector<string> errorNames (m->GetSystematicErrorNames());
      for (vector<string>::const_iterator isyserr = errorNames.begin(); isyserr != errorNames.end(); isyserr++) {
	const string &sysErrorName(*isyserr);
	_systematicErrors.FindOrCreateRooVar(sysErrorName, -10.0, 10.0);
      }
    }

    ///
    /// Build the central value gaussian that we are going to fit.
    ///

    vector<RooAbsPdf*> measurementGaussians;
    vector<RooAddition*> toDeleteAddition;
    for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++) {
      Measurement *m(*imeas);

      ///
      /// The variable we are measureing is also balenced by the various
      /// systematic errors for this measurement. eff+m1*s1+m2*s2+m3*s3...
      ///

      RooRealVar *var = _whatMeasurements.FindRooVar(m->What());

      RooArgList varAddition;
      varAddition.add(*var);

      vector<string> errorNames (m->GetSystematicErrorNames());
      for (vector<string>::const_iterator isyserr = errorNames.begin(); isyserr != errorNames.end(); isyserr++) {
	const string &errName(*isyserr);
	RooAbsReal *weight = m->GetSystematicErrorWeight(*_systematicErrors.FindRooVar(errName));
	varAddition.add(*weight);
      }

      string internalName = "InternalAddition" + m->Name();
      RooAddition *varSumed = new RooAddition (internalName.c_str(), internalName.c_str(), varAddition);
      toDeleteAddition.push_back(varSumed);

      ///
      /// The actual variable and th esystematic error are also inputs into this
      /// guassian.
      ///

      RooRealVar *actualValue = (m->GetActualMeasurement());
      RooConstVar *statValue = (m->GetStatisticalError());

      ///
      /// Finally, built the gaussian. Make sure that its name is not
      /// the same as anything else... or *very* odd errors (with/out error messages)
      /// show up later!
      ///

      string gName = m->Name() + "Gaussian";
      RooGaussian *g = new RooGaussian(gName.c_str(), gName.c_str(),
				       *actualValue, *varSumed, *statValue);

      measurementGaussians.push_back(g);
    }

    ///
    /// Get a list of all the measurement gaussians and all the systematic error constraints, and create the product PDF
    /// for the minimization.
    ///

    RooArgList products;
    for (vector<RooAbsPdf*>::iterator itr = measurementGaussians.begin(); itr != measurementGaussians.end(); itr++) {
      products.add(**itr);
    }

    RooConstVar *zero = new RooConstVar("zero", "zero", 0.0);
    RooConstVar *one = new RooConstVar("one", "one", 1.0);
    vector<string> allVars = _systematicErrors.GetAllVars();

    vector<RooGaussian*> toDeleteGaussians;

    for(vector<string>::const_iterator iVar = allVars.begin(); iVar != allVars.end(); iVar++) {
      string cName = *iVar + "ConstraintGaussian";
      RooRealVar *c = _systematicErrors.FindRooVar(*iVar);
      RooGaussian *constraint = new RooGaussian (cName.c_str(), cName.c_str(), *c, *zero, *one);
      products.add(*constraint);
      toDeleteGaussians.push_back(constraint);
    }

    RooProdPdf finalPDF("ConstraintPDF", "Constraint PDF", products);

    //cout << "Printing final PDF" << endl;
    //finalPDF.Print();

    ///
    /// Next, we need to fit to a dataset. It will have a single data point - the
    /// central values of all the measurements.
    ///
    /// Q: Do we really need two different RooArgList's?
    ///

    RooArgList varNames, varValues;
    for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++) {
      Measurement *m(*imeas);

      varNames.add(*(m->GetActualMeasurement()));
      varValues.add(*(m->GetActualMeasurement()));
    }

    RooDataSet measuredPoints ("pointsMeasured", "Measured Values", varNames);
    measuredPoints.add(varValues);

    ///
    /// And do the fit
    ///

    if (_verbose)
      cout << "Starting the master fit..." << endl;
    RooFitResult *r = finalPDF.fitTo(measuredPoints, RooFit::Strategy(cMINUITStrat));
    delete r;

    ///
    /// Dump out the graph-viz tree
    ///
    
    finalPDF.graphVizTree("combined.dot");

    ///
    /// Extract the central values
    ///

    map<string, FitResult> result;
    map<string, double> totalError;
    map<string, double> runningErrorXCheck;
    for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++) {
      Measurement *m(*imeas);

      RooRealVar *v = _whatMeasurements.FindRooVar(m->What());
      result[m->What()].centralValue = v->getVal();
      totalError[m->What()] = v->getError();
      runningErrorXCheck[m->What()] = 0.0;
    }

    //
    // To actually calculate the chi2 we have a fair amount of work to do.
    // Using the method from the BLUE paper, eqn 14 (loosely based on this, actually).
    //  (published xxx)
    //

    {
      // Get the matrix of the measurements, the fits, and the covariance.
      TMatrixT<double> y (gMeas.size(), 1); // Actual measruements
      TMatrixT<double> Ux (gMeas.size(), 1); // The fit measurements for each guy

      int i_meas_row = 0;
      for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++, i_meas_row++) {
	Measurement *m(*imeas);
	y(i_meas_row, 0) = m->centralValue();
	Ux(i_meas_row, 0) = result[m->What()].centralValue;
      }

      //TMatrixTSym<double> W (CalcCovarMatrixUsingRho(gMeas));
      TMatrixTSym<double> W (CalcCovarMatrixUsingComposition(gMeas));

      // Now, calculate the chi2

      TMatrixT<double> Winv(W);
      // Invert inverts in place!!
      Winv.Invert();

      // Do the covar calc -- oh for the "auto" keyword.
      TMatrixT<double> del(gMeas.size(), 1);
      del = Ux-y;
      
      TMatrixT<double> delT(del);
      delT.Transpose(delT);

      TMatrixT<double> xchi2(1,1);
      xchi2 = (delT*Winv)*del;

      _extraInfo._globalChi2 = xchi2(0,0);
      _extraInfo._ndof = gMeas.size() -_whatMeasurements.size();

      if (_verbose)
	cout << "Total chi2 for " << name << ": " << xchi2(0,0) << " measurements: " << gMeas.size() << " fits: " << _whatMeasurements.size() << endl;
    }

    //
    // Dump out the pulls that the fit settled on... so this crudely fornow.
    //

    for(vector<string>::const_iterator iVar = allVars.begin(); iVar != allVars.end(); iVar++) {
      RooRealVar *c (_systematicErrors.FindRooVar(*iVar));
      if (_verbose)
      _extraInfo._nuisance[*iVar] = make_pair(c->getVal(), c->getError());
      _extraInfo._pulls[*iVar] = c->getVal()/c->getError();
    }

    ///
    /// Now that the fit is done, dump out a root file that contains some good info
    ///

    {
#ifdef notyet
      TFile output ("output.root", "RECREATE");

      ///
      /// First task - look at the actual values that are input and dump some informative plots on what it is.
      ///

      for (unsigned int i = 0; i < gMeas.size(); i++) {
	Measurement *item(gMeas[i]);

	TH1F* h = new TH1F((string(item->Name()) + "_input_errors_absolute").c_str(),
			  (string(item->Name()) + " Absolute Input Errors").c_str(),
			  allVars.size()+1, 0.0, allVars.size()+1);

	for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	  h->Fill(allVars[i_av].c_str(), 0.0);
	}
	
	h->Fill("statistical", item->GetStatisticalError()->getVal());
	
	vector<string> allErrs = item->GetSystematicErrorNames();
	for (unsigned int i_ae = 0; i_ae < allErrs.size(); i_ae++) {
	  h->Fill(allErrs[i_ae].c_str(), item->GetSystematicErrorWidth(allErrs[i_ae]));
	}
	
	h->LabelsOption("a");
	h->SetStats(false);
	h->Write();

	TH1F* hRel = static_cast<TH1F*>(h->Clone());
	hRel->SetName((string(item->Name()) + "_input_errors_percent").c_str());
	hRel->SetTitle((string(item->Name()) + " Input Errors (%)").c_str());
	hRel->Scale(1.0/item->GetActualMeasurement()->getVal()*100.0);
	hRel->Write();
      }
#endif

      ///
      /// First, the measurements, with and w/out errors
      ///

      vector<string> allMeasureNames = _whatMeasurements.GetAllVars();
      if (_doPlots) {
	for(unsigned int i_mn = 0; i_mn < allMeasureNames.size(); i_mn++) {
	  const string item (allMeasureNames[i_mn]);
	  cout << "Starting plots for " << item << endl;
	  RooRealVar *m = _whatMeasurements.FindRooVar(item);
	  RooCmdArg plotrange (SigmaRange(*m, 5.0));

	  RooPlot *nllplot = m->frame(plotrange);
	  nllplot->SetTitle((string("NLL of ") + m->GetTitle()).c_str());
	  nllplot->SetName((string(m->GetName()) + "_nll").c_str());
	  RooAbsReal *nll = finalPDF.createNLL(measuredPoints);
	  nll->plotOn(nllplot);
	  nllplot->Write();

	  ///
	  /// Make the profile plot for all the various errors allowed to float
	  ///

	  RooPlot *profilePlot = m->frame(plotrange);
	  profilePlot->SetTitle((string("Profile of ") + m->GetTitle()).c_str());
	  profilePlot->SetName((string(m->GetName()) + "_profile").c_str());
	  RooAbsReal *profile = nll->createProfile(*m);
	  profile->plotOn(profilePlot);

	  ///
	  /// Next, make the plot for just the statistical error
	  ///

	  RooArgSet allRooVars;
	  for (unsigned int i_av=0; i_av < allVars.size(); i_av++) {
	    const string sysErrName(allVars[i_av]);
	    RooRealVar *sysVar = _systematicErrors.FindRooVar(sysErrName);
	    allRooVars.add(*sysVar);
	  }
	  allRooVars.add(*m);

	  RooAbsReal *profileStat = nll->createProfile(allRooVars);
	  profileStat->plotOn(profilePlot, RooFit::LineColor(kRed));
	  profilePlot->Write();

	  ///
	  /// Next, do an extra plot alloweing everything but one systematic error to
	  /// float.
	  ///

	  for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	    const string excludeSysErrorName(allVars[i_av]);
	    RooArgSet allButRooVars;
	    allButRooVars.add(*m);

	    for (unsigned int i_av2 = 0; i_av2 < allVars.size(); i_av2++) {
	      const string sysError(allVars[i_av2]);
	      if (sysError != excludeSysErrorName) {
		allButRooVars.add(*(_systematicErrors.FindRooVar(sysError)));
	      }
	    }

	    RooPlot *allButProfilePlot = m->frame(plotrange);
	    allButProfilePlot->SetTitle((string("Profile of ") + m->GetTitle() + " (" + excludeSysErrorName + ")").c_str() );
	    allButProfilePlot->SetName((string(m->GetName()) + "_profile_" + excludeSysErrorName).c_str());
	    RooAbsReal *allButProfile = nll->createProfile(allButRooVars);
	    allButProfile->plotOn(allButProfilePlot, RooFit::LineColor(kRed));
	    profile->plotOn(allButProfilePlot);
	    allButProfilePlot->Write();				
	  }
	}
      }
	
      ///
      /// Next, we need to re-run the fit,
      /// freezing each systematic error in turn, and extract the 
      /// errors so we can decide how large each error is.
      ///

      for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	const string sysErrorName(allVars[i_av]);

	RooRealVar *sysErr = _systematicErrors.FindRooVar(sysErrorName);

	double sysErrOldVal = sysErr->getVal();
	double sysErrOldError = sysErr->getError();
	sysErr->setConstant(true);
	sysErr->setVal(0.0);
	sysErr->setError(0.0);

	//cout << "  Fitting to find systematic error contribute for " << sysErrorName << endl;
	RooFitResult *r = finalPDF.fitTo(measuredPoints, RooFit::Strategy(cMINUITStrat));
	delete r;

	// Loop over all measurements. If the measurement knows about
	// this systematic error, then extract a number from it.

	for (size_t i_mn = 0; i_mn < allMeasureNames.size(); i_mn++) {
	  const string &item(allMeasureNames[i_mn]);
	  RooRealVar *m = _whatMeasurements.FindRooVar(item);

	  if (sysErrorUsedBy(sysErrorName, item)) {

	    double centralError = totalError[item];
	    double delta = centralError*centralError - m->getError()*m->getError();
	    double errDiff = sqrt(fabs(delta));
	    //cout << "  centralError = " << centralError << endl
	    //<< "  fit result = " << m->getError() << endl
	    //<< "  delta = " << delta << endl;

	    // Propagate the sign
	    if (delta < 0.0)
	      errDiff = -errDiff;

	    // Save for later use!
	    result[item].sysErrors[sysErrorName] = errDiff;
	    runningErrorXCheck[item] += errDiff*errDiff;

	    // Save the change in the central value
	    result[item].cvShifts[sysErrorName] = result[item].centralValue - m->getVal();
	  }
	}

	// Restore the systematic errors to their former glory

	sysErr->setConstant(false);
	sysErr->setVal(sysErrOldVal);
	sysErr->setError(sysErrOldError);

      }

      //
      // And the statistical error. For this we do a seperate calculation exactly. This avoids
      // a common problem with the fit when the actual values are seperated by many orders of 10's
      // of sigma. The fit just dosen't work well.
      //

      map<string, double> stat_errors = CalculateStatisticalErrors();

      for(unsigned int i_mn = 0; i_mn < allMeasureNames.size(); i_mn++) {
	const string item (allMeasureNames[i_mn]);
	const double e (stat_errors[item]);
	result[item].statisticalError = e;
	runningErrorXCheck[item] += e*e;
      }


#ifdef notanymore
      for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	const string sysErrorName (allVars[i_av]);
	RooRealVar *sysErr = _systematicErrors.FindRooVar(sysErrorName);
	sysErr->setConstant(true);
	sysErr->setVal(0.0);
	sysErr->setError(0.0);
      }

      cout << "  Finding the statistical error" << endl;
      finalPDF.fitTo(measuredPoints, RooFit::Strategy(cMINUITStrat));

      for(unsigned int i_mn = 0; i_mn < allMeasureNames.size(); i_mn++) {
	const string item (allMeasureNames[i_mn]);
	RooRealVar *m = _whatMeasurements.FindRooVar(item);
	result[item].statisticalError = m->getError();
	runningErrorXCheck[item] += m->getError()*m->getError();
      }

      for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	const string sysErrorName (allVars[i_av]);
	RooRealVar *sysErr = _systematicErrors.FindRooVar(sysErrorName);
	sysErr->setConstant(false);
      }

#endif
#ifdef false
	/// Put them in a nice plot so we can read them out.
	TH1F *errorSummary = new TH1F((string(m->GetName()) + "_error_summary_absolute").c_str(),
				      (string(m->GetTitle()) + " Absolute Error Summary").c_str(),
				      errorMap.size(), 0, errorMap.size());

	int bin_index = 1;
	for(map<string,double>::const_iterator i_em = errorMap.begin(); i_em != errorMap.end(); i_em++) {
	  const pair<string, double> item(*i_em);
	  errorSummary->Fill(item.first.c_str(), item.second);
	  bin_index++;
	}

	errorSummary->LabelsOption("a");
	errorSummary->SetStats(false);
	errorSummary->Write();

	TH1F* hRel = static_cast<TH1F*>(errorSummary->Clone());
	hRel->SetName((string(m->GetName()) + "_error_summary_percent").c_str());
	hRel->SetTitle((string(m->GetTitle()) + " Percent Error Summary").c_str());
	hRel->Scale(1.0/m->getVal()*100.0);
	hRel->Write();
#endif
    }
  

    ///
    /// Since we've been futzing with all of this, we had better return the fit to be "normal".
    ///

    finalPDF.fitTo(measuredPoints, RooFit::Strategy(cMINUITStrat));

    //
    // How did the total errors work out?
    //

    for (map<string, double>::const_iterator itr = runningErrorXCheck.begin(); itr != runningErrorXCheck.end(); itr++) {
      double terr = sqrt(itr->second);
      double delta = fabs(terr - totalError[itr->first]);
      if (delta > 0.01) {
	cout << "WARNING Checking errors for measurement " << itr->first
	     << "   total error: " << totalError[itr->first]
	     << "   Summed Error: " << terr << endl
	     << "   Delta Error: " << delta << endl
	     << "   somethign went wrong in how we calc errors" << endl;
      }
    }

    ///
    /// Done. We need to clean up the measurement Gaussian
    ///

    for(vector<RooAbsPdf*>::iterator item = measurementGaussians.begin(); item!=measurementGaussians.end(); item++)
      delete *item;
    
    //
    // One last thing to take care of - if there were any correlations that were
    // put in we need to "take them out", as it were.
    //

    for (size_t i_c = 0; i_c < _correlations.size(); i_c++) {
      const CorrInfo &ci (_correlations[i_c]);
      if (ci._errorName == "statistical") {
	for (map<string, FitResult>::const_iterator i_fr = result.begin(); i_fr != result.end(); i_fr++) {
	  FitResult fr(i_fr->second);
	  string fr_name(i_fr->first);
	  map<string,double>::iterator s_value = fr.sysErrors.find(ci._sharedSysName);
	  if (s_value != fr.sysErrors.end()) {
	    //cout << "Dealing with stat error: " << endl
	    //<< " uncor = " << fr.statisticalError << endl
	    //<< " cor = " << s_value->second << endl;
	      
	    fr.statisticalError = sqrt(fr.statisticalError*fr.statisticalError
				       + s_value->second*s_value->second);
	    fr.sysErrors.erase(s_value);
	    result[fr_name] = fr;
	  }
	}
      }
    }

    //
    // Clean up some memory
    //

    for (size_t i = 0; i < toDeleteGaussians.size(); i++) {
      delete toDeleteGaussians[i];
    }
    for (size_t i = 0; i < toDeleteAddition.size(); i++) {
      delete toDeleteAddition[i];
    }

    //
    // Return all the final results.
    //

    return result;
  }
}
