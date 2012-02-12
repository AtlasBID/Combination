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

namespace BTagCombination {
  class CombinationContext;

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

    // Get/Set the do not use flab. If set, then ignore this measurement
    // when doing the combination.
    // Use this when you have to wait until the full context is built before you decide that you
    // can't use. Also used internally to prevent bad combinations from occuring.
    void setDoNotUse (bool v) { _doNotUse = v; }
    bool doNotUse (void) const { return _doNotUse; }

    // Returns the error that is uncorrelated, correlated with another measurement.
    std::pair<double, double> SharedError (const Measurement *other) const;

    // Check and adjust a systematic error. Default argument, if it is
    // less than 0.5%, then adjust it till it is (and print out a warning).
    void CheckAndAdjustStatisticalError (double minFraction = 0.005);

    // Returns the total systematic error
    double totalSysError() const;

  private:

    RooRealVar *GetActualMeasurement() {return &_actualValue;}
    RooConstVar *GetStatisticalError() {return _statError;}

  private:
    /// The context is allowed access to everything.
    friend class CombinationContext;

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
    std::vector<std::string> GetSystematicErrorNames(void) const;
    RooAbsReal *GetSystematicErrorWeight (RooRealVar &error);
    double GetSystematicErrorWidth (const std::string &errorName) const;
    
    // Ignore this in a calculation.
    bool _doNotUse;
  };
}

#endif

