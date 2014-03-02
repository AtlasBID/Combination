// Contains the data model structs for the calibration stuff

#ifndef __CalibrationDataModel__
#define __CalibrationDataModel__

#include <string>
#include <vector>
#include <map>

namespace BTagCombination {

  // Helper
  bool doubleEqual (double p1, double p2);  

  // The binning boundaries
  struct CalibrationBinBoundary
  {
    double lowvalue;
    std::string variable;
    double highvalue;

    enum BinBoundaryFormatEnum { kNormal, kROOTFormatted };
    static BinBoundaryFormatEnum gFormatForNextBoundary;
  };

  //
  // Systematic error. Always stored as an absolute error.
  //
  struct SystematicError {
    std::string name;
    double value;
    bool uncorrelated;
    inline SystematicError()
      : value (0.0), uncorrelated (false)
    {}
  };

  // Data for a single bin (central value, stat error, sys errors, etc.)
  struct CalibrationBin
  {
    // What the bin boundary is
    std::vector<CalibrationBinBoundary> binSpec;

    // The central value and its error
    double centralValue;
    double centralValueStatisticalError;

    // Is it an extended errors bin
    bool isExtended;

    // A property bag of meta data that might or might not be there. We keep the name
    // a value, and a possible error (or set to zero).
    std::map<std::string, std::pair<double,double> > metadata;

    // Systematic Errors
    std::vector<SystematicError> systematicErrors;	
   
    // Helper for printing
    enum BinFormatEnum { kBinInfoOnly = 1,
			 kFullInfo = 2,
			 kROOTFormatted = 4};
    static unsigned int gForNextPrinting;

    // Default the values that don't initialize.
    CalibrationBin()
    : centralValue(0.0), centralValueStatisticalError(0.0), isExtended(false)
    {}
  };

  // Allowed flavors of calibration results
  enum Flavor {FBottom, FCharm, FLight};

  // The main calibration container
  struct CalibrationAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using

    std::vector<CalibrationBin> bins; // List of bins with the actual 

    std::map<std::string, std::vector<double> > metadata; // Meta data that we might pull along. Basically a property bag, with a list of numbers.
    std::map<std::string, std::string> metadata_s; // Meta data, strings.
  };

  // Contains the correlations for a single bin.
  struct BinCorrelation
  {
    std::vector<CalibrationBinBoundary> binSpec;

    bool hasStatCorrelation;
    double statCorrelation;

    BinCorrelation()
      : hasStatCorrelation(false), statCorrelation(0.0)
    {}
  };

  // The correlation between two analyses
  struct AnalysisCorrelation {
    std::string analysis1Name;
    std::string analysis2Name;
    std::string flavor;
    std::string tagger;
    std::string operatingPoint;
    std::string jetAlgorithm;

    std::vector<BinCorrelation> bins;
  };

  // Info on what analysis should be marked default
  struct DefaultAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using
  };

  struct AliasAnalysisCopyTo
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using
  };

  struct AliasAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using
    
    std::vector<AliasAnalysisCopyTo> CopyTargets;
  };

  // A list of everything that we might be reading in
  struct CalibrationInfo
  {
    std::vector<CalibrationAnalysis> Analyses;
    std::vector<AnalysisCorrelation> Correlations;
    std::vector<DefaultAnalysis> Defaults;
    std::vector<AliasAnalysis> Aliases;

    // The resulting combination should be stored as this analysis name.
    std::string CombinationAnalysisName;
    
    // What kind of binning boundaries do we need to respect? If true, then we
    // are doing bin-by-bin, otherwise, everythign in flavor/tag/OP/jet will
    // have to be respected.
    bool BinByBin;

    CalibrationInfo()
      : CombinationAnalysisName(""), BinByBin (false)
    {}
  };

  // Comparison operators of various types needed to put these guys in sets, etc.
  bool operator< (const CalibrationBinBoundary &x, const CalibrationBinBoundary &y);
  bool operator== (const CalibrationBinBoundary &x, const CalibrationBinBoundary &y);
  bool operator== (const SystematicError &e1, const SystematicError &e2);
  bool operator== (const CalibrationBin &b1, const CalibrationBin &b2);
  bool operator== (const CalibrationAnalysis &a1, const CalibrationAnalysis &a2);
}
#endif
