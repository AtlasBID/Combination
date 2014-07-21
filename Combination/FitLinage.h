// FitLinage.h
//
//  Some simple functions to help with dealing with fit linage tracking.

#ifndef FITLINAGE_COMBINER
#define FITLINAGE_COMBINER

#include "Combination/CalibrationDataModel.h"
#include <string>

namespace BTagCombination
{
	// Returns the linage for this analysis
	std::string Linage(const CalibrationAnalysis &ana);

	enum LinageCombinationOperator {
		LCFitCombine // Regular fit combination (bin-by-bin or normal fit)
	};

	enum LinageBinaryOperator {
		LBDStar, // DStar calc on two templates
		LBAddSys, // Add a systematic error to the list
		LBExtrapolate // Extrapolate an analysis
	};

	// Combine with a specific operator that is connected
	// with how the combination is done.
	std::string CombineLinage(const std::vector<CalibrationAnalysis> &ana, LinageCombinationOperator how);

	// A binary operation
	std::string BinaryLinageOp(const CalibrationAnalysis &a1, const CalibrationAnalysis &a2, LinageBinaryOperator how);
	std::string BinaryLinageOp(const CalibrationAnalysis &a1, const std::string &a2, LinageBinaryOperator how);
}

#endif
