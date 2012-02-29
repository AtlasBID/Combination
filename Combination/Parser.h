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
#include <istream>
#include <iostream>

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

    enum BinBoundaryFormatEnum { kNormal, kROOTFormatted };
    static BinBoundaryFormatEnum gFormatForNextBoundary;
  };

  class CalibrationBinBoundaryFormat {
  public:
    inline CalibrationBinBoundaryFormat (CalibrationBinBoundary::BinBoundaryFormatEnum how)
      : _how (how)
      {}

    CalibrationBinBoundary::BinBoundaryFormatEnum _how;
  };

  inline bool operator< (const CalibrationBinBoundary &x,
			 const CalibrationBinBoundary &y)
    {
      if (x.variable != y.variable)
	return x.variable < y.variable;
      if (x.lowvalue != y.lowvalue)
	return x.lowvalue < y.lowvalue;
      return x.highvalue < y.highvalue;
    }

  inline std::ostream &operator<< (std::ostream &out, const CalibrationBinBoundaryFormat &f) {
    CalibrationBinBoundary::gFormatForNextBoundary = f._how;
    return out;
  }

  inline std::string formatForRoot (const std::string &input) {
    if (input == "pt")
      return "p_{T}";
    if (input == "eta")
      return "\\eta";
    if (input == "abseta")
      return "|\\eta|";
    return input;
  }

  inline std::ostream &operator<< (std::ostream &out, const CalibrationBinBoundary &b) {
    if (CalibrationBinBoundary::gFormatForNextBoundary == CalibrationBinBoundary::kNormal) {
      out << b.lowvalue << " < " << b.variable << " < " << b.highvalue;
    }
    else if (CalibrationBinBoundary::gFormatForNextBoundary == CalibrationBinBoundary::kROOTFormatted) {
      out << b.lowvalue << "<" << formatForRoot(b.variable) << "<" << b.highvalue;
    }
    CalibrationBinBoundary::gFormatForNextBoundary = CalibrationBinBoundary::kNormal;

    return out;
  }

  inline bool operator== (const CalibrationBinBoundary &x,
			  const CalibrationBinBoundary &y) {
    return x.variable == y.variable
      && x.lowvalue == y.lowvalue
      && x.highvalue == x.highvalue;
  }


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

	// Systematic Errors
	std::vector<SystematicError> systematicErrors;	
   
    // Helper for printing
    enum BinFormatEnum { kBinInfoOnly = 1,
			 kFullInfo = 2,
			 kROOTFormatted = 4};
    static unsigned int gForNextPrinting;


  };

  inline double relativeErrorCalc (double central, double err) {
    return err / central * 100.0;
  }

  class CalibrationBinFormat {
  public:
    inline CalibrationBinFormat (unsigned int what)
      : _what(what)
      {
      }
      unsigned int _what;
  };

  // Deal with teh modifier - we need to keep state, so we do it globally.
  inline std::ostream &operator<< (std::ostream &out, const CalibrationBinFormat &f) {
    CalibrationBin::gForNextPrinting = f._what;
    return out;
  }

  inline std::string quoteStringIf (const std::string &s) {
    if (s[0] == ' ' || s[s.size()-1] == ' ') {
      return "\"" + s + "\"";
    } else
      return s;
  }

  inline std::ostream &operator<< (std::ostream &out, const CalibrationBin &b) {
    if (CalibrationBin::gForNextPrinting & CalibrationBin::kFullInfo) {
      out << "bin(";
      for (unsigned int i = 0; i < b.binSpec.size(); i++) {
	if (i != 0)
	  out << ",";
	out << b.binSpec[i];
      }
      out << ") {" << std::endl;
      out << "    central_value (" << b.centralValue
	  << ", ";
      if (b.centralValue != 0.0) {
	out << relativeErrorCalc(b.centralValue, b.centralValueStatisticalError)
	    << "%)";
      } else {
	out << b.centralValueStatisticalError << ")";
      }
      
      for (size_t i = 0; i < b.systematicErrors.size(); i++) {
	out << std::endl
	  << "    ";
	if (b.systematicErrors[i].uncorrelated) {
	  out << "usys";
	} else {
	  out << "sys";
	}
	out << " (" << quoteStringIf(b.systematicErrors[i].name)
	    << ", " << relativeErrorCalc(b.centralValue, b.systematicErrors[i].value)
	    << "%)";
      }
      out << std::endl << "  }";

    } else if (CalibrationBin::gForNextPrinting & CalibrationBin::kBinInfoOnly) {
      for (unsigned int i = 0; i < b.binSpec.size(); i++) {
	if (i != 0)
	  out << ", ";
	if (CalibrationBin::gForNextPrinting | CalibrationBin::kROOTFormatted)
	  out << CalibrationBinBoundaryFormat(CalibrationBinBoundary::kROOTFormatted);
	out << b.binSpec[i];
      }
    }
    CalibrationBin::gForNextPrinting = CalibrationBin::kFullInfo;

    return out;
  }


  // Allowed flavors of calibration results
  enum Flavor {FBottom, FCharm, FLight};

  struct CalibrationAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using

    std::vector<CalibrationBin> bins; // List of bins with the actual 
  };

  //
  // Dump out a full analysis.
  //
  inline std::ostream &operator<< (std::ostream &out, const CalibrationAnalysis &ana) {
    out	<< "Analysis("
	<< ana.name
	<< ", " << ana.flavor
	<< ", " << ana.tagger
	<< ", " << ana.operatingPoint
	<< ", " << ana.jetAlgorithm
	<< ") {"
	<< std::endl;
    for (unsigned int i = 0; i < ana.bins.size(); i++)
      out << "  " << ana.bins[i] << std::endl;
    out << "}" << std::endl;
    return out;
  }

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

  inline std::ostream &operator<< (std::ostream &out, const BinCorrelation &cor) {
    out << "bin (";
    for (size_t i = 0; i < cor.binSpec.size(); i++) {
      if (i != 0)
	out << ",";
      out << cor.binSpec[i];
    }
    out << ") {" << std::endl;
    if (cor.hasStatCorrelation) {
      out << "  statistical(" << cor.statCorrelation << ")" << std::endl;
    }
    out << "}";
    return out;
  }

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

  inline std::ostream &operator<< (std::ostream &out, const AnalysisCorrelation &cor) {
    out << "Correlation (" 
	<< cor.analysis1Name
	<< ", " << cor.analysis2Name 
	<< ", " << cor.flavor
	<< ", " << cor.tagger
	<< ", " << cor.operatingPoint
	<< ", " << cor.jetAlgorithm
	<< ") {";
    for (size_t i = 0; i < cor.bins.size(); i++) {
      out << cor.bins[i] << std::endl;
    }
    out << "}";
    return out;
  }


  // Info on what analysis should be marked default
  struct DefaultAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using
  };

  inline std::ostream &operator<< (std::ostream &out, const DefaultAnalysis &d) {
    out << "Default(" 
	<< d.name 
	<< ", " << d.flavor
	<< ", " << d.tagger
	<< ", " << d.operatingPoint
	<< ", " << d.jetAlgorithm
	<< ")";
    return out;
  }  

  struct AliasAnalysisCopyTo
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using
  };

  inline std::ostream &operator<< (std::ostream &out, const AliasAnalysisCopyTo &info) {
    out << "Analysis("
	<< info.name
	<< ", " << info.flavor
	<< ", " << info.tagger
	<< ", " << info.operatingPoint
	<< ", " << info.jetAlgorithm
	<< ")";

    return out;
  }

  struct AliasAnalysis
  {
    std::string name; // The name of the analyiss, like "system8"
    std::string flavor; // What is this calibration for?
    
    std::string tagger; // What tagger are we running
    std::string operatingPoint; // SV050 or similar - the operating point
    std::string jetAlgorithm; // The Jet algorithm we are using
    
    std::vector<AliasAnalysisCopyTo> CopyTargets;
  };

  inline std::ostream &operator<< (std::ostream &out, const AliasAnalysis &info) {
    out << "Copy("
	<< info.name
	<< ", " << info.flavor
	<< ", " << info.tagger
	<< ", " << info.operatingPoint
	<< ", " << info.jetAlgorithm
	<< ") {" << std::endl;
    for (size_t i = 0; i < info.CopyTargets.size(); i++) {
      out << "  " << info.CopyTargets[i] << std::endl;
    }
    out << "}";

    return out;
  }

  // A list of everything that we might be reading in
  struct CalibrationInfo
  {
    std::vector<CalibrationAnalysis> Analyses;
    std::vector<AnalysisCorrelation> Correlations;
    std::vector<DefaultAnalysis> Defaults;
    std::vector<AliasAnalysis> Aliases;
  };

  inline std::ostream &operator<< (std::ostream &out, const CalibrationInfo &info) {
    for (size_t i = 0; i < info.Analyses.size(); i++) {
      out << info.Analyses[i] << std::endl;
    }
    for (size_t i = 0; i < info.Correlations.size(); i++) {
      out << info.Correlations[i] << std::endl;
    }
    for (size_t i = 0; i < info.Defaults.size(); i++) {
      out << info.Defaults[i] << std::endl;
    }
    for (size_t i = 0; i < info.Aliases.size(); i++) {
      out << info.Aliases[i] << std::endl;
    }
    return out;
  }

  //////

  // Returns a list of analyses given an input string.
  CalibrationInfo Parse(const std::string &inputText);

  // Returns a list of analyses given an input text file (reads the complete text file)
  CalibrationInfo Parse(std::istream &input);
}

#endif
