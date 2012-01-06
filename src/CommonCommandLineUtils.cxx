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
    ifstream input(fname.c_str());
    vector<CalibrationAnalysis> calib = Parse(input);
    input.close();
    list.insert(list.end(), calib.begin(), calib.end());
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

    for (int index = 0; index < argc; index++) {
      // is it a flag or a file containing operating points?
      string a (argv[index]);
      if (a.size() > 0) {
	if (a.substr(0, 2) == "--") {
	  string flag(a.substr(2));
	  unknownFlags.push_back(flag);
	} else {
	  loadOPsFromFile(operatingPoints, a);
	}
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

  // return name of a bin
  string OPBinName (const CalibrationBin &bin)
  {
    ostringstream msg;
    for (unsigned int i = 0; i < bin.binSpec.size(); i++) {
      if (i > 0)
	msg << "|";
      msg << bin.binSpec[i].lowvalue
	  << "<" << bin.binSpec[i].variable
	  << "<" << bin.binSpec[i].highvalue;
    }
    return msg.str();
  }
}
