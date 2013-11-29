//
// Simpmle app to read in a clibration file and then
// will generate some twiki tables and a few plots
// (in a root file) and their eps/png matching files.
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>

#include <boost/spirit/version.hpp>

using namespace std;
using namespace BTagCombination;

void Usage(void);
void PrintList(const CalibrationInfo &info);
void PrintTable(const CalibrationInfo &info);

int main(int argc, char **argv)
{
  if (argc <= 2) {
    Usage();
    return 1;
  }

  // Parse the input arguments
  CalibrationInfo info;
  vector<string> otherFlags;
  ParseOPInputArgs ((const char**)&(argv[1]), argc-1, info, otherFlags);

  for (unsigned int i = 0; i < otherFlags.size(); i++) {
    if (otherFlags[i] == "list")
      PrintList(info);
    else if (otherFlags[i] == "table")
      PrintTable(info);
  }
  
  return 0;
}

void PrintList(const CalibrationInfo &info)
{

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
}

void PrintTable(const CalibrationInfo &info)
{

  const vector<CalibrationAnalysis> &items(info.Analyses);
  for (unsigned int iana = 0; iana < items.size(); iana++){
    const CalibrationAnalysis &ana (items[iana]);
    cout << "Analysis " << ana.name << ", " << ana.flavor
	 << ", " << ana.operatingPoint << endl;
    
    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
      const CalibrationBin &bin(ana.bins[ibin]);
      string binname (OPBinName(bin));
      if (!ibin)
	cout << "\tBin " << binname << " & ";
      else if (ibin==ana.bins.size()-1)
	cout << binname << " \\" << "\\";
      else
	cout << binname << " & ";
    }
    cout << endl;

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
      const CalibrationBin &bin(ana.bins[ibin]);
      if (!ibin)
	cout << "\tCentral value & " << bin.centralValue << "+-" << bin.centralValueStatisticalError << " & ";
      else if (ibin==ana.bins.size()-1)
	cout << bin.centralValue << "+-" << bin.centralValueStatisticalError << " \\" << "\\";
      else
	cout << bin.centralValue << "+-" << bin.centralValueStatisticalError << " & ";
    }
    cout << endl;
    
    for (unsigned int i = 0; i < ana.bins[0].systematicErrors.size(); i++){
      for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
	const CalibrationBin &bin(ana.bins[ibin]);
	const SystematicError &sys(bin.systematicErrors[i]);
	if (!ibin)
	  cout << "\t" << sys.name << " & " << (sys.value/bin.centralValue*100) << "% & ";
	else if (ibin==ana.bins.size()-1)
	  cout << (sys.value/bin.centralValue*100) << "% \\" << "\\";
	else
	  cout << (sys.value/bin.centralValue*100) << "% & ";	
      }
      cout << endl;
    }
  }
}

void Usage(void)
{
  cout << "FTGenerateSummary <file-options>" << endl;
  cout << "  --list - list the breakdown of syst uncertainties bin by bin" << endl;
  cout << "  --table - print summary table in latex format for all syst uncertainties" << endl;
}

