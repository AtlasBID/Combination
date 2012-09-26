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

  TMatrixTSym<double> CalcCovarMatrixUsingRho (const std::vector<Measurement*> measurements);
}

#endif
