///
// An abstract fitter class. The full parallel fit along with all sorts of others are based on
// this guy.
///
#ifndef COMBINATION_CombinationContextBase
#define COMBINATION_CombinationContextBase

#include "Combination/RooRealVarCache.h"

#include <string>
#include <vector>
#include <map>

class RooRealVar;

namespace BTagCombination {

  class Measurement;
  class CombinationContextBase
  {
  public:

    // Results of the fit for a particular measurement
    struct FitResult
    {
      double centralValue;
      double statisticalError;

      std::map<std::string, double> sysErrors;
    };

    class ExtraFitInfo
    {
    public:
      double _globalChi2; // The total chi21
      double _ndof; // The degrees of freedom

      std::map<std::string, double> _pulls; // Pulls from the fit.
      std::map<std::string, double> _nuisance; // Nuisance from the fit.

      void clear();
    };

    // Clean up
    virtual ~CombinationContextBase(void);

    /// Create a new measurement. You are trying to measure "what", and with
    /// this particular measurement you found a value "value", and statistical
    /// error "statError". Min and Max values are the min and max values of 'what'.
    /// You can name your measurement or let the code choose a generic name for you.
    /// Names must be unique!
    virtual Measurement *AddMeasurement (const std::string &measurementName,
					 const std::string &what,
					 const double minValue,
					 const double maxValue,
					 const double value,
					 const double statError);

    virtual Measurement *AddMeasurement (const std::string &what,
					 const double minValue,
					 const double maxValue,
					 const double value,
					 const double statError);

    // Return a measurement
    virtual Measurement *FindMeasurement (const std::string &measurementName);

    /// Add a correlation between two measurements for a particular error. If errorName is
    /// "statistical" then the statistical error is what is "marked".
    virtual void AddCorrelation(const std::string &errorName, Measurement *m1, Measurement *m2, double correlation);

    /// Fit all the measurements that we've asked for, and return results for each measurement done.
    virtual std::map<std::string, FitResult> Fit(const std::string &name = "") = 0;

    /// Returns the stat-only error on each measurement.
    virtual std::map<std::string, double> CalculateStatisticalErrors(void);

    // Get the extra info from a fit that was just run.
    virtual ExtraFitInfo GetExtraFitInformation (void) { return _extraInfo;}

  protected:
    // Helper method that scans the intenral list of measruements to get
    // a list of the good ones (i.e. that are participating in the fit).
    std::vector<Measurement*> GoodMeasurements(std::vector<Measurement*> &all);

    // Is this sys error connected by this measurement?
    bool sysErrorUsedBy(const std::string &sysErrName, const std::string &what);

  private:
    ExtraFitInfo _extraInfo;

    // Keep track of all the measurements.
    RooRealVarCache _whatMeasurements;

    // Keep track fo all the systematic errors between the various measurements.
    RooRealVarCache _systematicErrors;

    // Keep a list of all measurements
    std::vector<Measurement*> _measurements;


    // Keep a list of the correlations we are dealing with so we can put them back
    // together after a fit!
    struct CorrInfo {
      Measurement *_m1;
      Measurement *_m2;
      std::string _errorName;
      std::string _sharedSysName;
    };
    std::vector<CorrInfo> _correlations;
  };

  // Dump a fit result out.
  std::ostream &operator<< (std::ostream &out, const CombinationContextBase::FitResult &fr);
}

#endif

