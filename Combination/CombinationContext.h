///
/// The context for a combinatoin of several different measurements of the same thing (or things),
/// along with possibly shared systematic errors.
///
#ifndef COMBINATION_CombinationContext
#define COMBINATION_CombinationContext

#include "Combination/CombinationContextBase.h"

#include <string>
#include <vector>
#include <map>

namespace BTagCombination {

  class CombinationContext : public CombinationContextBase
  {
  public:
    /// Create/Destroy a new context. This will contain the common
    /// data to do a fit to mutliple measurements.
    CombinationContext(void);
    ~CombinationContext(void) {};

    /// Fit all the measurements that we've asked for, and return results for each measurement done.
    std::map<std::string, FitResult> Fit(const std::string &name = "");

    /// Turn on/off production of plots. Plots are expensive!
    inline void setDoPlots(bool v = false) { _doPlots = v;}

    inline void SetVerbose (bool v) { _verbose = v; }

  private:
    // How quiet should we be? Mouse like is false.
    bool _verbose;

    /// Should we make plots as a diagnostic output?
    bool _doPlots;

    // Any common measurements that are over correlated are "bad"
    void TurnOffOverCorrelations();

    // Any gaussian errors too small, we fix here.
    void AdjustTooSmallGaussians();

  };
}

#endif

