//
// Functions and code to help with parsing command line arguments for the various tools
// that load up the text file input formats.
//

#ifndef __CommonCommandLineUtils_H__
#define __CommonCommandLineUtils_H__

#include "Combination/Parser.h"

#include <string>
#include <vector>

namespace BTagCombination {

  // Given the command line arguments, return a list of the
  // operating points and the flags that we didn't know how
  // to parse.
  void ParseOPInputArgs (const char **argv, int argc,
			 std::vector<CalibrationAnalysis> &operatingPoints,
			 std::vector<std::string> &unknownFlags);

}

#endif
