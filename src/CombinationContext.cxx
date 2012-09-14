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
#include <RooXYChi2Var.h>
#include <RooFitResult.h>

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

namespace {
  using namespace BTagCombination;

  // Max length of a parameter we allow into RooFit to prevent a crash.
  // It does change with RooFit version number...
  const size_t cMaxParameterNameLength = 90;
  const int cMINUITStrat = 1;

  // Helper function that will look at the over correlation of two results and if it finds the over
  // correlation it will then turn it off.

  void CheckForAndDisableOverCorrelation (Measurement *m1, Measurement *m2)
  {
    pair<double, double> split1 = m1->SharedError(m2);
    pair<double, double> split2 = m2->SharedError(m1);

    // Calculate the rho

    double s12 = split1.first*split1.first + split1.second*split1.second;
    double s22 = split2.first*split2.first + split2.second*split2.second;
    double s1 = sqrt(s12);
    double s2 = sqrt(s22);

    double rho = split1.second*split2.second/(s1*s2);

    // And now the weight, assuming a straight combination.
    
    double wt = (s22 - rho*s1*s2)/(s12 + s22 - 2*rho*s1*s2);
    if (wt > 1.0 || wt < 0.0) {
      cout << "WARNING: Correlated and uncorrelated errors make it impossible to combine these measurements." << endl
	   << "  " << m1->What() << endl
	   << "  #1: " << m1->Name() << endl
	   << "  s1=" << s1 << " s1c=" << split1.second << " s1u=" << split1.first << endl
	   << "  #2: " << m2->Name() << endl
	   << "  s2=" << s2 << " s2c=" << split2.second << " s2u=" << split2.first << endl
	   << "  rho=" << rho << " wt=" << wt << endl;
      if (s1 > s2) {
	cout << "  Keeping #2" << endl;
	m1->setDoNotUse(true);
      } else {
	cout << "  Keeping #1" << endl;
	m2->setDoNotUse(true);
      }
    }
  }
}

namespace BTagCombination {

  ///
  /// Creates a new combination context.
  ///
  CombinationContext::CombinationContext(void)
    : _doPlots (false)
  {
  }

  ///
  /// Clean up everything.
  ///
  CombinationContext::~CombinationContext(void) {
    for (unsigned int im = 0; im < _measurements.size(); im++) {
      delete _measurements[im];
    }
  }

  //
  // establish a correlation between two measurements for one of the
  // errors.
  //
  void CombinationContext::AddCorrelation (const std::string &errorName,
					   Measurement *m1,
					   Measurement *m2,
					   double correlation)
  {
    if (errorName != "statistical") {
      throw runtime_error ("Can only deal with correlations for statsical errors!");
    }

    if (correlation == 1.0) {
      cout << "WARNING: Can't deal with a correlation that is 1.0 - setting it to 0.99..." << endl
	   << "  RooFit's fail to converge correctly in this case." << endl
	   << "  The common variable is " << m1->What() << endl
	   << "  m1 = " << m1->Name() << endl
	   << "  m2 = " << m2->Name() << endl;
      correlation = 0.99;
    }

    if (correlation == 0.0) {
      // If there is no correlation then we really don't care and don't need
      // to make the fitter do any extra work!
      return;
    }

    if (m1->doNotUse() || m2->doNotUse()) {
      return;
    }

    // The name we are going to use.
    string statSysErrorName (string("Correlated-") + m1->What() + "-" + m1->Name() + "-" + m2->Name());
    if (statSysErrorName.size() > cMaxParameterNameLength) {
      statSysErrorName = statSysErrorName.substr(0, cMaxParameterNameLength);
    }

    //
    // Now do some detailed checks about the sys error
    //  - Only one stat error correlation per two measruements, for example.
    //

    if (m1->hasSysError(statSysErrorName)
	|| m2->hasSysError(statSysErrorName)) {
      ostringstream err;
      err << "Only a single correlation can be established between two measurements: "
	  << m1->What()
	  << " (" << m1->Name() << ", " << m2->Name() << ")";
      throw runtime_error (err.str().c_str());
    }

    //
    // We have to decide how to split the correlated and uncorelated errors between
    // the two measurements. There is no unique solution. The simplest thing - make
    // one of the uncorrelated errors zero - doesn't work with lots of measurements;
    // roofit fails to converge sensibly when this happens.
    //
    // Instead, we choose it such that the uncorrelated errors are equal between the
    // two measurements.
    //
    // The equns you solve for this a fairly trival - but the result below in code
    // is unforutnately opaque.
    //

    // Stat errors of the two measurements
    double s1 = m1->GetStatisticalError()->getVal();
    double s2 = m2->GetStatisticalError()->getVal();
    double rho = correlation;

    //
    // first, check to see if this is going to drive us into a "bad" region - where
    // the combination weights would be less than zero or greater than one.
    // We test this by doing a straight combination and calculating the weight. If it's
    // range is ok, then we can do the fit. Otherwise, no!
    //

    double wt = (s2*s2 - rho*s1*s2)/(s1*s1 + s2*s2 - 2*rho*s1*s2);
    if (wt > 1.0 || wt < 0.0) {
      cout << "WARNING: Correlation coefficient leads to impossible region of phase space." << endl
	   << "  Cannot combine measurements!" << endl
	   << "  #1: " << m1->Name() << endl
	   << "     stat err = " << s1 << endl
	   << "  #2: " << m2->Name() << endl
	   << "     stat err = " << s2 << endl
	   << "  rho = " << rho << " (weight w1 = " << wt << ")" << endl;
      if (s1 > s2) {
	cout << "  Keeping measurement #2" << endl;
	m1->setDoNotUse (true);
      } else {
	cout << "  Keeping measurement #1" << endl;
	m2->setDoNotUse (true);
      }
    }

    // Solve a quad eqn for s2c (s2, the correlated component)
    double a = 1.0;
    double b = s1*s1 - s2*s2;
    double c = rho*s1*s2;
    c = -c*c;

    double radical = b*b - 4*a*c;
    if (radical < 0) {
      ostringstream err;
      err << "Unable to solve for corelated error: s1 = " << s1
	  << " s2 = " << s2 << " and rho = " << rho;
      throw runtime_error(err.str().c_str());
    }
    radical = sqrt(radical);

    double s2c_sol1 = (-b + radical) / 2.0;
    double s2c_sol2 = (-b - radical) / 2.0;

    double s2c_2 = 0;
    if (s2c_sol1 >= 0) {
      s2c_2 = s2c_sol1;
    } else if (s2c_sol2 >= 0) {
      s2c_2 = s2c_sol2;
    } else {
      ostringstream err;
      err << "The Corelated error squared is less than zero: s1 = " << s1
	  << " s2 = " << s2 << " and rho = " << rho;
      throw runtime_error(err.str().c_str());
    }

    double s2c = sqrt(s2c_2);
      
    double s2_2 = s2*s2;

    double s2u_2 = s2_2 - s2c_2;
    if (s2u_2 < 0) {
      ostringstream err;
      err << "s2u_2 was less than zero: s1 = " << s1
	  << " s2 = " << s2 << " and rho = " << rho;
      throw runtime_error(err.str().c_str());
    }
    double s2u = sqrt(s2u_2);

    double s1c = 0.0;
    if (s2c == 0.0) {
      if (rho == 0.0) {
	s2c = 0.0;
      } else {
	ostringstream err;
	err << "s2u is zero, but rho isn't: s1 = " << s1
	    << " s2 = " << s2 << " and rho = " << rho;
	throw runtime_error(err.str().c_str());
      }
    } else {
      s1c = rho*s1*s2/s2c;
    }

    double s1u = s2u;

    m1->ResetStatisticalError(s1u);
    m2->ResetStatisticalError(s2u);

    m1->addSystematicAbs(statSysErrorName, s1c);
    m2->addSystematicAbs(statSysErrorName, s2c);

    //cout << "Stat Correlation Calc: " << endl
    //<< "  s1 = " << s1 << endl
    //<< "  s2 = " << s2 << endl
    //<< "  rho = " << rho << endl
    //<< "  s1u = " << s1u << endl
    //<< "  s2u = " << s2u << endl
    //<< "  s1c = " << s1c << endl
    //<< "  s2c = " << s2c << endl
    //<< "  a=" << a << " b=" << b << " c=" << c << endl;

    CorrInfo cr;
    cr._m1 = m1;
    cr._m2 = m2;
    cr._errorName = errorName;
    cr._sharedSysName = statSysErrorName;
    _correlations.push_back(cr);
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

    // Helper to return a list of good measurements
    vector<Measurement*> GoodMeasurements(vector<Measurement*> &all)
    {
      vector<Measurement*> gMeas;
      for (vector<Measurement*>::const_iterator imeas = all.begin(); imeas != all.end(); imeas++) {
	if (!(*imeas)->doNotUse())
	  gMeas.push_back(*imeas);
      }

      return gMeas;
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

    if (measurementName.size() > cMaxParameterNameLength) {
      ostringstream err;
      err << "Parameter names is too long and will cause a crash in RooFit::migrad - "
	  << "'" << measurementName << "'";
      throw runtime_error (err.str().c_str());
    }

    //
    // Create the variable we are going to be fitting for, and a data point and an
    // object to hand back.
    //

    RooRealVar* whatVar = _whatMeasurements.FindOrCreateRooVar(what, minValue, maxValue);
    whatVar->setVal(value);

    Measurement *m = new Measurement(measurementName, what, value, statError);
    _measurements.push_back(m);
    return m;
  }

  Measurement *CombinationContext::AddMeasurement(const string &what, const double minValue, const double maxValue,
						  const double value, const double statError) {
    return AddMeasurement(NewMeasurementName(what), what, minValue, maxValue, value, statError);
  }

  //
  // Find the measurements if we can - otherwise blow this off and return null.
  //
  Measurement *CombinationContext::FindMeasurement(const string &measurementName)
  {
    for (size_t i = 0; i < _measurements.size(); i++) {
      if (_measurements[i]->Name() == measurementName)
	return _measurements[i];
    }
    return 0;
  }

  RooCmdArg SigmaRange (const RooRealVar &v, const double sigma = 5.0)
  {
    double low = v.getVal() - sigma*v.getError();
    double high = v.getVal() + sigma*v.getError();
    return RooFit::Range(low, high);
  }

  //
  // If a gaussian error is less than 1% then we must bump it up, unfortunately.
  //
  void CombinationContext::AdjustTooSmallGaussians()
  {
    for (vector<Measurement*>::iterator itr = _measurements.begin(); itr != _measurements.end(); itr++) {
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
    for (vector<Measurement*>::const_iterator itr = _measurements.begin(); itr != _measurements.end(); itr++) {
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
	    CheckForAndDisableOverCorrelation (itr->second[i_1], itr->second[i_2]);
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

    vector<Measurement*> gMeas (GoodMeasurements(_measurements));

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
    for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++) {
      Measurement *m(*imeas);

      ///
      /// The variable we are measureing is also balenced by the various
      /// systematic errors for this measurement. eff+m1*s1+m2*s2+m3*s3...
      ///

      RooRealVar *var = _whatMeasurements.FindRooVar(m->What());

      RooArgList varAddition;
      //cout << "What goes into the addition:" << endl;
      //cout << "The main variable" << endl;
      //var->Print();
      varAddition.add(*var);

      vector<string> errorNames (m->GetSystematicErrorNames());
      for (vector<string>::const_iterator isyserr = errorNames.begin(); isyserr != errorNames.end(); isyserr++) {
	const string &errName(*isyserr);
	RooAbsReal *weight = m->GetSystematicErrorWeight(*_systematicErrors.FindRooVar(errName));
	//cout << "A weight:" << endl;
	//weight->Print();

	varAddition.add(*weight);
      }

      //cout << "The addition:" << endl;
      //varAddition.Print();
      
      string internalName = "InternalAddition" + m->Name();
      RooAddition *varSumed = new RooAddition (internalName.c_str(), internalName.c_str(), varAddition);

      ///
      /// The actual variable and th esystematic error are also inputs into this
      /// guassian.
      ///

      RooRealVar *actualValue = (m->GetActualMeasurement());
      RooConstVar *statValue = (m->GetStatisticalError());

      //cout << "Actual value: " << endl;
      //actualValue->Print();
      //cout << "Stat value" << endl;
      //statValue->Print();

      ///
      /// Finally, built the gaussian. Make sure that its name is not
      /// the same as anything else... or *very* odd errors (with/out error messages)
      /// show up later!
      ///

      string gName = m->Name() + "Gaussian";
      RooGaussian *g = new RooGaussian(gName.c_str(), gName.c_str(),
				       *actualValue, *varSumed, *statValue);

      //cout << "Measurement " << gName << endl;
      //g->Print();

      measurementGaussians.push_back(g);
    }

    ///
    /// Get a list of all the measurement gaussians and all the systematic error constraints, and create the product PDF
    /// for the minimization.
    ///

    RooArgList products;
    for (vector<RooAbsPdf*>::iterator itr = measurementGaussians.begin(); itr != measurementGaussians.end(); itr++) {
      products.add(**itr);
      //(*itr)->Print();
    }

    //cout << "Product Gaussian:" << endl;
    //products.Print();

    RooConstVar *zero = new RooConstVar("zero", "zero", 0.0);
    RooConstVar *one = new RooConstVar("one", "one", 1.0);
    vector<string> allVars = _systematicErrors.GetAllVars();

    //cout << "Constraint guassians" << endl;
    for(vector<string>::const_iterator iVar = allVars.begin(); iVar != allVars.end(); iVar++) {
      string cName = *iVar + "ConstraintGaussian";
      RooRealVar *c = _systematicErrors.FindRooVar(*iVar);
      //RooGaussian *constraint = new RooGaussian (cName.c_str(), cName.c_str(), *zero, *c, *one);
      RooGaussian *constraint = new RooGaussian (cName.c_str(), cName.c_str(), *c, *zero, *one);
      products.add(*constraint);
      //constraint->Print();
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

    cout << "Starting the master fit..." << endl;
    //RooMsgService::instance().setSilentMode(false);
    //RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
    RooFitResult *fitResult = finalPDF.fitTo(measuredPoints, RooFit::Strategy(cMINUITStrat), RooFit::Save());
    //RooMsgService::instance().setSilentMode(true);
    //RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    cout << "Back!" << endl;
    //fitResult->Print();
    //cout << "EDM = " << fitResult->EDM() << endl;
    cout << "Master fit is finished..." << endl;

    //
    // To actually calculate the chi2 we have a fair amount of work to do.
    // Using the method from the BLUE paper, eqn 14 (loosely based on this, actually).
    //  (published xxx)
    //

    {
      // Get the resulting covarience matrix
      
    }

    ///
    /// Dump out the graph-viz tree
    ///
    
    finalPDF.graphVizTree("combined.dot");

    //
    // Do the chi2 for this.
    //

#ifdef notyet
    cout << "Creating chi2" << endl;
    try {
      RooXYChi2Var globalChi2 ("chi2_global", "chi2_global", finalPDF, measuredPoints);
      cout << "Creating chi2" << endl;
      double g_gChi2 = globalChi2.getVal();
      cout << "Creating chi2" << endl;
      double g_nDOF = fitResult->floatParsFinal().getSize();
      cout << "Creating chi2" << endl;
      cout << " -> Global chi2: " << g_gChi2 << ", nDOF: " << g_nDOF
	   << ", chi2/nDOF: " << g_gChi2/g_nDOF
	   << endl;
      cout << "Creating chi2" << endl;
    } catch (string &s) {
      cout << "error! " << s << endl;
      throw;
    }
#endif

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
    // Extract a crude chi2
    //
    
    map<string, double> individualChi2;
    for (vector<Measurement*>::const_iterator imeas = gMeas.begin(); imeas != gMeas.end(); imeas++) {
      Measurement *m(*imeas);
      
      if (individualChi2.find(m->What()) == individualChi2.end())
	individualChi2[m->What()] = 0.0;

      double cv = m->centralValue();
      double err = m->totalError();
      double meas = result[m->What()].centralValue;

      double delta = meas - cv;
      double chi2Contrib = delta*delta/(err*err);
      individualChi2[m->What()] += chi2Contrib;

      cout << "Chi2 Contrib for " << m->Name() << " is " << chi2Contrib << endl;
    }

    double totalChi2 = 0.0;
    for (map<string,double>::const_iterator itr = individualChi2.begin(); itr != individualChi2.end(); itr++) {
      cout << "Chi2 contrib for bin " << itr->first << " is " << itr->second << endl;
      totalChi2 += itr->second;
    }
    cout << "Total chi2 for " << name << ": " << totalChi2 << endl;

    //
    // Dump out the pulls that the fit settled on... so this crudely fornow.
    //

    for(vector<string>::const_iterator iVar = allVars.begin(); iVar != allVars.end(); iVar++) {
      RooRealVar *c (_systematicErrors.FindRooVar(*iVar));
      cout << " Sys " << *iVar << " pull: " << c->getVal() << " +- " << c->getError() << endl;
    }

    ///
    /// Now that the fit is done, dump out a root file that contains some good info
    ///

    {
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
	finalPDF.fitTo(measuredPoints, RooFit::Strategy(cMINUITStrat));

	// Loop over all measurements. If the measurement knows about
	// this systematic error, then extract a number from it.
	//

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

	    sysErr->setConstant(false);
	    sysErr->setVal(sysErrOldVal);
	    sysErr->setError(sysErrOldError);
	  }
	}
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

    cout << "  Refit to restore the state." << endl;
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
    // Return all the final results.
    //

    return result;
  }

  //
  // Calculate the statistical errors for each measurement, ignoring the
  // systematic errors totally.
  //
  map<string, double> CombinationContext::CalculateStatisticalErrors()
  {
    map<string, double> result;

    // Get the sub-set of measurements that we can use.
    vector<Measurement*> gMeas (GoodMeasurements(_measurements));

    // Catalog them by what is being measured.
    map<string, vector<Measurement*> > byItem;
    for(vector<Measurement*>::const_iterator itr = gMeas.begin(); itr != gMeas.end(); itr++) {
      Measurement *m (*itr);
      byItem[m->What()].push_back(m);
    }

    // For each item, calculate the new central value and the statistical error
    for(map<string, vector<Measurement*> >::const_iterator itr = byItem.begin(); itr != byItem.end(); itr++) {
      double s2 = 0.0;
      for (vector<Measurement*>::const_iterator m_itr = itr->second.begin(); m_itr != itr->second.end(); m_itr++) {
	Measurement *m (*m_itr);
	double stat_error (m->statError());
	double wt = 1.0 / (stat_error*stat_error);
	s2 += wt;
      }
      result[itr->first] = sqrt(1.0/s2);
    }

    return result;
  }

  // Is this sys error valid for this particular measurement?
  bool CombinationContext::sysErrorUsedBy (const std::string &sysErrName, const std::string &whatVariable)
  {
    for (size_t i = 0; i < _measurements.size(); i++) {
      const Measurement *m (_measurements[i]);
      if (whatVariable == m->What() && !m->doNotUse()) {
	if (m->hasSysError(sysErrName))
	  return true;
      }
    }

    return false;
  }

  // Dump a fit result out to an output stream (mostly for debugging)
  ostream &operator<< (ostream &out, const CombinationContext::FitResult &fr)
  {
    out << "fit cv: " << fr.centralValue << " +- " << fr.statisticalError << endl;
    if (fr.sysErrors.size() == 0) {
      out << "  0 systematic errors" << endl;
    } else {
      for (map<string, double>::const_iterator itr = fr.sysErrors.begin(); itr != fr.sysErrors.end(); itr++) {
	cout << "  sys " << itr->first << " +- " << itr->second << endl;
      }
    }
    return out;
  }
}
