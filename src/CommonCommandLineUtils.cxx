//
// Some common command line tools to be used when parsing arguments to make life easy
//

#include "Combination/BinNameUtils.h"
#include "Combination/CalibrationDataModel.h"
#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/BinBoundaryUtils.h"

#include <TSystem.h>

#include <boost/regex.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <algorithm>

using namespace std;

//
// First, some utility routines that will help us keep the important code
// below simple.
//
namespace {
  using namespace BTagCombination;

  void Combine(vector<CalibrationAnalysis> &master, const vector<CalibrationAnalysis> &newstuff)
  {
    for (vector<CalibrationAnalysis>::const_iterator itr = newstuff.begin(); itr != newstuff.end(); itr++) {
      bool inserted = false;
      for (vector<CalibrationAnalysis>::iterator mitr = master.begin(); mitr != master.end(); mitr++) {
        if (itr->name == mitr->name
          && itr->flavor == mitr->flavor
          && itr->tagger == mitr->tagger
          && itr->operatingPoint == mitr->operatingPoint
          && itr->jetAlgorithm == mitr->jetAlgorithm) {
          // Should make sure there are no collisions!
          for (vector<CalibrationBin>::const_iterator bitr = itr->bins.begin(); bitr != itr->bins.end(); bitr++) {
            mitr->bins.push_back(*bitr);
          }
          inserted = true;
          break;
        }
      }
      if (!inserted) {
        master.push_back(*itr);
      }
    }
  }

  // Load operating points from a text file on disk.
  void loadOPsFromFile(CalibrationInfo &list, const string &fname, calibrationFilterInfo &fInfo)
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
      CalibrationInfo calib = Parse(input, fInfo);
      input.close();
      Combine(list.Analyses, calib.Analyses);
      list.Correlations.insert(list.Correlations.end(), calib.Correlations.begin(), calib.Correlations.end());
      list.Defaults.insert(list.Defaults.begin(), calib.Defaults.begin(), calib.Defaults.end());
      list.Aliases.insert(list.Aliases.begin(), calib.Aliases.begin(), calib.Aliases.end());
    }
    catch (exception &e) {
      ostringstream msg;
      msg << "Caught error parsing file '" << fname << "': " << e.what();
      throw runtime_error(msg.str().c_str());
    }
  }

  // The file contains a list of items to ignore, one per line.
  vector<string> loadIgnoreFile(string fname)
  {
    vector<string> result;
    ifstream input(fname.c_str());
    if (!input.is_open()) {
      throw runtime_error("Unable to open --ignore file '" + fname + "'");
    }

    while (!input.eof()) {
      string line;
      getline(input, line);
      result.push_back(line);
    }
    return result;
  }

  // Return true if the list is empty or the match is in the list.
  bool CheckInList(const set<const boost::regex> &l, const string &match)
  {
    if (l.size() == 0)
      return true;
    for (set<boost::regex>::const_iterator itr = l.begin(); itr != l.end(); itr++) {
      if (boost::regex_match(match, *itr))
        return true;
    }
    return false;
  }

  // Helper predicate to look for a name in a list.
  class SysErrorNameMatch {
  public:
    inline SysErrorNameMatch(const string &name)
      : _name(name) {}
    inline bool operator() (const SystematicError &test) {
      return test.name == _name;
    }
  private:
    const string &_name;
  };

  // Return a string with "'" stripped off.
  string cleanup(const string &inp)
  {
    string result(inp);
    if (result[0] == '\'') {
      result = result.substr(1);
    }
    if (result[result.size() - 1] == '\'') {
      result = result.substr(0, result.size() - 1);
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
  void ParseOPInputArgs(const char **argv, int argc,
    CalibrationInfo &operatingPoints,
    vector<string> &unknownFlags)
  {
    vector<string> args;
    for (int i = 0; i < argc; i++) {
      args.push_back(argv[i]);
    }
    ParseOPInputArgs(args, operatingPoints, unknownFlags);
  }

  // Clean out the incoming analysis according to spec.
  void FilterAnalyses(CalibrationInfo &operatingPoints, const calibrationFilterInfo &fInfo)
  {
    for (set<boost::regex>::const_iterator itr = fInfo.OPsToIgnore.begin(); itr != fInfo.OPsToIgnore.end(); itr++) {
      vector<CalibrationAnalysis> &ops(operatingPoints.Analyses);
      for (unsigned int op = 0; op < ops.size(); op++) {
        for (unsigned int b = 0; b < ops[op].bins.size(); b++) {
          if (regex_match(OPIgnoreFormat(ops[op], ops[op].bins[b]), *itr)) {
            ops[op].bins.erase(ops[op].bins.begin() + b);
            b = b - 1;
          }
        }
      }

      vector<AnalysisCorrelation> &cors(operatingPoints.Correlations);
      for (unsigned int ic = 0; ic < cors.size(); ic++) {
        for (unsigned int b = 0; b < cors[ic].bins.size(); b++) {
          if (regex_match(OPIgnoreFormat(cors[ic], cors[ic].bins[b]), *itr)) {
            cors[ic].bins.erase(cors[ic].bins.begin() + b);
            b = b - 1;
          }
        }
      }
    }

    //
    // Next process the "only" lists. If the only list is empty, then treat that
    // as a wild-card. Otherwise, demand an exact match.
    //

    for (size_t op = operatingPoints.Analyses.size(); op > size_t(0); op--) {
      if (!CheckInList(fInfo.spOnlyFlavor, operatingPoints.Analyses[op - 1].flavor)
        || !CheckInList(fInfo.spOnlyAnalysis, operatingPoints.Analyses[op - 1].name)
        || !CheckInList(fInfo.spOnlyTagger, operatingPoints.Analyses[op - 1].tagger)
        || !CheckInList(fInfo.spOnlyOP, operatingPoints.Analyses[op - 1].operatingPoint)
        || !CheckInList(fInfo.spOnlyJetAlgorithm, operatingPoints.Analyses[op - 1].jetAlgorithm)) {
        operatingPoints.Analyses.erase(operatingPoints.Analyses.begin() + (op - 1));
      }
    }

    for (size_t op = operatingPoints.Correlations.size(); op > size_t(0); op--) {
      if (!CheckInList(fInfo.spOnlyFlavor, operatingPoints.Correlations[op - 1].flavor)
        || !CheckInList(fInfo.spOnlyTagger, operatingPoints.Correlations[op - 1].tagger)
        || !CheckInList(fInfo.spOnlyAnalysis, operatingPoints.Correlations[op - 1].analysis1Name)
        || !CheckInList(fInfo.spOnlyAnalysis, operatingPoints.Correlations[op - 1].analysis2Name)
        || !CheckInList(fInfo.spOnlyOP, operatingPoints.Correlations[op - 1].operatingPoint)
        || !CheckInList(fInfo.spOnlyJetAlgorithm, operatingPoints.Correlations[op - 1].jetAlgorithm)) {
        operatingPoints.Correlations.erase(operatingPoints.Correlations.begin() + (op - 1));
      }
    }
  }

  //
  // Parse a set of input arguments
  //
  void ParseOPInputArgs(const vector<string> &args,
    CalibrationInfo &operatingPoints,
    vector<string> &unknownFlags)
  {
    //
    // Reset the inputs
    //

    operatingPoints.Analyses.clear();
    operatingPoints.Correlations.clear();
    operatingPoints.Defaults.clear();
    operatingPoints.Aliases.clear();
    operatingPoints.CombinationAnalysisName = "combined";

    unknownFlags.clear();

    //
    // Basic argument x-checks
    //

    if (args.size() == 0)
      return;

    //
    // Start processing each argument
    //

    calibrationFilterInfo fInfo;
    vector<string> ignoreSysError;
    vector<string> filesToLoad;

    for (size_t index = 0; index < args.size(); index++) {
      // is it a flag or a file containing operating points?
      string a(args[index]);
      if (a.size() > 0) {
        if (a.substr(0, 2) == "--") {
          string flag(a.substr(2));

          if (flag == "ignore") {
            if (index + 1 == args.size()) {
              throw runtime_error("every --ignore must have an analysis name");
            }
            string ignore(args[++index]);
            if (ignore[0] != '@') {
              fInfo.OPsToIgnore.insert(boost::regex(ignore));
            }
            else {
              vector<string> alltoignore(loadIgnoreFile(ignore.substr(1)));
              for (size_t idx = 0; idx < alltoignore.size(); idx++) {
                fInfo.OPsToIgnore.insert(boost::regex(alltoignore[idx]));
              }
            }
          }
          else if (flag == "ignoreSysError") {
            if (index + 1 == args.size())
              throw runtime_error("every --ignoreSysError must have a systematic error name");
            index++;
            ignoreSysError.push_back(args[index]);
          }
          else if (flag == "flavor") {
            if (index + 1 == args.size()) {
              throw runtime_error("every --flavor must have an analysis name");
            }
            index++;
            fInfo.spOnlyFlavor.insert(boost::regex(cleanup(args[index])));
          }
          else if (flag == "tagger") {
            if (index + 1 == args.size()) {
              throw runtime_error("every --tagger must have an analysis name");
            }
            index++;
            fInfo.spOnlyTagger.insert(boost::regex(cleanup(args[index])));
          }
          else if (flag == "operatingPoint") {
            if (index + 1 == args.size()) {
              throw runtime_error("every --operatingPoint must have an analysis name");
            }
            index++;
            fInfo.spOnlyOP.insert(boost::regex(cleanup(args[index])));
          }
          else if (flag == "jetAlgorithm") {
            if (index + 1 == args.size()) {
              throw runtime_error("every --jetAlgorithm must have an analysis name");
            }
            index++;
            fInfo.spOnlyJetAlgorithm.insert(boost::regex(cleanup(args[index])));
          }
          else if (flag == "analysis") {
            if (index + 1 == args.size()) {
              throw runtime_error("every --analysis must have an analysis name");
            }
            index++;
            fInfo.spOnlyAnalysis.insert(boost::regex(cleanup(args[index])));
          }
          else if (flag == "combinedName") {
            if (index + 1 == args.size()) {
              throw runtime_error("--combinedName must have a output combined name");
            }
            index++;
            operatingPoints.CombinationAnalysisName = args[index];
          }
          else if (flag == "binbybin") {
            operatingPoints.BinByBin = true;
          }
          else if (flag == "profile") {
            operatingPoints.BinByBin = false;
          }
          else {
            unknownFlags.push_back(flag);
          }

        }
        else {
          filesToLoad.push_back(a);
        }
      }
    }

    //
    // Now that we have a complete profile of everything, load in the files.
    //

    for (size_t i = 0; i < filesToLoad.size(); i++) {
      loadOPsFromFile(operatingPoints, filesToLoad[i], fInfo);
    }

    //
    // Remove any systematic errors we've been asked to.
    //

    for (size_t i_sys = 0; i_sys < ignoreSysError.size(); i_sys++) {
      for (vector<CalibrationAnalysis>::iterator anaItr = operatingPoints.Analyses.begin(); anaItr != operatingPoints.Analyses.end(); anaItr++) {
        for (vector<CalibrationBin>::iterator i_bin = anaItr->bins.begin(); i_bin != anaItr->bins.end(); i_bin++) {
          vector<SystematicError> errs;
          // copy_if does not seem to be available
          for (size_t i_se = 0; i_se < i_bin->systematicErrors.size(); i_se++) {
            if (ignoreSysError[i_sys] != i_bin->systematicErrors[i_se].name)
              errs.push_back(i_bin->systematicErrors[i_se]);
          }
          i_bin->systematicErrors = errs;
        }
      }
    }

    //
    // Process the copy commands... we will basically copy over anything listed
    //

    for (size_t i = 0; i < operatingPoints.Aliases.size(); i++) {
      const AliasAnalysis a(operatingPoints.Aliases[i]);
      vector<CalibrationAnalysis> copied;
      for (vector<CalibrationAnalysis>::const_iterator anaItr = operatingPoints.Analyses.begin(); anaItr != operatingPoints.Analyses.end(); anaItr++) {
        if (OPFullName(*anaItr) == OPFullName(a)) {
          for (vector<AliasAnalysisCopyTo>::const_iterator cItr = a.CopyTargets.begin(); cItr != a.CopyTargets.end(); cItr++) {
            CalibrationAnalysis cp(*anaItr);
            cp.name = cItr->name;
            cp.flavor = cItr->flavor;
            cp.tagger = cItr->tagger;
            cp.operatingPoint = cItr->operatingPoint;
            cp.jetAlgorithm = cItr->jetAlgorithm;
            copied.push_back(cp);
          }
        }
      }
      operatingPoints.Analyses.insert(operatingPoints.Analyses.begin(), copied.begin(), copied.end());
    }

    operatingPoints.Aliases.clear();

    //
    // Process the main ignore lists - anything fully matching these are
    // dropped.
    //

    FilterAnalyses(operatingPoints, fInfo);

    //
    // After that we may have some analyses which have zero bins in them. In that case
    // we want to eliminate them.
    //

    for (size_t op = operatingPoints.Analyses.size(); op > size_t(0); op--) {
      if (operatingPoints.Analyses[op - 1].bins.size() == 0) {
        operatingPoints.Analyses.erase(operatingPoints.Analyses.begin() + (op - 1));
      }
    }

    for (size_t cr = operatingPoints.Correlations.size(); cr > size_t(0); cr--) {
      if (operatingPoints.Correlations[cr - 1].bins.size() == 0) {
        operatingPoints.Correlations.erase(operatingPoints.Correlations.begin() + (cr - 1));
      }
    }
  }

  //
  // Split analyzes into lists. These lists are generally what we need when dealing
  // with the combination.
  //
  map<string, vector<CalibrationAnalysis> > BinAnalysesByJetTagFlavOp(const vector<CalibrationAnalysis> &anas)
  {
    map<string, vector<CalibrationAnalysis> > result;
    for (size_t i = 0; i < anas.size(); i++) {
      result[OPIndependentName(anas[i])].push_back(anas[i]);
    }
    return result;
  }

  //
  // Given two analyses with the same name, combine their bins.
  vector<CalibrationAnalysis> CombineSameAnalyses(const vector<CalibrationAnalysis> &anas)
  {
    // Combine when they are the same.
    map<string, CalibrationAnalysis> combinedAnalyses;
    for (vector<CalibrationAnalysis>::const_iterator itr(anas.begin()); itr != anas.end(); itr++) {
      string anaName(OPFullName(*itr));
      map<string, CalibrationAnalysis>::const_iterator f = combinedAnalyses.find(anaName);
      if (f == combinedAnalyses.end()) {
        combinedAnalyses[anaName] = *itr;
      }
      else {
        combinedAnalyses[anaName].bins.insert(combinedAnalyses[anaName].bins.end(), itr->bins.begin(), itr->bins.end());
      }
    }

    // Change the map back into a list. For each one make sure we can calculate a reasonable set of binning boundaries.
    // Only return bins that have at least one bin in them (e.g. aren't empty!).
    vector<CalibrationAnalysis> result;
    for (map<string, CalibrationAnalysis>::const_iterator itr(combinedAnalyses.begin()); itr != combinedAnalyses.end(); itr++) {
      if (itr->second.bins.size() > 0) {
        bin_boundaries temp(calcBoundaries(itr->second, false));
        result.push_back(itr->second);
      }
    }

    return result;
  }

}
