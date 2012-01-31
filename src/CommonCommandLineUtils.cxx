//
// Some common command line tools to be used when parsing arguments to make life easy
//

#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Parser.h"

#include <TSystem.h>

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
  // Load operating poitns from a text file on disk.
  void loadOPsFromFile(vector<CalibrationAnalysis> &list, const string &fname)
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
      vector<CalibrationAnalysis> calib = Parse(input);
      input.close();
      list.insert(list.end(), calib.begin(), calib.end());
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
}

//
// Now the main code to implement the command line support.
//
namespace BTagCombination {

  //
  // Parse a set of input arguments
  //
  void ParseOPInputArgs (const char **argv, int argc,
			 vector<CalibrationAnalysis> &operatingPoints,
			 vector<string> &unknownFlags)
  {
    //
    // Reset the intputs
    //

    operatingPoints.clear();
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
	  } else {
	    unknownFlags.push_back(flag);
	  }

	} else {
	  loadOPsFromFile(operatingPoints, a);
	}
      }
    }

    // If anything is to be ignored, better do that now.
    // Make sure to remove analyses that have nothing in them in the end...
    for (unsigned int i = 0; i < OPsToIgnore.size(); i++) {
      for (unsigned int op = 0; op < operatingPoints.size(); op++) {
	for (unsigned int b = 0; b < operatingPoints[op].bins.size(); b++) {
	  if (OPsToIgnore[i] == OPIgnoreFormat(operatingPoints[op], operatingPoints[op].bins[b])) {
	    operatingPoints[op].bins.erase(operatingPoints[op].bins.begin() + b);
	    break;
	  }
	}
      }
    }

    for (size_t op = operatingPoints.size(); op > size_t(0); op--) {
      if (operatingPoints[op-1].bins.size() == 0) {
	operatingPoints.erase(operatingPoints.begin() + (op-1));
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

  //
  // Split analyzes into lists. These lists are generally what we need when dealing
  // with the combination.
  //
  map<string, vector<CalibrationAnalysis> > BinAnalysesByJetTagFlavOp (const vector<CalibrationAnalysis> &anas)
  {
    map<string, vector<CalibrationAnalysis> > result;
    for (int i = 0; i < anas.size(); i++) {
      result[OPIndependentName(anas[i])].push_back(anas[i]);
    }
    return result;
  }

}
