//
// Naming utils for turning bins into names
//
#ifndef __BinNameUtils__
#define __BinNameUtils__

#include "Combination/CalibrationDataModel.h"

#include <set>
#include <vector>

namespace BTagCombination {
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

  // Returns a name that uses flavor, tagger, op, but not jet.
  std::string OPByFlavorTaggerOp (const CalibrationAnalysis &ana);

  // Returns a name that uses calib for partition everything
  std::string OPByCalibName (const CalibrationAnalysis &ana);
  // Returns a name that uses calib for partition everything - use the eff rather than the OP
  std::string OPByCalibAndEffName (const CalibrationAnalysis &ana);
  // Returns a name that uses the calib, the jet, and the tagger for partitioning, so
  // only the eff isn't set here.
  std::string OPByCalibJetTagger (const CalibrationAnalysis &ana);
  // Convert some eff to strings
  std::string OPEff(const CalibrationAnalysis &ana);
}
#endif
