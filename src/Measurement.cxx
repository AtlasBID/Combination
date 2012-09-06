///
/// The value for a sinle measurement
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

using std::string;
using std::map;
using std::vector;
using std::transform;
using std::find_if;
using std::back_inserter;
using std::runtime_error;

namespace BTagCombination {

  Measurement::~Measurement(void)
  {
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
  // See if the stat error is too small, artifiically boost it if it is.
  //
  void Measurement::CheckAndAdjustStatisticalError(double minFraction)
  {
    double fract = _statError->getVal() / _actualValue.getVal();
#ifdef notyet
    if (fract < minFraction) {
      double newStatError = _actualValue.getVal() * minFraction;
      cout << "WARNING: Statistical error is too small for fitting to succeed." << endl
	   << "  " << Name() << " (" << What() << ")" << endl
	   << "  central value " << _actualValue.getVal() << " +- " << _statError->getVal() << " (stat) +- " << totalSysError() << " (sys)" << endl
	   << "  Adjusting the stat error to " << newStatError << endl;
      ResetStatisticalError(newStatError);
    }
#endif
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

    RooConstVar *s1Width = new RooConstVar(s1WidthName.c_str(), s1WidthName.c_str(), GetSystematicErrorWidth(error.GetName()));
    RooProduct *innerWidth = new RooProduct (s1WidthProduct.c_str(), s1WidthProduct.c_str(), RooArgList(error, *s1Width));

    return innerWidth;
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
  /// Add a new systematic error to the list of systeematic errors.
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
  // Determine what errosr are correlated and not correlated between measurements.
  // Include both systeamtic and statistical errors in the calculation.
  // Returns pair<uncorrelated, correlated>.
  //
  pair<double, double> Measurement::SharedError (const Measurement *other) const
  {
    double corErr2 = 0.0;
    double uncorErr2 = _statError->getVal()*_statError->getVal();

    for (size_t i = 0; i < _sysErrors.size(); i++) {
      double v2 = _sysErrors[i].second;
      v2 = v2*v2;
      if (other->hasSysError(_sysErrors[i].first)) {
	corErr2 += v2;
      } else {
	uncorErr2 += v2;
      }
    }

    return make_pair(sqrt(uncorErr2), sqrt(corErr2));
  }
}
