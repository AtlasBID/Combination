///
/// Implementation of the context for a combination of sevearl measurements.
///

#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"

#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooGaussian.h>
#include <RooProdPdf.h>
#include <RooArgList.h>
#include <RooDataSet.h>
#include <RooProduct.h>
#include <RooAddition.h>
#include <RooPlot.h>

#include <TFile.h>
#include <TH1F.h>

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

namespace BTagCombination {

  ///
  /// Creates a new combination context.
  ///
  CombinationContext::CombinationContext(void) {
  }

  ///
  /// Clean up everything.
  ///
  CombinationContext::~CombinationContext(void) {
    for (unsigned int im = 0; im < _measurements.size(); im++) {
      delete _measurements[im];
    }
  }

  namespace {
    /// When we don't have a measurement name, generate it!
    static string NewMeasurementName(const string &name) {
      static map<string, int> gNameIndex;

      int index = 0;
      map<string, int>::const_iterator f = gNameIndex.find(name);
      if (f == gNameIndex.end()) {
	gNameIndex[name] = 1;
      } else {
	index = gNameIndex[name];
	gNameIndex[name] += 1;
      }

      ostringstream result;
      result << "m_" << name << "_" << index;
      return result.str();	
    }
  }

  ///
  /// Creates a new measurement.
  ///
  Measurement *CombinationContext::AddMeasurement(const string &measurementName, const string &what, const double minValue, const double maxValue,
						  const double value, const double statError) {
    ///
    /// Get the thing we are fitting to
    ///

    RooRealVar* whatVar = _whatMeasurements.FindOrCreateRooVar(what, minValue, maxValue);
    whatVar->setVal(value);

    Measurement *m = new Measurement(measurementName, what, value, statError);
    _measurements.push_back(m);
    return m;
  }

  Measurement *CombinationContext::AddMeasurement(const string &what, const double minValue, const double maxValue,
						  const double value, const double statError) {
    return AddMeasurement("blah", what, minValue, maxValue, value, statError);
  }

  ///
  /// Return the value that we got by doing the fit.
  ///
  RooRealVar CombinationContext::GetFitValue (const string &what) const
  {
    RooRealVar *var = _whatMeasurements.FindRooVar(what);
    if (var == nullptr)
      throw new runtime_error("There is no measurement named " + what);

    return *var;
  }

  RooCmdArg SigmaRange (const RooRealVar &v, const double sigma = 5.0)
  {
    double low = v.getVal() - sigma*v.getError();
    double high = v.getVal() + sigma*v.getError();
    return RooFit::Range(low, high);
  }

  ///
  /// Do the fit. We do all the building here.
  ///
  void CombinationContext::Fit(void)
  {
    ///
    /// Get all the systematic errors and create the variables we will need for them.
    ///

    for (vector<Measurement*>::const_iterator imeas = _measurements.begin(); imeas != _measurements.end(); imeas++) {
      Measurement *m(*imeas);
      vector<string> errorNames (m->GetSystematicErrorNames());
      for (vector<string>::const_iterator isyserr = errorNames.begin(); isyserr != errorNames.end(); isyserr++) {
	const string &sysErrorName(*isyserr);
	_systematicErrors.FindOrCreateRooVar(sysErrorName, -5.0, 5.0);
      }
    }

    ///
    /// Build the central value gaussian that we are going to fit.
    ///

    vector<RooAbsPdf*> measurementGaussians;
    for (vector<Measurement*>::const_iterator imeas = _measurements.begin(); imeas != _measurements.end(); imeas++) {
      Measurement *m(*imeas);

      ///
      /// The variable we are measureing is also balenced by the various
      /// systematic errors for this measurement. eff*(1+m1*s1)*(1.m2*s2)*(1+m3*s3)...
      ///

      RooRealVar *var = _whatMeasurements.FindRooVar(m->What());

      RooArgList varAddition;
      varAddition.add(*var);

      vector<string> errorNames (m->GetSystematicErrorNames());
      for (vector<string>::const_iterator isyserr = errorNames.begin(); isyserr != errorNames.end(); isyserr++) {
	const string &errName(*isyserr);
	RooAbsReal *weight = m->GetSystematicErrorWeight(*_systematicErrors.FindRooVar(errName));
	/// Mutiply by the eff that we are already looking at!
	varAddition.add(*weight);
      }
      
      string internalName = "InternalAddition" + m->Name();
      RooAddition *varSumed = new RooAddition (internalName.c_str(), internalName.c_str(), varAddition);

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

    for(vector<string>::const_iterator iVar = allVars.begin(); iVar != allVars.end(); iVar++) {
      string cName = *iVar + "ConstraintGaussian";
      RooRealVar *c = _systematicErrors.FindRooVar(*iVar);
      RooGaussian *constraint = new RooGaussian (cName.c_str(), cName.c_str(), *zero, *c, *one);
      products.add(*constraint);
    }

    RooProdPdf finalPDF("ConstraintPDF", "Constraint PDF", products);

    ///
    /// Next, we need to fit to a dataset. It will have a single data point - the
    /// central values of all the measurements.
    ///
    /// Q: Do we really need two different RooArgList's?
    ///

    RooArgList varNames, varValues;
    for (vector<Measurement*>::const_iterator imeas = _measurements.begin(); imeas != _measurements.end(); imeas++) {
      Measurement *m(*imeas);
      varNames.add(*(m->GetActualMeasurement()));
      varValues.add(*(m->GetActualMeasurement()));
    }

    RooDataSet measuredPoints ("pointsMeasured", "Measured Values", varNames);
    measuredPoints.add(varValues);

    ///
    /// And do the fit
    ///

    finalPDF.fitTo(measuredPoints);

    ///
    /// Now that the fit is done, dump out a root file that contains some good info
    ///

    {
      TFile output ("output.root", "RECREATE");

      ///
      /// First task - look at the actual values that are input and dump some informative plots on what it is.
      ///

      for (unsigned int i = 0; i < _measurements.size(); i++) {
	Measurement *item(_measurements[i]);
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

      ///
      /// First, the measurements, with and w/out errors
      ///

      vector<string> allMeasureNames = _whatMeasurements.GetAllVars();
      for(unsigned int i_mn = 0; i_mn < allMeasureNames.size(); i_mn++) {
	const string item (allMeasureNames[i_mn]);
	RooRealVar *m = _whatMeasurements.FindRooVar(item);
	RooCmdArg plotrange (SigmaRange(*m, 10.0));

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
	
	///
	/// Next, we need to re-run the fit, freezing each variable in turn, and extract the 
	/// fit values from there.
	///

	double centralValue = m->getVal();
	double centralError = m->getError();

	map<string, double> errorMap;

	/// Do each stasticial error by running the total.
	for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	  const string sysErrorName(allVars[i_av]);
	  RooRealVar *sysErr = _systematicErrors.FindRooVar(sysErrorName);
	  sysErr->setConstant(true);
	  sysErr->setVal(0.0);
	  sysErr->setError(0.0);

	  finalPDF.fitTo(measuredPoints);

	  double errDiff = sqrt(centralError*centralError - m->getError()*m->getError());
	  errorMap[sysErrorName] = errDiff;

	  sysErr->setConstant(false);
	}

	/// And the statistical error
	for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	  const string sysErrorName (allVars[i_av]);
	  RooRealVar *sysErr = _systematicErrors.FindRooVar(sysErrorName);
	  sysErr->setConstant(true);
	  sysErr->setVal(0.0);
	  sysErr->setError(0.0);
	}

	finalPDF.fitTo(measuredPoints);
	errorMap["statistical"] = m->getError();

	for (unsigned int i_av = 0; i_av < allVars.size(); i_av++) {
	  const string sysErrorName (allVars[i_av]);
	  RooRealVar *sysErr = _systematicErrors.FindRooVar(sysErrorName);
	  sysErr->setConstant(false);
	}

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
      }

      ///
      /// Since we've been futzing with all of this, we had better return the fit to be "normal".
      ///

      finalPDF.fitTo(measuredPoints);
    }

    ///
    /// Done. We need to clean up the measurement Gaussian
    ///

    for(vector<RooAbsPdf*>::iterator item = measurementGaussians.begin(); item!=measurementGaussians.end(); item++)
      delete *item;
  }
}
