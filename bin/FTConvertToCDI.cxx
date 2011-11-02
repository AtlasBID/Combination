//
// This program will convert from an input calibration results file (in the standard text format)
// to a ROOT file for use by the CalibrationDataInterface.
//

#include "Combination/Parser.h"
#include "Combination/CDIConverter.h"

#include <TFile.h>
#include <TDirectory.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>

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

  if (argc != 2) {
    Usage();
    return 1;
  }

  string input_file(argv[1]);

  //
  // Parse the input file to extract the calibration
  //

  ifstream input (input_file.c_str());
  if (!input.is_open()) {
    cout << "Unable to open file '" << input_file << "'." << endl;
    return 1;
  }
  vector<CalibrationAnalysis> calib = Parse (input);

  if (calib.size() == 0) {
    cerr << "Input file contains no analyses - so nothign will get copied over!" << endl;
    return 1;
  }

  //
  // Convert it to an output root file. The directory structure is pretty
  // specific!
  //

  TFile *output = TFile::Open("output.root", "RECREATE");

  for (unsigned int i = 0; i < calib.size(); i++) {
    const CalibrationAnalysis &c(calib[0]);

    string name = c.name + "_SF";
    CalibrationDataContainer *container = ConvertToCDI (c, name);

    TDirectory *loc = get_sub_dir(output, c.tagger);
    loc = get_sub_dir(loc, c.jetAlgorithm);
    loc = get_sub_dir(loc, c.operatingPoint);
    loc = get_sub_dir(loc, convert_flavor(c.flavor));

    loc->WriteTObject(container);
  }

  output->Close();
  delete output;

  return 0;
}

void Usage (void)
{
  cout << "Convert a text file to a CalibrationDataInterface ROOT file" << endl;
  cout << "FTConvertToCDI <input-filename>" << endl;
  cout << "  The output file will have the same name as the input file, with a file" << endl;
  cout << "  extension of .root." << endl;
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
