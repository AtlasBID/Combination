//
// This program will convert from an input calibration results file (in the standard text format)
// to a ROOT file for use by the CalibrationDataInterface.
//

#include "Combination/Parser.h"
#include "Combination/CDIConverter.h"
#include "Combination/CommonCommandLineUtils.h"

#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <set>

using namespace std;
using namespace BTagCombination;
using namespace Analysis;

void Usage (void);

TDirectory *get_sub_dir (TDirectory *parent, const string &name);
string convert_flavor (const string &flavor);

int main(int argc, char **argv)
{
  //
  // Parse input arguments
  //

  if (argc <= 1) {
    Usage();
    return 1;
  }

  CalibrationInfo info;
  bool updateROOTFile = false;
  try {
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);
    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "update") {
	updateROOTFile = true;
      } else {
	cerr << "Unknown command line flag '" << otherFlags[i] << "'." << endl;
	Usage();
	return 1;
      }
    }
  } catch (exception &e) {
    cerr << "Error parsing input files: " << e.what() << endl;
    return 1;
  }

  //
  // Convert it to an output root file. The directory structure is pretty
  // specific!
  //

  TH1::AddDirectory(false);
  TFile *output;
  string outputROOTName ("output.root");
  if (!updateROOTFile) {
    output = TFile::Open(outputROOTName.c_str(), "RECREATE");
  } else {
    output = TFile::Open(outputROOTName.c_str(), "UPDATE");
  }
  if (!output->IsOpen()) {
    cerr << "Unable to open 'output.root' for output!" << endl;
    return 1;
  }

  //
  // Next, index the defaults so we can quickly decide who is a default
  // analysis and who isn't.
  //

  set<string> defaultAnalysisNames;
  for (vector<DefaultAnalysis>::const_iterator itr = info.Defaults.begin(); itr != info.Defaults.end(); itr++) {
    defaultAnalysisNames.insert(OPFullName(*itr));
  }

  //
  // Now, write out everything. If the analysis is a default re-run the conversion
  // (to make sure that we are not doing somethign funny in ROOT).
  //

  const vector<CalibrationAnalysis> calib (info.Analyses);
  for (unsigned int i = 0; i < calib.size(); i++) {
    const CalibrationAnalysis &c(calib[i]);

    string name = c.name + "_SF";
    CalibrationDataContainer *container = ConvertToCDI (c, name);

    TDirectory *loc = get_sub_dir(output, c.tagger);
    loc = get_sub_dir(loc, c.jetAlgorithm);
    loc = get_sub_dir(loc, c.operatingPoint);
    loc = get_sub_dir(loc, convert_flavor(c.flavor));

    loc->WriteTObject(container);


    if (defaultAnalysisNames.find(OPFullName(c)) != defaultAnalysisNames.end()) {
      CalibrationDataContainer *def_c = ConvertToCDI (c, "default_SF");
      loc->WriteTObject(def_c);
    }
  }

  output->Close();
  delete output;

  return 0;
}

void Usage (void)
{
  cout << "Convert a text file to a CalibrationDataInterface ROOT file" << endl;
  cout << "FTConvertToCDI <input-filenames> <options>" << endl;
  cout << "  --ignore <item> - use to ignore a particular bin in the input" << endl;
  cout << "  --update <rootfname> - use to update the root file" << endl;
}

//
// Convert from the flavor used in our input text files to the one
// that is used by the calibration data interface
//

string convert_flavor (const string &flavor)
{
  if (flavor == "bottom" || flavor == "B" || flavor == "b")
    return "B";
  if (flavor == "charm" || flavor == "C" || flavor == "c")
    return "C";
  if (flavor == "light" || flavor == "L" || flavor == "l")
    return "Light";
  if (flavor == "tau" || flavor == "T" || flavor == 't')
    return "Tau";

  throw runtime_error (("Do not know flavor '" + flavor + "' - please use 'bottom', 'charm', or 'light' in the input text file!").c_str());
}

//
// Create a sub-directory in the given parent directory. If it is already
// there then return it. Sanitize the directory name.
TDirectory *get_sub_dir (TDirectory *parent, const string &name)
{
  //
  // Sanitize the name of the sub directory
  //

  string sname (name);
  replace (sname.begin(), sname.end(), '.', '_');

  //
  // If the sub dir is already there...
  //

  TDirectory *candidate = static_cast<TDirectory*>(parent->Get(sname.c_str()));
  if (candidate != 0)
    return candidate;

  //
  // Create it.
  //

  return parent->mkdir(sname.c_str());
}
