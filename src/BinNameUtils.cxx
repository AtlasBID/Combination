//
// Code to help with bin naming.
//

#include "Combination/BinNameUtils.h"

#include <string>
#include <sstream>

using namespace std;

namespace BTagCombination {

  // Returns a well formed name for the analysis. This is text only,
  // and is what a person can use to talk to us. :-)
  string OPFullName (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.name
	<< "-" << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint
	<< "-" << ana.jetAlgorithm;
    return msg.str();
  }

  string OPFullName (const DefaultAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.name
	<< "-" << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint
	<< "-" << ana.jetAlgorithm;
    return msg.str();
  }

  string OPFullName (const AliasAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.name
	<< "-" << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint
	<< "-" << ana.jetAlgorithm;
    return msg.str();
  }

  string OPFullName (const AnalysisCorrelation &ana)
  {
    ostringstream msg;
    msg << ana.analysis1Name
	<< "-" << ana.analysis2Name
	<< "-" << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint
	<< "-" << ana.jetAlgorithm;
    return msg.str();
  }

  string OPComputerName (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.name
	<< "//" << ana.flavor
	<< "//" << ana.tagger
	<< "//" << ana.operatingPoint
	<< "//" << ana.jetAlgorithm;
    return msg.str();
  }

  // Returns a well formed name for the analysis that ignores the actual
  // analysis name. This is text only,
  // and is what a person can use to talk to us. And can be used to sort things
  // by strings into how they should be combined.
  string OPIndependentName (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint
	<< "-" << ana.jetAlgorithm;
    return msg.str();
  }

  string OPByFlavorTaggerOp (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint;
    return msg.str();
  }

  string OPByCalibName (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.operatingPoint
	<< "-" << ana.name;
    return msg.str();
  }

  string OPByCalibJetTagger (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.flavor
	<< "-" << ana.tagger
	<< "-" << ana.name
	<< "-" << ana.jetAlgorithm;
    return msg.str();
  }

  // Return an eff string given an analysis for theOP
  string OPEff(const CalibrationAnalysis &ana)
  {
    if (ana.tagger == "MV1") {
      if (ana.operatingPoint == "0.992515446"
	  || ana.operatingPoint == "0.993981"
	  ) {
	return "50";
      } else if (ana.operatingPoint == "0.9867"
		 || ana.operatingPoint == "0.9827"
		 || ana.operatingPoint == "0.905363"
		 ) {
	return "60";
      } else if (ana.operatingPoint == "0.8119"
		 || ana.operatingPoint == "0.7892"
		 || ana.operatingPoint == "0.601713"
		 ) {
	return "70";
      } else if (ana.operatingPoint == "0.404219"
		 ) {
	return "75";
      } else if (ana.operatingPoint == "0.3900"
		 || ana.operatingPoint == "0.3511"
		 ) {
	return "80";
      } else if (ana.operatingPoint == "0.0714225"
		 || ana.operatingPoint == "0.1644"
		 || ana.operatingPoint == "0.1340"
		 ) {
	return "85";
      } else if (ana.operatingPoint == "0.0616"
		 || ana.operatingPoint == "0.0617"
		 ) {
	return "90";
      } else {
	return "MV1_unkown";
      }

    } else if (ana.tagger == "MV1c") {
      if (ana.operatingPoint == "0.9237"
	  || ana.operatingPoint == "0.9195") {
	return "50";
      } else if (ana.operatingPoint == "0.8674"
		 || ana.operatingPoint == "0.8641") {
	return "57";
      } else if (ana.operatingPoint == "0.8353"
		 || ana.operatingPoint == "0.8349") {
	return "60";
      } else if (ana.operatingPoint == "0.7028"
		 || ana.operatingPoint == "0.7068") {
	return "70";
      } else if (ana.operatingPoint == "0.4050"
		 || ana.operatingPoint == "0.4051") {
	return "80";
      } else {
	return "MV1c_unknown";
      }

	} else if (ana.tagger == "IP3DSV1") {
      if (ana.operatingPoint == "4.55") {
	return "60";
      } else if (ana.operatingPoint == "1.70") {
	return "70";
      } else if (ana.operatingPoint == "-0.80") {
	return "80";
      } else {
	return "IP3DSV1_unknown";
      }

    } else if (ana.tagger == "JetFitterCOMBNN") {
      if (ana.operatingPoint == "2.20") {
	return "57";
      } else if (ana.operatingPoint == "1.80") {
	return "60";
      } else if (ana.operatingPoint == "0.35") {
	return "70";
      } else if (ana.operatingPoint == "-1.25") {
	return "80";
      } else {
	return "JetFitterCOMBNN_unknown";
      }

    } else if (ana.tagger == "JetFitterCOMBNNc") {
      if (ana.operatingPoint == "1.33") {
        return "50";
      } else if (ana.operatingPoint == "0.98") {
        return "55";
      } else {
        return "JetFitterCOMBNNc_unknown";
      }

    } else if (ana.tagger == "JetFitterCharm") {
      if (ana.operatingPoint == "-0.9_NONE") {
	return "90";
      } else if (ana.operatingPoint == "-0.9_0.95") {
	return "20";
      } else if (ana.operatingPoint == "-0.9_2.5") {
	return "10";
      } else {
	return "JetFitterCOMBNNc_unknown";
      }

    } else if (ana.tagger == "SV0") {
      if (ana.operatingPoint == "5.65") {
	return "50";
      } else {
	return "SV0_unknown";
      }
    } else {
      return "tagger_unknown";
    }
  }

  string OPByCalibAndEffName (const CalibrationAnalysis &ana)
  {
    ostringstream msg;
    msg << ana.flavor
	<< "-" << ana.tagger
	<< "-" << OPEff(ana)
	<< "-" << ana.name;
    return msg.str();
  }

  // return name of a bin (in a command-line friendly way)
  string OPBinName (const CalibrationBin &bin)
  {
    return OPBinName (bin.binSpec);
  }

  string OPBinName (const vector<CalibrationBinBoundary> &binspec)
  {
    // Ordering is important, so we should have it all powered by something that sorts.
    return OPBinName(set<CalibrationBinBoundary>(binspec.begin(), binspec.end()));
  }

  string OPBinName (const set<CalibrationBinBoundary> &binspec)
  {
    ostringstream msg;
    bool first = true;
    for (set<CalibrationBinBoundary>::const_iterator i = binspec.begin(); i != binspec.end(); i++) {
      if (!first)
	msg << ":";
      first = false;

      msg << i->lowvalue
	  << "-" << i->variable
	  << "-" << i->highvalue;
    }
    return msg.str();
  }

  string OPBinName (const BinCorrelation &bin)
  {
    return OPBinName(bin.binSpec);
  }

  // The format of the name used in the ignore command line option
  string OPIgnoreFormat(const CalibrationAnalysis &ana, const CalibrationBin &bin)
  {
    return OPFullName(ana) + ":" + OPBinName(bin);
  }

  // The format that is pretty "easy" to parse by python or similar.
  string OPComputerFormat(const CalibrationAnalysis &ana, const CalibrationBin &bin)
  {
    return OPComputerName(ana) + "//" + OPBinName(bin);
  }

  // The format of the name used in the ignore command line option
  string OPIgnoreFormat(const AnalysisCorrelation &ana, const BinCorrelation &bin)
  {
    return OPFullName(ana) + ":" + OPBinName(bin);
  }

  // Build the ignore format for the two analyses associated with ana
  pair<string, string> OPIgnoreCorrelatedFormat(const AnalysisCorrelation &ana,
						const BinCorrelation &bin)
  {
    CalibrationAnalysis ca;
    ca.name = ana.analysis1Name;
    ca.flavor = ana.flavor;
    ca.tagger = ana.tagger;
    ca.operatingPoint = ana.operatingPoint;
    ca.jetAlgorithm = ana.jetAlgorithm;

    string n1 = OPFullName(ca) + ":" + OPBinName(bin);
    ca.name = ana.analysis2Name;
    string n2 = OPFullName(ca) + ":" + OPBinName(bin);
    return make_pair(n1, n2);
  }

}
