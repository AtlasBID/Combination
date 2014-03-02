// Code to help with input and output of the calibration data model.

#include "Combination/CalibrationDataModel.h"

namespace BTagCombination {

  /////////////////
  // Stream operators
  /////////////////

  std::ostream &operator<< (std::ostream &out, const CalibrationBinBoundary &b);
  std::ostream &operator<< (std::ostream &out, const CalibrationBin &b);
  std::ostream &operator<< (std::ostream &out, const CalibrationAnalysis &ana);
  std::ostream &operator<< (std::ostream &out, const BinCorrelation &cor);
  std::ostream &operator<< (std::ostream &out, const AnalysisCorrelation &cor);
  std::ostream &operator<< (std::ostream &out, const DefaultAnalysis &d);
  std::ostream &operator<< (std::ostream &out, const AliasAnalysisCopyTo &info);
  std::ostream &operator<< (std::ostream &out, const AliasAnalysis &info);
  std::ostream &operator<< (std::ostream &out, const CalibrationInfo &info);

  //////////////
  // stream formatting modifiers
  /////////////

  // What format shoudl the bin boundary bin in?
  class CalibrationBinBoundaryFormat {
  public:
    inline CalibrationBinBoundaryFormat (CalibrationBinBoundary::BinBoundaryFormatEnum how)
      : _how (how)
      {}

    CalibrationBinBoundary::BinBoundaryFormatEnum _how;
  };
  std::ostream &operator<< (std::ostream &out, const CalibrationBinBoundaryFormat &f);

  // What should be bin format look like?
  class CalibrationBinFormat {
  public:
    inline CalibrationBinFormat (unsigned int what)
      : _what(what)
      {
      }
      unsigned int _what;
  };
  std::ostream &operator<< (std::ostream &out, const CalibrationBinFormat &f);
}
