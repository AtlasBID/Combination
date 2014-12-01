#ifndef __CalibrationFilterUtils__
#define __CalibrationFilterUtils__

#include "Combination/CalibrationDataModel.h"

#include <vector>
#include <string>

namespace BTagCombination {

	// What can be filtered out, and a method that filters everythign out.
	// Note: operatingPoints is an in/out argument! :(
	struct calibrationFilterInfo {
		std::vector<std::string> OPsToIgnore;
		std::vector<std::string> spOnlyFlavor, spOnlyTagger, spOnlyOP, spOnlyJetAlgorithm, spOnlyAnalysis;
	};
	void FilterAnalyses(CalibrationInfo &operatingPoints, const calibrationFilterInfo &fInfo);
}

#endif
