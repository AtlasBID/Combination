//
// Functions and code to help with parsing command line arguments for the various tools
// that load up the text file input formats.
//

#ifndef __CommonCommandLineUtils_H__
#define __CommonCommandLineUtils_H__

#include "Combination/Parser.h"

#include <string>
#include <vector>
#include <map>
#include <set>

namespace BTagCombination {

  // Given the command line arguments, return a list of the
  // operating points and the flags that we didn't know how
  // to parse.
  void ParseOPInputArgs (const char **argv, int argc,
			 CalibrationInfo &operatingPoints,
			 std::vector<std::string> &unknownFlags);

  void ParseOPInputArgs (const std::vector<std::string> &args,
			 CalibrationInfo &operatingPoints,
			 std::vector<std::string> &unknownFlags);

  // Split a list of analyses by the bins we often use for doing the combination.
  // Useful utility. :-)
  std::map<std::string, std::vector<CalibrationAnalysis> > BinAnalysesByJetTagFlavOp (const std::vector<CalibrationAnalysis> &anas);
}

#endif
