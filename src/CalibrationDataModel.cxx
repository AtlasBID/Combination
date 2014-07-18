// Code to help with the data model (comparison operators, etc.)

#include "Combination/CalibrationDataModel.h"

#include <algorithm>

using namespace std;

namespace BTagCombination {

  bool doubleEqual (double p1, double p2) {
    double m = std::min(p1, p2);
    double fraction = fabs(p1-p2)/m;
    return fraction < 0.0001;
  }

  // Comparison operators of various types needed to put these guys in sets, etc.
  bool operator< (const CalibrationBinBoundary &x,
		  const CalibrationBinBoundary &y) {
    if (x.variable != y.variable)
      return x.variable < y.variable;
    if (x.lowvalue != y.lowvalue)
      return x.lowvalue < y.lowvalue;
    return x.highvalue < y.highvalue;
  }

  bool operator== (const CalibrationBinBoundary &x,
		   const CalibrationBinBoundary &y) {
    return x.variable == y.variable
      && x.lowvalue == y.lowvalue
      && x.highvalue == y.highvalue;
  }

  bool operator== (const SystematicError &e1, const SystematicError &e2) {
    if (e1.name != e2.name
	|| !doubleEqual(e1.value, e2.value)
	|| e1.uncorrelated != e2.uncorrelated)
      return false;
    return true;
  }

  bool operator== (const CalibrationBin &b1, const CalibrationBin &b2) {
    if (b1.centralValue != b2.centralValue
	|| b1.centralValueStatisticalError != b2.centralValueStatisticalError)
      return false;

    if (b1.isExtended != b2.isExtended)
      return false;

    if (b1.metadata != b2.metadata
	|| b1.binSpec != b2.binSpec
	|| b1.systematicErrors != b2.systematicErrors)
      return false;
    return true;
  }

  bool operator== (const CalibrationAnalysis &a1, const CalibrationAnalysis &a2) {
    if (a1.name != a2.name
	|| a1.flavor != a2.flavor
	|| a1.tagger != a2.tagger
	|| a1.operatingPoint != a2.operatingPoint
	|| a1.jetAlgorithm != a2.jetAlgorithm)
      return false;

    if (a1.bins != a2.bins
	|| a1.metadata != a2.metadata)
      return false;
    return true;
  }
}
