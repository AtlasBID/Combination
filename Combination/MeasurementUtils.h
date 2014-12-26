//
// Some utility functions to help with calculating things for measurements that 
// don't really need measurement internals, or operate on collections of measurements.
//
#ifndef COMBINATION_MeasurementUtils
#define COMBINATION_MeasurementUtils

#include <TMatrixTSym.h>

#include <vector>

namespace BTagCombination {

  class Measurement;

  TMatrixTSym<double> CalcCovarMatrixUsingRho (const std::vector<Measurement*> &measurements);
  TMatrixTSym<double> CalcCovarMatrixUsingComposition (const std::vector<Measurement*> &measurements);

  // Calculate the fully correlated value of the chi2 for a sequence of measurements.
  double CalcChi2(const std::vector<Measurement*> &measurements, const std::vector<Measurement*> &fitResults);
}

#endif
