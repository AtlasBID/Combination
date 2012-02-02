//
// Simpmle app to read in a clibration file and then
// will generate some twiki tables and a few plots
// (in a root file) and their eps/png matching files.
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <boost/spirit/version.hpp>

using namespace std;
using namespace BTagCombination;

int main(int argc, char **argv)
{
  // Parse the input arguments
  CalibrationInfo info;
  vector<string> otherFlags;
  ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);

  const vector<CalibrationAnalysis> &items(info.Analyses);
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

