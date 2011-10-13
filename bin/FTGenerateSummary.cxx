//
// Simpmle app to read in a clibration file and then
// will generate some twiki tables and a few plots
// (in a root file) and their eps/png matching files.
//

#include "Combination/Parser.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <boost/spirit/version.hpp>

using namespace std;
using namespace BTagCombination;

string LoadFromFile (const string &fname);

int main()
{
  string filename = "input.txt";
  
  vector<CalibrationAnalysis> items = BTagCombination::Parse(LoadFromFile(filename));

  cout << "See " << items.size() << " analyses" << endl;
  for (unsigned int iana = 0; iana < items.size(); iana++) {
    const CalibrationAnalysis &ana (items[iana]);
    cout << "Analysis " << ana.name
	 << ", " << ana.flavor
	 << ", " << ana.operatingPoint
	 << endl;

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
      const CalibrationBin &bin(ana.bins[ibin]);
      cout << "  Bin with CV " << bin.centralValue << " +- " << bin.centralValueStatisticalError << endl;

      for(unsigned int isys = 0; isys < bin.systematicErrors.size(); isys++) {
	const SystematicError &sys(bin.systematicErrors[isys]);
	cout << "    Sys " << sys.name << " " << sys.value << endl;
      }
    }
  }


  return 0;
}

//
// Load a file into memory
//
string LoadFromFile (const string &fname)
{
  ifstream input(fname.c_str());
  ostringstream result;

  while (!input.eof())
    {
      string line;
      getline(input, line);
      result << line << endl;
    }

  return result.str();
}
