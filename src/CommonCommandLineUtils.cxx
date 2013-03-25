//
// Some common command line tools to be used when parsing arguments to make life easy
//

#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Parser.h"

#include <TSystem.h>

#include <boost/regex.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>

using namespace std;

//
// First, some utility routines that will help us keep the important code
// below simple.
//
namespace {
  using namespace BTagCombination;

  void Combine(vector<CalibrationAnalysis> &master, const vector<CalibrationAnalysis> &newstuff)
  {
    for (vector<CalibrationAnalysis>::const_iterator itr = newstuff.begin(); itr != newstuff.end(); itr++) {
      bool inserted = false;
      for (vector<CalibrationAnalysis>::iterator mitr = master.begin(); mitr != master.end(); mitr++) {
	if (itr->name == mitr->name
	    && itr->flavor == mitr->flavor
	    && itr->tagger == mitr->tagger
	    && itr->operatingPoint == mitr->operatingPoint
	    && itr->jetAlgorithm == mitr->jetAlgorithm) {
	  // Should make sure there are no collisions!
	  for (vector<CalibrationBin>::const_iterator bitr = itr->bins.begin(); bitr != itr->bins.end(); bitr++) {
	    mitr->bins.push_back(*bitr);
	  }
	  inserted = true;
	  break;
	}
      }
      if (!inserted) {
	master.push_back(*itr);
      }
    }
  }

  // Load operating poitns from a text file on disk.
  void loadOPsFromFile(CalibrationInfo &list, const string &fname)
  {
    // See if the file exists - bomb if not!
    if (gSystem->AccessPathName(fname.c_str(), kFileExists)) {
      ostringstream msg;
      msg << "Unable to operating points file find file '" << fname << "'.";
      throw runtime_error(msg.str().c_str());
    }

    // Load it up!
    try {
      ifstream input(fname.c_str());
      CalibrationInfo calib = Parse(input);
      input.close();
      Combine(list.Analyses, calib.Analyses);
      list.Correlations.insert(list.Correlations.end(), calib.Correlations.begin(), calib.Correlations.end());
      list.Defaults.insert(list.Defaults.begin(), calib.Defaults.begin(), calib.Defaults.end());
      list.Aliases.insert(list.Aliases.begin(), calib.Aliases.begin(), calib.Aliases.end());
    } catch (exception &e) {
      ostringstream msg;
      msg << "Caught error parsing file '" << fname << "': " << e.what();
      throw runtime_error(msg.str().c_str());
    }
  }

  // The file contains a list of items to ignore, one per line.
  vector<string> loadIgnoreFile (string fname)
  {
    vector<string> result;
    ifstream input (fname.c_str());
    if (!input.is_open()) {
      throw runtime_error ("Unable to open --ignore file '" + fname + "'");
    }
    
    while (!input.eof()) {
      string line;
      getline(input, line);
      result.push_back(line);
    }
    return result;
  }

  // Return true if the list is empty or the match is in the lsit.
  bool CheckInList (const vector<string> &l, const string &match)
  {
    if (l.size() == 0)
      return true;
    for (vector<string>::const_iterator itr = l.begin(); itr != l.end(); itr++) {
      if (*itr == match)
	return true;
    }
    return false;
  }
}

//
// Now the main code to implement the command line support.
//
namespace BTagCombination {

  //
  // Parse a set of input arguments
  //
  void ParseOPInputArgs (const char **argv, int argc,
			 CalibrationInfo &operatingPoints,
			 vector<string> &unknownFlags)
  {
    //
    // Reset the intputs
    //

    operatingPoints.Analyses.clear();
    operatingPoints.Correlations.clear();
    operatingPoints.Defaults.clear();
    operatingPoints.Aliases.clear();
    operatingPoints.CombinationAnalysisName = "combined";

    unknownFlags.clear();

    //
    // Basic argument x-checks
    //

    if (argc == 0 || argv == 0)
      return;

    //
    // Start processing each argument
    //

    vector<string> OPsToIgnore;
    vector<string> spOnlyFlavor, spOnlyTagger, spOnlyOP, spOnlyJetAlgorithm, spOnlyAnalysis;

    for (int index = 0; index < argc; index++) {
      // is it a flag or a file containing operating points?
      string a (argv[index]);
      if (a.size() > 0) {
	if (a.substr(0, 2) == "--") {
	  string flag(a.substr(2));

	  if (flag == "ignore") {
	    if (index+1 == argc) {
	      throw runtime_error ("every --ignore must have an analysis name");
	    }
	    string ignore (argv[++index]);
	    if (ignore[0] != '@') {
	      OPsToIgnore.push_back(ignore);
	    } else {
	      vector<string> alltoignore (loadIgnoreFile(ignore.substr(1)));
	      OPsToIgnore.insert(OPsToIgnore.end(), alltoignore.begin(), alltoignore.end());
	    }
	  } else if (flag == "flavor") {
	    if (index+1 == argc) {
	      throw runtime_error ("every --flavor must have an analysis name");
	    }
	    index++;
	    spOnlyFlavor.push_back(argv[index]);
	  } else if (flag == "tagger") {
	    if (index+1 == argc) {
	      throw runtime_error ("every --tagger must have an analysis name");
	    }
	    index++;
	    spOnlyTagger.push_back(argv[index]);
	  } else if (flag == "operatingPoint") {
	    if (index+1 == argc) {
	      throw runtime_error ("every --operatingPoint must have an analysis name");
	    }
	    index++;
	    spOnlyOP.push_back(argv[index]);
	  } else if (flag == "jetAlgorithm") {
	    if (index+1 == argc) {
	      throw runtime_error ("every --jetAlgorithm must have an analysis name");
	    }
	    index++;
	    spOnlyJetAlgorithm.push_back(argv[index]);
	  } else if (flag == "analysis") {
	    if (index+1 == argc) {
	      throw runtime_error ("every --analysis must have an analysis name");
	    }
	    index++;
	    spOnlyAnalysis.push_back(argv[index]);
	  } else if (flag == "combinedName") {
	    if (index+1 == argc) {
	      throw runtime_error ("--combinedName must have a output combined name");
	    }
	    index++;
	    operatingPoints.CombinationAnalysisName = argv[index];
	  } else {
	    unknownFlags.push_back(flag);
	  }

	} else {
	  loadOPsFromFile(operatingPoints, a);
	}
      }
    }

    //
    // Process the copy commands... we will basically copy over anything listed
    //

    for (size_t i = 0; i < operatingPoints.Aliases.size(); i++) {
      const AliasAnalysis a(operatingPoints.Aliases[i]);
      vector<CalibrationAnalysis> copied;
      for (vector<CalibrationAnalysis>::const_iterator anaItr = operatingPoints.Analyses.begin(); anaItr != operatingPoints.Analyses.end(); anaItr++) {
	if (OPFullName(*anaItr) == OPFullName(a)) {
	  for (vector<AliasAnalysisCopyTo>::const_iterator cItr = a.CopyTargets.begin(); cItr != a.CopyTargets.end(); cItr++) {
	    CalibrationAnalysis cp (*anaItr);
	    cp.name = cItr->name;
	    cp.flavor = cItr->flavor;
	    cp.tagger = cItr->tagger;
	    cp.operatingPoint = cItr->operatingPoint;
	    cp.jetAlgorithm = cItr->jetAlgorithm;
	    copied.push_back(cp);
	  }
	}
      }
      operatingPoints.Analyses.insert(operatingPoints.Analyses.begin(), copied.begin(), copied.end());
    }
    
    operatingPoints.Aliases.clear();

    //
    // Process the main ignore lists - anything fully matching these are
    // dropped.
    //

    for (unsigned int i = 0; i < OPsToIgnore.size(); i++) {
      const boost::regex rIgnore (OPsToIgnore[i]);
      vector<CalibrationAnalysis> &ops(operatingPoints.Analyses);
      for (unsigned int op = 0; op < ops.size(); op++) {
	for (unsigned int b = 0; b < ops[op].bins.size(); b++) {
	  if (regex_match(OPIgnoreFormat(ops[op], ops[op].bins[b]), rIgnore)) {
	    ops[op].bins.erase(ops[op].bins.begin() + b);
	    b = b - 1;
	  }
	}
      }

      vector<AnalysisCorrelation> &cors(operatingPoints.Correlations);
      for (unsigned int ic = 0; ic < cors.size(); ic++) {
	for (unsigned int b = 0; b < cors[ic].bins.size(); b++) {
	  if (regex_match(OPIgnoreFormat(cors[ic], cors[ic].bins[b]), rIgnore)) {
	    cors[ic].bins.erase(cors[ic].bins.begin() + b);
	    b = b - 1;
	  }
	}
      }
    }

    //
    // Next process the "only" lists. If the only list is empty, then treat that
    // as a wild-card. Otherwise, demand an exact match.
    //

    for (size_t op = operatingPoints.Analyses.size(); op > size_t(0); op--) {
      if (!CheckInList(spOnlyFlavor, operatingPoints.Analyses[op-1].flavor)
	  || !CheckInList(spOnlyAnalysis, operatingPoints.Analyses[op-1].name)
	  || !CheckInList(spOnlyTagger, operatingPoints.Analyses[op-1].tagger)
	  || !CheckInList(spOnlyOP, operatingPoints.Analyses[op-1].operatingPoint)
	  || !CheckInList(spOnlyJetAlgorithm, operatingPoints.Analyses[op-1].jetAlgorithm)) {
	operatingPoints.Analyses.erase(operatingPoints.Analyses.begin() + (op-1));
      }
    }    

    for (size_t op = operatingPoints.Correlations.size(); op > size_t(0); op--) {
      if (!CheckInList(spOnlyFlavor, operatingPoints.Correlations[op-1].flavor)
	  || !CheckInList(spOnlyTagger, operatingPoints.Correlations[op-1].tagger)
	  || !CheckInList(spOnlyAnalysis, operatingPoints.Correlations[op-1].analysis1Name)
	  || !CheckInList(spOnlyAnalysis, operatingPoints.Correlations[op-1].analysis2Name)
	  || !CheckInList(spOnlyOP, operatingPoints.Correlations[op-1].operatingPoint)
	  || !CheckInList(spOnlyJetAlgorithm, operatingPoints.Correlations[op-1].jetAlgorithm)) {
	operatingPoints.Correlations.erase(operatingPoints.Correlations.begin() + (op-1));
      }
    }

    //
    // After that we may have some analyses which have zero bins in them. In that case
    // we want to eliminate them.
    //

    for (size_t op = operatingPoints.Analyses.size(); op > size_t(0); op--) {
      if (operatingPoints.Analyses[op-1].bins.size() == 0) {
	operatingPoints.Analyses.erase(operatingPoints.Analyses.begin() + (op-1));
      }
    }

    for (size_t cr = operatingPoints.Correlations.size(); cr > size_t(0); cr--) {
      if (operatingPoints.Correlations[cr-1].bins.size() == 0) {
	operatingPoints.Correlations.erase(operatingPoints.Correlations.begin() + (cr-1));
      }
    }
  }

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

  // return name of a bin (in a command-line friendly way)
  string OPBinName (const CalibrationBin &bin)
  {
    return OPBinName (bin.binSpec);
  }

  string OPBinName (const vector<CalibrationBinBoundary> &binspec)
  {
    ostringstream msg;
    for (unsigned int i = 0; i < binspec.size(); i++) {
      if (i > 0)
	msg << ":";
      msg << binspec[i].lowvalue
	  << "-" << binspec[i].variable
	  << "-" << binspec[i].highvalue;
    }
    return msg.str();
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
    ostringstream msg;
    for (unsigned int i = 0; i < bin.binSpec.size(); i++) {
      if (i > 0)
	msg << ":";
      msg << bin.binSpec[i].lowvalue
	  << "-" << bin.binSpec[i].variable
	  << "-" << bin.binSpec[i].highvalue;
    }
    return msg.str();
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

  //
  // Split analyzes into lists. These lists are generally what we need when dealing
  // with the combination.
  //
  map<string, vector<CalibrationAnalysis> > BinAnalysesByJetTagFlavOp (const vector<CalibrationAnalysis> &anas)
  {
    map<string, vector<CalibrationAnalysis> > result;
    for (size_t i = 0; i < anas.size(); i++) {
      result[OPIndependentName(anas[i])].push_back(anas[i]);
    }
    return result;
  }

}
