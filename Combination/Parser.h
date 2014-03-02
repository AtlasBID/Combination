///
/// Paser.h
///
///  Utility code to turn a text file into a set of
/// data structures with b-tagging calibration results
///

#ifndef COMBINATION_PARSER
#define COMBINATION_PARSER

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <istream>
#include <iostream>
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "Combination/CalibrationDataModel.h"

namespace BTagCombination
{

  // Returns a list of analyses given an input string.
  CalibrationInfo Parse(const std::string &inputText);

  // Returns a list of analyses given an input text file (reads the complete text file)
  CalibrationInfo Parse(std::istream &input);
}

#endif
