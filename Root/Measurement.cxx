///
/// The value for a single measurement
///
#include "Combination/Measurement.h"

#include <RooRealVar.h>
#include <RooConstVar.h>
#include <RooConstVar.h>
#include <RooProduct.h>
#include <RooAddition.h>

#include <map>
#include <algorithm>
#include <stdexcept>
#include <vector>

using namespace std;

namespace {
  // Deal with a signed number we want to take a square root of by passing
  // it all the way through.
  double ssqrt (double v)
  {
    double r = sqrt(fabs(v));
    if (v < 0.0)
      return -r;
    return r;
  }
}

namespace BTagCombination {

  // Clean up some of the RooFit stuff we are holding onto. Ugh in the extreme.
  Measurement::~Measurement(void)
  {
    for (map<string, RooProduct*>::const_iterator itr = _innerWidthCache.begin(); itr != _innerWidthCache.end(); itr++) {
      delete itr->second;
    }
    for (vector<RooConstVar*>::const_iterator itr = _widthCache.begin(); itr != _widthCache.end(); itr++) {
      delete *itr;
    }
    delete _statError;
  }

  Measurement::Measurement(const string &measurementName, const string &what, const double val, const double statError)
    : _name(measurementName), _what(what),
      _actualValue(_name.c_str(), _name.c_str(), val),
      _statError(new RooConstVar((_name + "StatError").c_str(), (_name + "StatError").c_str(), statError)),
      _doNotUse (false)
  {
    _actualValue.setConstant(true);
  }

  //
  // Reset the statistical error
  //
  void Measurement::ResetStatisticalError (double statErr)
  {
    _statError = new RooConstVar((_name + "StatError").c_str(),
			     (_name + "StatError").c_str(),
			     statErr);
  }

  //
  // Calc the total systematic error in quad
  //
  double Measurement::totalSysError() const
  {
    double tot = 0.0;
    for (size_t i = 0; i < _sysErrors.size(); i++) {
      tot += _sysErrors[i].second*_sysErrors[i].second;
    }
    return sqrt(tot);
  }

  //
  // Return the statistical error
  //
  double Measurement::statError() const
  {
    return _statError->getVal();
  }

  //
  // Return the total error, added in quad
  //
  double Measurement::totalError() const
  {
    double stat = statError();
    double sys = totalSysError();
    return sqrt(stat*stat + sys*sys);
  }

  //
  // Return the central value
  //
  double Measurement::centralValue() const
  {
    return _actualValue.getVal();
  }

  ///
  /// Return a list of the systematic errors that we are "using".
  ///
  vector<string> Measurement::GetSystematicErrorNames(void) const
  {
    vector<string> result;
    for(vector<pair<string,double> >::const_iterator itr = _sysErrors.begin(); itr != _sysErrors.end(); itr++) {
      result.push_back(itr->first);
    }
    return result;
  }

  ///
  /// Do we know about this sys error?
  ///
  bool Measurement::hasSysError (const string &name) const
  {
    for(vector<pair<string,double> >::const_iterator itr = _sysErrors.begin(); itr != _sysErrors.end(); itr++) {
      if (itr->first == name)
	return true;
    }
    return false;
  }

  ///
  /// Fetch back the w*s weighting. Keep the objects around so we can
  /// deal with Roo's funny ownership rules.
  ///
  RooAbsReal *Measurement::GetSystematicErrorWeight (RooRealVar &error)
  {
    string s1WidthName = Name() + error.GetName() + "Width";
    string s1WidthProduct = Name() + error.GetName() + "Product";

    /// Q: Why is innerWidht allowed as a pointer, but that causes a crash?

    map<string, RooProduct*>::const_iterator itr = _innerWidthCache.find(s1WidthProduct);
    if (itr != _innerWidthCache.end()) {
      return itr->second;
    } else {
      RooConstVar *s1Width = new RooConstVar(s1WidthName.c_str(), s1WidthName.c_str(), GetSystematicErrorWidth(error.GetName()));
      _widthCache.push_back(s1Width);
      RooProduct *innerWidth = new RooProduct (s1WidthProduct.c_str(), s1WidthProduct.c_str(), RooArgList(error, *s1Width));
      _innerWidthCache[s1WidthProduct] = innerWidth;
      return innerWidth;
    }
  }

  ///
  /// Get back the width of an error. Throw if we don't know about the error.
  ///
  double Measurement::GetSystematicErrorWidth (const std::string &errorName) const
  {
    for(vector<pair<string,double> >::const_iterator itr = _sysErrors.begin(); itr != _sysErrors.end(); itr++) {
      if (errorName == itr->first) {
	return itr->second;
      }
    }

    throw runtime_error ("Don't know about error '" + errorName + "'.");
  }

  ///
  /// Add a new systematic error to the list of systematic errors.
  ///
  void Measurement::addSystematicAbs (const std::string &errorName, const double oneSigmaSizeAbsoulte)
  {
    _sysErrors.push_back(std::make_pair(errorName, oneSigmaSizeAbsoulte));
  }
  void Measurement::addSystematicRel (const std::string &errorName, const double oneSigmaSizeRelativeFractional)
  {
    addSystematicAbs(errorName,  _actualValue.getVal()*oneSigmaSizeRelativeFractional);
  }
  void Measurement::addSystematicPer (const std::string &errorName, const double oneSigmaSizeRelativePercent)
  {
    addSystematicRel(errorName,  oneSigmaSizeRelativePercent/100.0);
  }

  //
  // Determine what errors are correlated and not correlated between measurements.
  // Include both systematic and statistical errors in the calculation.
  // Returns pair<uncorrelated, correlated>.
  //
  pair<double, double> Measurement::SharedError (const Measurement *other) const
  {
    double corErr2 = 0.0;
    double uncorErr2 = _statError->getVal()*_statError->getVal();

    for (size_t i = 0; i < _sysErrors.size(); i++) {
      double v2r = _sysErrors[i].second;
      double v2 = v2r*v2r;
      if (v2r < 0.0)
	v2 = -v2;

      if (other->hasSysError(_sysErrors[i].first)) {
	corErr2 += v2;
      } else {
	uncorErr2 += v2;
      }
    }

    
    return make_pair(ssqrt(uncorErr2), ssqrt(corErr2));
  }

  // Calculate the covariance coefficient between two measurements
  double Measurement::Rho (const Measurement *other) const
  {
    double rho = RhoUnbounded(other);

    if (rho < -1.0) {
      cout << "Error calculating covariance between "
	   << this->Name() << " and "
	   << other->Name() << " - rho found to be " << rho << endl;
      rho = -1.0;
    }
    if (rho > 1.0) {
      cout << "Error calculating covariance between "
	   << this->Name() << " and "
	   << other->Name() << " - rho found to be " << rho << endl;
      rho = 1.0;
    }
    return rho;
  }

  // Calc the correlation coeff for this measurement and a second one.
  // Let rho be whatever it wants.
  double Measurement::RhoUnbounded (const Measurement *other) const
  {
    // Loop over all systematic errors, calculating the shared systematic (sigma_1j*sigma_2j, over all
    // sys errors j). If a systematic error is missing, it is assumed to be zero.

    map<string, double> myErrors;
    for(size_t i = 0; i < _sysErrors.size(); i++) {
      myErrors[_sysErrors[i].first] = _sysErrors[i].second;
    }

    double sigma12 = 0.0;
    for(size_t i = 0; i < other->_sysErrors.size(); i++) {
      map<string,double>::const_iterator my_i = myErrors.find(other->_sysErrors[i].first);
      if (my_i != myErrors.end()) {
	sigma12 += other->_sysErrors[i].second * my_i->second;
      }
    }

    // Now can calculate rho.

    double s1 = totalError();
    double s2 = other->totalError();

    double rho = sigma12/(s1*s2);

    return rho;
  }

  //
  // Calculate the covar term between this measurement and the
  // one passed in. We calculate this as rho*s1*s2, where s1 and s2
  // are the total error for each measurement. And rho is the
  // correlation factor - how much of the error is shared (should
  // range from zero to 1).
  //
  double Measurement::Covar (const Measurement *other) const
  {
    // Get the total errors
    double s1 = totalError();
    double s2 = other->totalError();

    // Statistical error is treated specially, unfortunately - even if we
    // ask for covar of ourselves, the stat error will be treated as a
    // different type of error, and rho will be incorrectly be calculated
    // as rho < 1. So, special case.

    if (other == this)
      return s1*s2;

    // Now, return the covariance
    double rho = Rho (other);

    return rho*s1*s2;
  }

}
