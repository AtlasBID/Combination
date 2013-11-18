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
#include <TKey.h>
#include <TClass.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <set>

using namespace std;
using namespace BTagCombination;
using namespace Analysis;

void Usage (void);

TDirectory *get_sub_dir (TDirectory *parent, const string &name, bool create = true);
string convert_flavor (const string &flavor);

namespace {
  string eatArg (char **argv, int &index, const int maxArg)
  {
    if (index == (maxArg-1)) 
      throw runtime_error ("Not enough arguments.");
    index++;
    return argv[index];
  }

  // Very simple wild-card matching
  bool wCompare (const string &s1, const string &s2)
  {
    if (s1 == "*" || s2 == "*")
      return true;
    return s1 == s2;
  }

  // Match with some basic wildcard info
  bool isAMatch (const vector<DefaultAnalysis> &defaults, const CalibrationAnalysis &ana)
  {
    for (unsigned int i = 0; i < defaults.size(); i++) {
      const DefaultAnalysis &d(defaults[i]);
      if (wCompare(ana.jetAlgorithm, d.jetAlgorithm)
	  && wCompare(ana.flavor, d.flavor)
	  && wCompare(ana.tagger, d.tagger)
	  && wCompare(ana.operatingPoint, d.operatingPoint)
	  && wCompare(ana.name, d.name)
	  )
	return true;
    }
    return false;
  }

  //
  // Sometimes we need to kludge a directry name translation in the CDI efficiency files
  // (we can't easily edit those - they are ROOT files, not text files). This provides for
  // a very simple extension point to enable that.
  //
  string TranslateDirectoryName(const string &sName)
  {
    if(sName == "1_01")
      return "1_00";
    return sName;
  }

  //
  // We are copying these objects, so we have to be able to reference them
  // to make sure they are linked in.
  //

  Analysis::CalibrationDataFunctionContainer *__c1;
  Analysis::CalibrationDataContainer *__c2;

  // Do a deep copy of the in directory into the out directory
  void copy_directory_structure (TDirectory *out, TDirectory *in, bool create = true)
  {
    //
    // Do a simple depth copy.
    //

    TClass *directory (TDirectory::Class());
    TIter next(in->GetListOfKeys());
    TKey *k;
    while ((k = (TKey*)next())) {
      if (TClass::GetClass(k->GetClassName())->InheritsFrom(directory)) {
	string outname(TranslateDirectoryName(k->GetName()));
	TDirectory *out_subdir = get_sub_dir(out, outname, create);
	if (out_subdir != 0) {
	  TDirectory *in_subdir = (TDirectory*) get_sub_dir(in, k->GetName());
	  copy_directory_structure(out_subdir, in_subdir, create);
	}
      } else {
	out->cd();
	TObject *o = k->ReadObj();
	out->WriteTObject(o, k->GetName(), "SingleKey");
      }
    }
  }
}

int main(int argc, char **argv)
{
  //
  // Parse input arguments
  //

  if (argc <= 1) {
    Usage();
    return 1;
  }

  vector<string> otherArgs;
    
  // Parse the input args for commands
  string outputFile ("output.root");
  map<string, string> configInfo;
  vector<string> taggers;

  for (int i = 1; i < argc; i++) {
    string a(argv[i]);
    if (a == "output") {
      outputFile = eatArg(argv, i, argc);
    } else if (a == "--config-info") {
      string k (eatArg(argv, i, argc));
      string v (eatArg(argv, i, argc));
      configInfo[k] = v;
    } else {
      otherArgs.push_back(a);
    }
  }

  CalibrationInfo info;
  bool updateROOTFile = false;
  bool restricted_mc_add = false;
  vector<string> other_mcfiles;
  try {
    vector<string> otherFlags;
    ParseOPInputArgs (otherArgs, info, otherFlags);
    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "update") {
	updateROOTFile = true;
      } else if (otherFlags[i].find("copy") == 0) {
	other_mcfiles.push_back(otherFlags[i].substr(4));
      } else if (otherFlags[i] == "restrictedMC") {
	restricted_mc_add = true;
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

  // If there are any config info guys that should be written out,
  // do that here

  if (configInfo.size() > 0) {
    TDirectory *d = output->mkdir("VersionInfo");
    for (map<string,string>::const_iterator itr = configInfo.begin(); itr != configInfo.end(); itr++) {
      TObjString *s = new TObjString(itr->second.c_str());
      d->WriteTObject(s, itr->first.c_str());
    }
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

    loc->WriteTObject(container, 0, "SingleKey");

    if (isAMatch(info.Defaults, c)) {
      CalibrationDataContainer *def_c = ConvertToCDI (c, "default_SF");
      loc->WriteTObject(def_c, 0, "SingleKey");
    }
  }

  //
  // The other mc files may need copying in...
  //

  for (unsigned int i = 0; i < other_mcfiles.size(); i++) {
    TFile *in = TFile::Open(other_mcfiles[i].c_str(), "READ");
    copy_directory_structure(output, in, !restricted_mc_add);
    in->Close();
    delete in;
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
  cout << "  --restrictedMC - only emit MC Eff plots for directories there is a actual SF in" << endl;
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
    return "T";

  throw runtime_error (("Do not know flavor '" + flavor + "' - please use 'bottom', 'charm', or 'light' in the input text file!").c_str());
}

//
// Create a sub-directory in the given parent directory. If it is already
// there then return it. Sanitize the directory name.
// If create is false and the directory doesn't exist, then return null.
TDirectory *get_sub_dir (TDirectory *parent, const string &name, bool create)
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
  // if we are not meant to do the creation...
  //

  if (!create)
    return 0;

  //
  // Create it.
  //

  return parent->mkdir(sname.c_str());
}
