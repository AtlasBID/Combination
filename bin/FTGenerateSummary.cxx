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
void PrintList(const CalibrationInfo &info, const float &prec);
void PrintTable(const CalibrationInfo &info, const float &prec);

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

  float prec=2;

  for (unsigned int i = 0; i < otherFlags.size(); i++) {
    if (otherFlags[i].substr(0,10) == "precision=") {
      istringstream is(otherFlags[i].substr(10,otherFlags[i].length()-10));
      is >> prec;
      break;
    }
  }

  for (unsigned int i = 0; i < otherFlags.size(); i++) {
    if (otherFlags[i] == "list")
      PrintList(info,prec);
    else if (otherFlags[i] == "table")
      PrintTable(info,prec);
  }
  
  return 0;
}

void PrintList(const CalibrationInfo &info, const float &prec)
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
      cout << setw(5) << setprecision(prec) << "  Bin with CV " << bin.centralValue << " +- " << bin.centralValueStatisticalError << endl;

      for(unsigned int isys = 0; isys < bin.systematicErrors.size(); isys++) {
	const SystematicError &sys(bin.systematicErrors[isys]);
	cout << "    Sys " << sys.name << " " << sys.value << endl;
      }
    }
  }
}

void PrintTable(const CalibrationInfo &info, const float &prec)
{

  const vector<CalibrationAnalysis> &items(info.Analyses);
  for (unsigned int iana = 0; iana < items.size(); iana++){
    const CalibrationAnalysis &ana (items[iana]);
    cout << "Analysis " << ana.name << ", " << ana.flavor
	 << ", " << ana.operatingPoint << endl;
    cout << endl; cout << "\\" << "hline" << endl;

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
      const CalibrationBin &bin(ana.bins[ibin]);
      string binname (OPBinName(bin));
      if (!ibin)
	cout << "Bin " << binname << " & ";
      else if (ibin==ana.bins.size()-1)
	cout << binname << " \\" << "\\";
      else
	cout << binname << " & ";
    }
    cout << endl; cout << "\\" << "hline" << endl;
    
    vector<double> totSyst;

    for (unsigned int i = 0; i < ana.bins[0].systematicErrors.size(); i++){
      for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
	const CalibrationBin &bin(ana.bins[ibin]);
	const SystematicError &sys(bin.systematicErrors[i]);
	if (!ibin)
	  cout << setw(5) << setprecision(prec) << sys.name << " & " << (sys.value/bin.centralValue*100) << "% & ";
	else if (ibin==ana.bins.size()-1)
	  cout << (sys.value/bin.centralValue*100) << "% \\" << "\\";
	else
	  cout << (sys.value/bin.centralValue*100) << "% & ";
	if (!i)
	  totSyst.push_back((sys.value/bin.centralValue*100)*(sys.value/bin.centralValue*100));
	else
	  totSyst[ibin]+=(sys.value/bin.centralValue*100)*(sys.value/bin.centralValue*100);
      }
      cout << endl; cout << "\\" << "hline" << endl;
    }

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
      if (!ibin)
	cout << "Total systematic & " << sqrt(totSyst[ibin]) << "% & ";
      else if (ibin==ana.bins.size()-1)
	cout << sqrt(totSyst[ibin]) << "% \\" << "\\";
      else 
	cout << sqrt(totSyst[ibin]) << "% & ";
    }
    cout << endl; cout << "\\" << "hline" << endl;

    for (unsigned int ibin = 0; ibin < ana.bins.size(); ibin++){
      const CalibrationBin &bin(ana.bins[ibin]);
      if (!ibin)
	cout << "Statistics & " << bin.centralValueStatisticalError*100 << "% & ";
      else if (ibin==ana.bins.size()-1)
	cout << bin.centralValueStatisticalError*100 << "% \\" << "\\";
      else
	cout << bin.centralValueStatisticalError*100 << "% & ";
    }
    cout << endl; cout << "\\" << "hline" << endl;
  }
}

void Usage(void)
{
  cout << "FTGenerateSummary <file-options>" << endl;
  cout << "  --list - list the breakdown of syst uncertainties bin by bin" << endl;
  cout << "  --table - print summary table in latex format for all syst uncertainties" << endl;
  cout << "  --precision - option for displaying the number of digits (default is 2)" << endl;
}

