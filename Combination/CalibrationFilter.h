#ifndef __CalibrationFilterUtils__
#define __CalibrationFilterUtils__

#include "Combination/CalibrationDataModel.h"

#include <set>
#include <string>
#include <boost/regex.hpp>

namespace BTagCombination {

  // What can be filtered out, and a method that filters everythign out.
  // Note: operatingPoints is an in/out argument! :(
  struct calibrationFilterInfo {
    std::set<const boost::regex> OPsToIgnore;
    std::set<const boost::regex> spOnlyFlavor, spOnlyTagger, spOnlyOP, spOnlyJetAlgorithm, spOnlyAnalysis;
  };
  void FilterAnalyses(CalibrationInfo &operatingPoints, const calibrationFilterInfo &fInfo);
}

#endif
