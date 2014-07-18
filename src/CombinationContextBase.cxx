///
/// Implementation of the context for a combination of sevearl measurements.
///

#include "Combination/CombinationContextBase.h"
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

using namespace std;

namespace {

  // Max length of a parameter we allow into RooFit to prevent a crash.
  // It does change with RooFit version number...
  const size_t cMaxParameterNameLength = 90;

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

namespace BTagCombination {

  // Clean up
  void CombinationContextBase::ExtraFitInfo::clear (void)
  {
    _globalChi2 = 0.0;
    _ndof = 0.0;
  }

  ///
  /// Clean up everything.
  ///
  CombinationContextBase::~CombinationContextBase(void) {
    for (unsigned int im = 0; im < _measurements.size(); im++) {
      delete _measurements[im];
    }
  }

  //
  // establish a correlation between two measurements for one of the
  // errors.
  //
  void CombinationContextBase::AddCorrelation (const std::string &errorName,
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

  //
  // What are the good measruements? Return them.
  //
  vector<Measurement*> CombinationContextBase::GoodMeasurements(void)
  {
    vector<Measurement*> gMeas;
    for (vector<Measurement*>::const_iterator imeas = _measurements.begin(); imeas != _measurements.end(); imeas++) {
      if (!(*imeas)->doNotUse())
	gMeas.push_back(*imeas);
    }
    
    return gMeas;
  }

  ///
  /// Add a new measurement to our internal list.
  ///
  Measurement *CombinationContextBase::AddMeasurement(const string &measurementName,
						      const string &what,
						      const double minValue,
						      const double maxValue,
						      const double value,
						      const double statError) {
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

  //
  // Add a new measurement, but make up the name (i.e. the caller really doesn't care
  // what we are talking about here!
  //
  Measurement *CombinationContextBase::AddMeasurement(const string &what,
						      const double minValue,
						      const double maxValue,
						      const double value,
						      const double statError) {
    return AddMeasurement(NewMeasurementName(what), what, minValue, maxValue, value, statError);
  }

  //
  // Find the measurements if we can - otherwise blow this off and return null.
  //
  Measurement *CombinationContextBase::FindMeasurement(const string &measurementName)
  {
    for (size_t i = 0; i < _measurements.size(); i++) {
      if (_measurements[i]->Name() == measurementName)
	return _measurements[i];
    }
    return 0;
  }

  //
  // Calculate the statistical errors for each measurement, ignoring the
  // systematic errors totally.
  //
  map<string, double> CombinationContextBase::CalculateStatisticalErrors()
  {
    map<string, double> result;

    // Get the sub-set of measurements that we can use.
    vector<Measurement*> gMeas (GoodMeasurements());

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
  bool CombinationContextBase::sysErrorUsedBy (const std::string &sysErrName, const std::string &whatVariable)
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
  ostream &operator<< (ostream &out, const CombinationContextBase::FitResult &fr)
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
