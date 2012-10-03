//
//  FTExploreFit
//
// Explore a fit - see if we can figure out why it behaves the way
// it does. Usually run on a bad fit.
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/Combiner.h"
#include "Combination/BinUtils.h"
#include "Combination/Plots.h"

#include <RooMsgService.h>
#include <TFile.h>
#include <TKey.h>

#include <algorithm>
#include <sstream>

using namespace std;
using namespace BTagCombination;

void usage(void);

namespace {

  // 
  // Dump plots into an output file.
  //
  void DumpPlotResults (TDirectory *outDir,
			const CalibrationInfo &info,
			const vector<CalibrationAnalysis> &r)
  {
    vector<CalibrationAnalysis> anas (info.Analyses);
    anas.insert(anas.end(), r.begin(), r.end());
    DumpPlots (outDir, anas);
  }

  string Normalize (const string &name)
  {
    string r (name);

    replace(r.begin(), r.end(), '.', '-');
    replace(r.begin(), r.end(), ':', '-');
    replace(r.begin(), r.end(), '/', '-');

    return r;
  }

  // Find a sub directory in a root directory. If it isn't there, then
  // create it.
  TDirectory *FindRootSubDir (TDirectory *dir, const string &subDirName)
  {
    TObject *o = dir->Get(subDirName.c_str());
    if (o != 0) {
      TDirectory *d = dynamic_cast<TDirectory*>(o);
      if (d != 0)
	return d;
    }

    return dir->mkdir(subDirName.c_str());
  }

  // Interface for a fit
  class FitTask {
  public:
    inline virtual ~FitTask (void) {}
    virtual string UserTitle () const = 0;

    virtual CalibrationInfo GetAnalyses (const CalibrationInfo &info) const = 0;

    virtual string StudyClassDirName() const = 0;
    virtual string StudyDirName() const = 0;
  };

  // Simple fit, do it unadulterated.
  class SimpleFit : public FitTask {
  public:
    SimpleFit (const string &name) : _name (name) {}
    string UserTitle () const { return _name + " fit"; }
    string StudyClassDirName (void) const { return ""; }
    string StudyDirName (void) const { return _name; }
    CalibrationInfo GetAnalyses (const CalibrationInfo &info) const { return info; }

  private:
    const string _name;
  };

  // Remove a list of bins from the fit.
  class RemoveBinFit : public FitTask {
  public:
    RemoveBinFit (int binsRemoved, const set<set<CalibrationBinBoundary> > &binsToRemove) : _remove (binsToRemove) {

      // Get the main directory name that will be used
      ostringstream msg;
      msg << "remove-" << binsRemoved << "-bins";
      _dirName = msg.str();

      // Next, the name of the study and the directory name for the study
      ostringstream studyName, studyDir;
      studyName << " with bins ";
      bool first = true;

      for (set<set<CalibrationBinBoundary> >::const_iterator itr = _remove.begin(); itr != _remove.end(); itr++) {
	if (!first) {
	  studyName << ", ";
	  studyDir << "-";
	}
	first = false;

	vector<CalibrationBinBoundary> temp (itr->begin(), itr->end());
	studyName << OPBinName(temp);
	studyDir << Normalize(OPBinName(temp));
      }
      studyName << " removed";
      _studyName = studyName.str();
      _studyDir = studyDir.str();
    }

    string UserTitle () const { return _studyName; }
    string StudyClassDirName (void) const { return _dirName; }
    string StudyDirName (void) const { return _studyDir; }

    CalibrationInfo GetAnalyses (const CalibrationInfo &info) const {
      CalibrationInfo missingInfo (info);
      for (set<set<CalibrationBinBoundary> >::const_iterator itr = _remove.begin(); itr != _remove.end(); itr++) {
	missingInfo.Analyses = removeBin (missingInfo.Analyses, *itr);
      }
      return missingInfo;
    }

  private:
    const set<set<CalibrationBinBoundary> > _remove;
    string _dirName;
    string _studyName;
    string _studyDir;
  };

  // We are given a set of bin names (or objects, whatever) - as long as they are listed in a
  // set. We will then will return a list of them to remove, numberToRemove at a time.
  template <typename ST>
  set<set<ST> > permutationsWithoutNItems (const set<ST> allBins, int numberToRemove)
  {
    // Recursion end case

    set<set<ST> > result;
    if (numberToRemove <= 0) {
      set<ST> dummy;
      result.insert(result.begin(), dummy);
    } else {
      
      // Remove one at a time, and recurse to get the next one.

      for (typename set<ST>::const_iterator itr = allBins.begin(); itr != allBins.end(); itr++) {
	set<ST> minusOne (allBins);
	minusOne.erase(*itr);

	set<set<ST> > subResult (permutationsWithoutNItems(minusOne, numberToRemove - 1));
	for(typename set<set<ST> >::iterator i_r = subResult.begin(); i_r != subResult.end(); i_r++) {
	  set<ST> sr (*i_r);
	  sr.insert(*itr);
	  result.insert(sr);
	}
      }
    }

    return result;
  }

}

int main (int argc, char **argv)
{
  // Parse input arguments
  CalibrationInfo allInfo;

  vector<int> removeBins;
  bool doRemoveSys = false;
  int nRemoveSys = 0;

  try {
    vector<string> otherFlags;
    ParseOPInputArgs ((const char**)&(argv[1]), argc-1, allInfo, otherFlags);

    for (vector<string>::const_iterator itr = otherFlags.begin(); itr != otherFlags.end(); itr++) {
      if (itr->find("remove-bin-") == 0) {
	istringstream buf (itr->substr(11).c_str());
	int r;
	buf >> r;
	removeBins.push_back(r);
      } else if (itr->find("remove-sys-") == 0) {
	doRemoveSys = true;
	istringstream buf (itr->substr(11).c_str());
	buf >> nRemoveSys;
      } else {
	cerr << "Unknown flag '" << *itr << "'" << endl;
	usage();
	return 1;
      }
    }

  } catch (exception &e) {
    cerr << "Error parsing command line: " << e.what() << endl;
    usage();
    return 1;
  }

  //
  // Simple setups
  //

  RooMsgService::instance().setSilentMode(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

  //
  // We will store the outputs in file.
  //

  TFile *outputPlots = TFile::Open("explore.root", "RECREATE");

  // What we do can only be done by looking at one fit at a time. So we
  // have to split everything by flavor, tagger, op, and jet algorithm.

  typedef map<string, vector<CalibrationAnalysis> > t_anaMap;
  t_anaMap binnedAnalyses (BinAnalysesByJetTagFlavOp(allInfo.Analyses));

  for(t_anaMap::const_iterator i_ana = binnedAnalyses.begin(); i_ana != binnedAnalyses.end(); i_ana++) {

    // Make sure this is worth our time...

    if (i_ana->second.size() == 1) {
      cout << "Can't explore fits that have only one input" << endl;
      continue;
    }

    // Put each set of analyses in a seperate guy.
    TDirectory *outDir = outputPlots->mkdir(Normalize(i_ana->first).c_str());

    // Next, build up a new Calibration Info as the central guy - this will have just one fit. This
    // will be the template we build all the other fits off of.

    CalibrationInfo centralInfo (allInfo);
    centralInfo.Analyses = i_ana->second;

    vector<FitTask*> fits;
    fits.push_back (new SimpleFit ("Default"));

    //
    // Next, if we are doing it by removing bins.
    //

    for (vector<int>::const_iterator i_bins = removeBins.begin(); i_bins != removeBins.end(); i_bins++) {
      
      // Get a list of all bins that we know about
      set<set<CalibrationBinBoundary> > allBins (listAllBins(centralInfo.Analyses));

      set<set<set<CalibrationBinBoundary> > > binPerm (permutationsWithoutNItems (allBins, *i_bins));
      for (set<set<set<CalibrationBinBoundary> > >::const_iterator i_p = binPerm.begin(); i_p != binPerm.end(); i_p++) {
	fits.push_back (new RemoveBinFit (*i_bins, *i_p));
      }
    }

    //
    // Now that all the fits are queued up, time to run them!
    //

    for (vector<FitTask*>::const_iterator itr = fits.begin(); itr != fits.end(); itr++) {
      const FitTask *fit (*itr);
      
      CalibrationInfo info (fit->GetAnalyses(centralInfo));

      cout << "Doing fit " << fit->UserTitle() << endl;
      vector<CalibrationAnalysis> result (CombineAnalyses(info, false));

      double chi2 = result[0].metadata["gchi2"]/result[0].metadata["gndof"];
      cout << "  chi2/ndof = " << chi2 << endl;

      // Analysis directory. There are two levels, one, what we are investigating,
      // and one if there is a name under that. If the investigation is empty, don't
      // do anything.

      string rootClassDir (fit->StudyClassDirName());
      TDirectory *outClassDir (outDir);
      if (rootClassDir.size() > 0)
	outClassDir = FindRootSubDir (outDir, rootClassDir);

      DumpPlotResults (outClassDir->mkdir(fit->StudyDirName().c_str()), info, result);
    }

    //cout << "Doing central fit..." << endl;
    // We use the global chi2/ndof as the figure of merit.
    // Sore the default in a single one.

#ifdef notyet
    // Now, if we've been asked to remove a bin at a time.
    if (doRemoveBin) {

      // Get a list of the bins in these analyses.

      // Now, remove the bins one at a time
      for (set<set<CalibrationBinBoundary> >::const_iterator itr = allBins.begin(); itr != allBins.end(); itr++) {
	CalibrationInfo missingBinInfo (allInfo);
	missingBInfo.Analyses = removeBin (missingBinInfo.Analyses, *itr);
	vector<CalibrationBinBoundary> tempBinInfo (itr->begin(), itr->end());

	cout << "Doing fit without bin " << OPBinName(tempBinInfo) << endl;
	vector<CalibrationAnalysis> missingBinResult (CombineAnalyses(missingBinInfo, false));

	// Print out the chi2
	double missingChi2 = missingBinResult[0].metadata["gchi2"]/missingBinResult[0].metadata["gndof"];
	cout << "  Missing bin chi2/ndof = " << missingChi2 << endl;

	// Generate plots.
	DumpPlotResults (outDir->mkdir(Normalize(OPBinName(tempBinInfo)).c_str()), missingBinInfo, missingBinResult);
      }
    }
#endif

    // Now, do it by removing one sys error at a time.

    if (doRemoveSys) {

      set<string> allSysErrors (listAllSysErrors(centralInfo.Analyses));
      for (set<string>::const_iterator itr = allSysErrors.begin(); itr != allSysErrors.end(); itr++) {
	CalibrationInfo missingSysInfo (allInfo);
	missingSysInfo.Analyses = removeSysError (missingSysInfo.Analyses, *itr);

	cout << "Doing fit wihtout sys error " << *itr << endl;
	vector<CalibrationAnalysis> missingSysResult (CombineAnalyses(missingSysInfo, false));

	double missingChi2 = missingSysResult[0].metadata["gchi2"]/missingSysResult[0].metadata["gndof"];
	cout << "  Missing bin chi2/ndof = " << missingChi2 << endl;

	DumpPlotResults (outDir->mkdir(Normalize(*itr).c_str()), missingSysInfo, missingSysResult);
      }
    }
  }

  //
  // Make sure all plot get written out.
  //

  outputPlots->Write();
  outputPlots->Close();

  return 0;
}

void usage(void)
{
  cout << "FTExploreFit <std-cmd-line-argsw> --remove-bin-NN --remove-sys-NN" << endl;
  cout << "  NN is a number - how many to remove or run on each iteration" << endl;
}
