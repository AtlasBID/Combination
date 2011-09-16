///
/// Paser.h
///
///  Utility code to turn a text file into a set of
/// data structures with b-tagging calibration results
///

#ifndef COMBINATION_PARSER
#define COMBINATION_PARSER

#include <string>
#include <vector>
#include <ostream>

namespace BTagCombination
{

  ///////
  // The datastructures that comes back that describes everything. Of course, the last thing
  // is the actual container.
  //


  // The binning boundaries
  struct CalibrationBinBoundary
  {
	double lowvalue;
	std::string variable;
	double highvalue;
  };

 inline std::ostream &operator<< (std::ostream &out, const CalibrationBinBoundary &b)
	{
	  out << b.lowvalue << " < " << b.variable << " < " << b.highvalue;
	  return out;
	}

//
// Systematic error. Always stored as an absolute error.
//
  struct SystematicError
{
	std::string name;
	double value;
};

  // Data for a single bin (central value, stat error, sys errors, etc.)
  struct CalibrationBin
  {
	  // What the bin boundary is
	std::vector<CalibrationBinBoundary> binSpec;

	// The central value and its error
	double centralValue;
	double centralValueStatisticalError;

	// Systematic Errors
	std::vector<SystematicError> systematicErrors;	
  };

 inline std::ostream &operator<< (std::ostream &out, const CalibrationBin &b)
	{
	  out << "bin{";
	  for (unsigned int i = 0; i < b.binSpec.size(); i++) {
	out << b.binSpec[i] << ",";
	  }
	  out << "}";

	  return out;
	}


  // Allowed flavors of calibration results
  enum Flavor {FBottom, FCharm, FLight};

  struct CalibrationAnalysis
  {
	std::string name; // The name of the analyiss, like "system8"
	//Flavor flavor; // What is this calibration for?
	std::string flavor; // What is this calibration for?
	std::string operatingPoint; // SV050 or similar - the operating point

	std::vector<CalibrationBin> bins; // List of bins with the actual 
  };

  //////

  // Returns a list of analyses given an input string.
  std::vector<CalibrationAnalysis> Parse(const std::string &inputText);
}

#endif
