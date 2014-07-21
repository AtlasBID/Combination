// FitLinage.cpp
//
// A few helper functions

#include "Combination/FitLinage.h"

#include <stdexcept>
#include <sstream>

using namespace std;

namespace {
	using namespace BTagCombination;

	// Return string representation of the linar combination operator.
	string getOperator(LinageCombinationOperator op)
	{
		switch (op)
		{
		case BTagCombination::LCFitCombine:
			return "+";
			break;
		default:
			throw runtime_error("Unknown linage combination operator");
		}
	}
}

namespace BTagCombination {

	// Return the linage of this calibration. If there is none, just return the name.
	string Linage(const CalibrationAnalysis &ana)
	{
		map<string, string>::const_iterator l = ana.metadata_s.find("Linage");
		if (l == ana.metadata_s.end())
			return ana.name;
		return l->second;
	}

	// When combining with a serial operation, redo the linage as given.
	string CombineLinage(const vector<CalibrationAnalysis> &ana, LinageCombinationOperator how)
	{
		string result;
		string op(getOperator(how));
		for (vector<CalibrationAnalysis>::const_iterator itr = ana.begin(); itr != ana.end(); itr++) {
			if (result.size() > 0) {
				result += op;
			}
			result += Linage(*itr);
		}
		return result;
	}

	// For a binary operation, format a nice string.
	string BinaryLinageOp(const CalibrationAnalysis &a1, const string &a2, LinageBinaryOperator how)
	{
		ostringstream r;
		switch (how)
		{
		case BTagCombination::LBDStar:
			r << "D*(" << a2 << "=>" << Linage(a1) << ")";
			break;
		case BTagCombination::LBAddSys:
			r << "addSys(" << a2 << "=>" << Linage(a1) << ")";
			break;
		case BTagCombination::LBExtrapolate:
			r << "extrap(" << a2 << "=>" << Linage(a1) << ")";
			break;
		default:
			throw runtime_error("Unknown binary linage operator");
		}
		return r.str();
	}

	string BinaryLinageOp(const CalibrationAnalysis &a1, const CalibrationAnalysis &a2, LinageBinaryOperator how)
	{
		return BinaryLinageOp(a1, Linage(a2), how);
	}
}
