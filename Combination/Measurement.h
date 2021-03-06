///
/// Code for a particular measurement of some "value" (like an efficiency).
/// User creates using CombinationContext::AddMeasurement.
///

#ifndef COMBINATION_Measurement
#define COMBINATION_Measurement

#include <RooRealVar.h>
#include <RooConstVar.h>
#include <RooAbsReal.h>

#include <string>
#include <vector>
#include <map>


class RooAbsReal;
class RooProduct;

namespace BTagCombination {
  class CombinationContext;
  class CombinationContextBase;

  class Measurement
  {
  public:
    /// Add a new systematic error
    void addSystematicAbs (const std::string &errorName, const double oneSigmaSizeAbsolute);
    void addSystematicRel (const std::string &errorName, const double oneSigmaSizeRelativeFractional);
    void addSystematicPer (const std::string &errorName, const double oneSigmaSizePercent);

    void ResetStatisticalError (double statErr);

    inline const std::string &Name(void) const
      { return _name; }
    inline const std::string &What(void) const
      { return _what; }
    bool hasSysError (const std::string &name) const;

    // Get/Set the do not use flag. If set, then ignore this measurement
    // when doing the combination.
    // Use this when you have to wait until the full context is built before you decide that you
    // can't use. Also used internally to prevent bad combinations from occuring.
    void setDoNotUse (bool v) { _doNotUse = v; }
    bool doNotUse (void) const { return _doNotUse; }

    // Returns the error that is uncorrelated, correlated with another measurement.
    // This erorr calculation takes into account negative correlations!
    // Returns (<uncorrelated, correlated> error).
    std::pair<double, double> SharedError (const Measurement *other) const;

    // Calculate the covar(m1, m2)
    double Covar (const Measurement *other) const;

    // Calculate rho, the covar coeff. Bounded by 1 (explicitly if the test fails)
    double Rho (const Measurement *other) const;

    // Calculate rho, the covar coeff. Bounded by 1 if all goes well, otherwise... not.
    double RhoUnbounded (const Measurement *other) const;

    // Returns the total systematic error
    double totalSysError() const;

    // Returns the statistical error
    double statError() const;

    // Returns the total error (in quad)
    double totalError() const;

    // Returns the central value
    double centralValue() const;

    // Get the complete list of systematic errors we know about
    std::vector<std::string> GetSystematicErrorNames(void) const;
    double GetSystematicErrorWidth (const std::string &errorName) const;

  private:

    RooRealVar *GetActualMeasurement() {return &_actualValue;}
    RooConstVar *GetStatisticalError() {return _statError;}

    std::map<std::string, RooProduct*> _innerWidthCache;
    std::vector<RooConstVar*> _widthCache;

  private:
    /// The context is allowed access to everything.
    friend class CombinationContext;
    friend class CombinationContextBase;

    /// Only the Context can create a new measurement.
    Measurement(const std::string &measurementName, const std::string &what, const double val, const double statError);

    ~Measurement(void);


    /// Keep track...
    const std::string _name;
    const std::string _what;

    std::vector<std::pair<std::string, double> > _sysErrors;

    /// Variables we'll need later
    RooRealVar _actualValue;
    RooConstVar *_statError;

    inline std::string NameStat(void) const
      { return Name() + "StatError";}

    /// Systematic Error Access
    RooAbsReal *GetSystematicErrorWeight (RooRealVar &error);
    
    // Ignore this in a calculation.
    bool _doNotUse;
  };
}

#endif

