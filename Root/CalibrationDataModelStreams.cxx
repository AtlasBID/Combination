// Code to output (correctly) the calibration data model.

#include "Combination/CalibrationDataModelStreams.h"
#include "Combination/BinNameUtils.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <stdexcept>

using namespace std;

// Older versions of VC don't have NaN quite the same way as the standard.
#ifdef _MSC_VER
#if (_MSC_VER <= 1800)
namespace std {
  inline double isnan(double a)
  {
    return _isnan(a);
  }
}
#endif
#endif

namespace {

  double relativeErrorCalc (double central, double err) {
    return err / central * 100.0;
  }

  string formatForRoot (const string &input) {
    if (input == "pt")
      return "p_{T}";
    if (input == "eta")
      return "\\eta";
    if (input == "abseta")
      return "|\\eta|";
    return input;
  }

  string quoteStringIf (const string &s) {
    if (s[0] == ' ' || s[s.size()-1] == ' ') {
      return "\"" + s + "\"";
    } else
      return s;
  }

}

namespace BTagCombination {

  ostream &operator<< (ostream &out, const CalibrationBinBoundaryFormat &f) {
    CalibrationBinBoundary::gFormatForNextBoundary = f._how;
    return out;
  }

  ostream &operator<< (ostream &out, const CalibrationBinBoundary &b) {
    if (CalibrationBinBoundary::gFormatForNextBoundary == CalibrationBinBoundary::kNormal) {
      out << b.lowvalue << " < " << b.variable << " < " << b.highvalue;
    }
    else if (CalibrationBinBoundary::gFormatForNextBoundary == CalibrationBinBoundary::kROOTFormatted) {
      out << b.lowvalue << "<" << formatForRoot(b.variable) << "<" << b.highvalue;
    }
    CalibrationBinBoundary::gFormatForNextBoundary = CalibrationBinBoundary::kNormal;

    return out;
  }

  // Deal with teh modifier - we need to keep state, so we do it globally.
  ostream &operator<< (ostream &out, const CalibrationBinFormat &f) {
    CalibrationBin::gForNextPrinting = f._what;
    return out;
  }

  ostream &operator<< (ostream &out, const CalibrationBin &b) {
    if (std::isnan(b.centralValue) || std::isnan(b.centralValueStatisticalError)) {
      ostringstream err;
      err << "Central value or stat error is NaN - can not write out bin" << endl
	  << "  Bin: " << OPBinName(b) << endl;
      throw runtime_error(err.str());
    }
    if (CalibrationBin::gForNextPrinting & CalibrationBin::kFullInfo) {
      if (b.isExtended) {
	out << "exbin(";
      } else {
	out << "bin(";
      }
      for (unsigned int i = 0; i < b.binSpec.size(); i++) {
	if (i != 0)
	  out << ",";
	out << b.binSpec[i];
      }
      out << ") {" << endl;

      out << "    central_value (" << b.centralValue
	  << ", ";
      if (b.centralValue != 0.0) {
	out << relativeErrorCalc(b.centralValue, b.centralValueStatisticalError)
	    << "%)";
      } else {
	out << b.centralValueStatisticalError << ")";
      }
      
      for (size_t i = 0; i < b.systematicErrors.size(); i++) {
	if (std::isnan(b.systematicErrors[i].value)) {
	  ostringstream err;
	  err << "Systematic Error is NaN - can not write out bin";
	  throw runtime_error(err.str());
	}
	out << endl
	  << "    ";
	if (b.systematicErrors[i].uncorrelated) {
	  out << "usys";
	} else {
	  out << "sys";
	}
	out << " (" << quoteStringIf(b.systematicErrors[i].name)
	    << ", " << (b.centralValue != 0 ? relativeErrorCalc(b.centralValue, b.systematicErrors[i].value) : b.systematicErrors[i].value)
	    << (b.centralValue == 0 ? "" : "%")
	    << ")";
      }

      for (map<string, pair<double,double> >::const_iterator itr = b.metadata.begin(); itr != b.metadata.end(); itr++) {
	out << endl;
	out << "    meta_data(" << itr->first << ", " << itr->second.first << ", " << itr->second.second << ")";
      }

      out << endl << "  }";

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


  //
  // Dump out a full analysis.
  //
  ostream &operator<< (ostream &out, const CalibrationAnalysis &ana) {
    out	<< "Analysis("
	<< ana.name
	<< ", " << ana.flavor
	<< ", " << ana.tagger
	<< ", " << ana.operatingPoint
	<< ", " << ana.jetAlgorithm
	<< ") {"
	<< endl;
    try {
      for (unsigned int i = 0; i < ana.bins.size(); i++)
	out << "  " << ana.bins[i] << endl;
      for (map<string, string>::const_iterator itr = ana.metadata_s.begin(); itr != ana.metadata_s.end(); itr++) {
	out << "  meta_data_s (" << itr->first << ", " << itr->second << ")" << endl;
      }
      for (map<string, vector<double> >::const_iterator itr = ana.metadata.begin(); itr != ana.metadata.end(); itr++) {
	string t (itr->first);
	if (t.find(',') != string::npos)
	  t = "\"" + t + "\"";
	out << "  meta_data (" << t;
	for (size_t i = 0; i < itr->second.size(); i++)
	  out << ", " << itr->second[i];
	out << ")" << endl;
      }
    } catch (runtime_error &e) {
      ostringstream err;
      err << e.what() << endl
	  << "  Analysis: " << OPFullName(ana) << endl;
      throw runtime_error(err.str());
    }
    out << "}" << endl;
    return out;
  }

  ostream &operator<< (ostream &out, const BinCorrelation &cor) {
    out << "bin (";
    for (size_t i = 0; i < cor.binSpec.size(); i++) {
      if (i != 0)
	out << ",";
      out << cor.binSpec[i];
    }
    out << ") {" << endl;
    if (cor.hasStatCorrelation) {
      out << "  statistical(" << cor.statCorrelation << ")" << endl;
    }
    out << "}";
    return out;
  }

  ostream &operator<< (ostream &out, const AnalysisCorrelation &cor) {
    out << "Correlation (" 
	<< cor.analysis1Name
	<< ", " << cor.analysis2Name 
	<< ", " << cor.flavor
	<< ", " << cor.tagger
	<< ", " << cor.operatingPoint
	<< ", " << cor.jetAlgorithm
	<< ") {";
    for (size_t i = 0; i < cor.bins.size(); i++) {
      out << cor.bins[i] << endl;
    }
    out << "}";
    return out;
  }

  ostream &operator<< (ostream &out, const DefaultAnalysis &d) {
    out << "Default(" 
	<< d.name 
	<< ", " << d.flavor
	<< ", " << d.tagger
	<< ", " << d.operatingPoint
	<< ", " << d.jetAlgorithm
	<< ")";
    return out;
  }  

  ostream &operator<< (ostream &out, const AliasAnalysisCopyTo &info) {
    out << "Analysis("
	<< info.name
	<< ", " << info.flavor
	<< ", " << info.tagger
	<< ", " << info.operatingPoint
	<< ", " << info.jetAlgorithm
	<< ")";

    return out;
  }

  ostream &operator<< (ostream &out, const AliasAnalysis &info) {
    out << "Copy("
	<< info.name
	<< ", " << info.flavor
	<< ", " << info.tagger
	<< ", " << info.operatingPoint
	<< ", " << info.jetAlgorithm
	<< ") {" << endl;
    for (size_t i = 0; i < info.CopyTargets.size(); i++) {
      out << "  " << info.CopyTargets[i] << endl;
    }
    out << "}";

    return out;
  }

  ostream &operator<< (ostream &out, const CalibrationInfo &info) {
    for (size_t i = 0; i < info.Analyses.size(); i++) {
      out << info.Analyses[i] << endl;
    }
    for (size_t i = 0; i < info.Correlations.size(); i++) {
      out << info.Correlations[i] << endl;
    }
    for (size_t i = 0; i < info.Defaults.size(); i++) {
      out << info.Defaults[i] << endl;
    }
    for (size_t i = 0; i < info.Aliases.size(); i++) {
      out << info.Aliases[i] << endl;
    }
    return out;
  }
}
