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
#ifdef notyet
  Measurement::Measurement(const string &measurementName, const string &what, const double val, const double statError)
    : _name(measurementName), _what(what),
      _actualValue(_name.c_str(), _name.c_str(), val),
      _statError((_name + "StatError").c_str(), (_name + "StatError").c_str(), statError)
  {
    _actualValue.setConstant(true);
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

  ///
  /// Fetch back the w*s weighting. Keep the objects around so we can
  /// deal with Roo's funny ownership rules.
  ///
  RooAbsReal *Measurement::GetSystematicErrorWeight (RooRealVar &error)
  {
    auto s1WidthName = string(error.GetName()) + "Width";
    auto s1WidthProduct = string(error.GetName()) + "Product";
    auto s1WidthSumName = string(error.GetName()) + "Weight";

    /// Q: Why is innerWidht allowed as a pointer, but that causes a crash?

    auto s1Width = new RooConstVar(s1WidthName.c_str(), s1WidthName.c_str(), GetSystematicErrorWidth(error.GetName()));
    auto innerWidth = new RooProduct (s1WidthProduct.c_str(), s1WidthProduct.c_str(), RooArgList(error, *s1Width));

    return innerWidth;
  }

  ///
  /// Return a list of the systematic errors that we are "using".
  ///
  vector<string> Measurement::GetSystematicErrorNames(void) const
  {
    vector<string> result;
    transform(_sysErrors.begin(), _sysErrors.end(),
	      back_inserter(result),
	      [&result] (const pair<string, double> &item) { return item.first; } );
    return result;
  }

  ///
  /// Get back the width of an error. Throw if we don't know about the error.
  ///
  double Measurement::GetSystematicErrorWidth (const std::string &errorName) const
  {
    auto err = find_if(_sysErrors.begin(), _sysErrors.end(), 
		       [&errorName] (const pair<string, double> &item) { return item.first == errorName; } );
    if (err == _sysErrors.end())
      throw runtime_error ("Don't know about error '" + errorName + "'.");
    return err->second;
  }

#endif
}
