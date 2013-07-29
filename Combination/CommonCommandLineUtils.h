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

  // Returns a common format name for teh analysis. Can be used in suppression
  // files, etc.
  std::string OPFullName (const CalibrationAnalysis &ana);
  std::string OPFullName (const AnalysisCorrelation &ana);
  std::string OPFullName (const DefaultAnalysis &ana);
  std::string OPFullName (const AliasAnalysis &ana);
  std::string OPBinName (const CalibrationBin &bin);
  std::string OPBinName (const std::vector<CalibrationBinBoundary> &binspec);
  std::string OPBinName (const std::set<CalibrationBinBoundary> &binspec);
  std::string OPBinName (const BinCorrelation &bin);
  std::string OPIgnoreFormat(const CalibrationAnalysis &ana, const CalibrationBin &bin);
  std::string OPIgnoreFormat(const AnalysisCorrelation &ana, const BinCorrelation &bin);
  std::string OPComputerFormat(const CalibrationAnalysis &ana, const CalibrationBin &bin);

  // Returns the OPIgnoreName for the two analyses involved in this guy.
  std::pair<std::string, std::string> OPIgnoreCorrelatedFormat (const AnalysisCorrelation &ana,
								const BinCorrelation &bin);

  // Returns a name that is how we partition everything (flavor, tagger, jet, op, etc.).
  std::string OPIndependentName (const CalibrationAnalysis &ana);

  // Returns a name that uses calib for partition everything
  std::string OPByCalibName (const CalibrationAnalysis &ana);
  // Returns a name that uses calib for partition everything - use the eff rather than the OP
  std::string OPByCalibAndEffName (const CalibrationAnalysis &ana);

  // Split a list of analyses by the bins we often use for doing the combination.
  // Useful utility. :-)
  std::map<std::string, std::vector<CalibrationAnalysis> > BinAnalysesByJetTagFlavOp (const std::vector<CalibrationAnalysis> &anas);
}

#endif
