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

namespace BTagCombination
{

  ///////
  // The datastructures that comes back that describes everything. Of course, the last thing
  // is the actual container.
  //


  // Allowed flavors of calibration results
  enum Flavor {FBottom, FCharm, FLight};


  struct CalibrationAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    Flavor flavor; // What is this calibration for?
    std::string operatingPoint; // SV050 or similar - the operating point
  };

  //////

  // Returns a list of analyses given an input string.
  std::vector<CalibrationAnalysis> Parse(const std::string &inputText);
}

#endif
