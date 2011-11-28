///
/// The context for a combinatoin of several different measurements of the same thing (or things),
/// along with possibly shared systematic errors.
///
#ifndef COMBINATION_CombinationContext
#define COMBINATION_CombinationContext

//#include "RooRealVarCache.h"

#include <string>
#include <vector>
#include <map>

class Measurement;
class RooRealVar;

class CombinationContext
{
public:
	/// Create/Destroy a new context. This will contain the common
	/// data to do a fit to mutliple measurements.
	CombinationContext(void);
	~CombinationContext(void);

	/// Create a new measurement. You are trying to measure "what", and with
	/// this particular measurement you found a value "value", and statistical
	/// error "statError". Min and Max values are the min and max values of 'what'.
	/// You can name your measurement or let the code choose a generic name for you.
	/// Names must be unique!
	Measurement *AddMeasurement (const std::string &measurementName, const std::string &what, const double minValue, const double maxValue,
			const double value, const double statError);
	Measurement *AddMeasurement (const std::string &what, const double minValue, const double maxValue,
			const double value, const double statError);

	/// Fit all the measurements that we've asked for.
	void Fit(void);

	/// Get the fit value (the 'what' of the add measurement)
	RooRealVar GetFitValue(const std::string &what) const;
private:
	/// Keep track of all the measurements.
	//RooRealVarCache _whatMeasurements;

	/// Keep track fo all the systematic errors between the various measurements.
	//RooRealVarCache _systematicErrors;

	/// Keep a list of all measurements
	std::vector<Measurement*> _measurements;
};
#endif

