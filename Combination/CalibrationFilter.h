#ifndef __CalibrationFilterUtils__
#define __CalibrationFilterUtils__

#include "Combination/CalibrationDataModel.h"

#include <map>
#include <string>
#include <boost/regex.hpp>

namespace BTagCombination {

  // What can be filtered out, and a method that filters everythign out.
  // Note: operatingPoints is an in/out argument! :(
  struct calibrationFilterInfo {
    // The logical way to do this is with a set, however older versions of gcc/boost can't
    // do this.
    std::map<std::string, boost::regex*> OPsToIgnore;
    std::map<std::string, boost::regex*> spOnlyFlavor, spOnlyTagger, spOnlyOP, spOnlyJetAlgorithm, spOnlyAnalysis;
  };
  void FilterAnalyses(CalibrationInfo &operatingPoints, const calibrationFilterInfo &fInfo);
}

#endif
