//
// A diagnostics program that will dump
// out and check the input files
//

#include "Combination/Parser.h"
#include "Combination/CommonCommandLineUtils.h"
#include "Combination/BinBoundaryUtils.h"
#include "Combination/BinNameUtils.h"
#include "Combination/CalibrationDataModelStreams.h"
#include "Combination/FitLinage.h"

#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace BTagCombination;

// Helper routines forward defined.
void Usage(void);
void DumpEverything(const vector<CalibrationAnalysis> &calibs, ostream &output);
void CheckEverythingFullyCorrelated(const vector<CalibrationAnalysis> &info);
void CheckEverythingBinByBin(const vector<CalibrationAnalysis> &info);
void PrintNames(const CalibrationInfo &info, ostream &output);
void PrintQNames(const CalibrationInfo &info, ostream &output);
bool CompareNames(const CalibrationAnalysis &ana, ostream &output, string &inputfile);
void PrintLinage(const vector<CalibrationAnalysis> &calibs, ostream &output);

struct CaseInsensitiveCompare {
  bool operator() (const string &a, const string &b) const {
    return boost::ilexicographical_compare(a, b);
  }
};

class html_table {
public:
  html_table(ostream &out)
    : _out(out)
  {
    _out << "<table>";
  }
  virtual ~html_table()
  {
    _out << "</table>";
  }
  template <class InputIterator>
  void emit_line(const string &col1, InputIterator first, InputIterator last) {
    _out << "<tr><td>" << col1 << "</td>";
    while (first != last) {
      _out << "<td>" << *first << "</td>";
      first++;
    }
    _out << "</tr>" << endl;
  }
  template <class InputIterator>
  void emit_header(const string &col1, InputIterator first, InputIterator last)
  {
    emit_line(col1, first, last);
  }
private:
  ostream &_out;
};

string eatArg(char **argv, int &index, const int maxArg)
{
  if (index == (maxArg - 1))
    throw runtime_error("Not enough arguments.");
  index++;
  return argv[index];
}

// Main program - run & control everything.
int main(int argc, char **argv)
{
  if (argc <= 1) {
    Usage();
    return 1;
  }

  vector<string> otherArgs;

  // Parse the input args for commands
  string outputFilename;

  for (int i = 1; i < argc; i++) {
    string a(argv[i]);
    if (a == "output") {
      outputFilename = eatArg(argv, i, argc);
    }
    else {
      otherArgs.push_back(a);
    }
  }

  try {
    // Parse the input arguments
    CalibrationInfo info;
    vector<string> otherFlags;
    ParseOPInputArgs(otherArgs, info, otherFlags);

    bool useInputFile = false;
    string inputfile = "";

    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i].substr(0, 10) == "inputfile=") {
        istringstream is(otherFlags[i].substr(10, otherFlags[i].length() - 10));
        is >> inputfile;
        ifstream inputdata;
        inputdata.open(inputfile.c_str());
        if (!inputdata.is_open()) {
          cerr << "ERROR: couldn't open input file for cross checks" << endl;
          Usage();
          return 1;
        }
        else {
          useInputFile = true;
        }
      }
    }

    bool doCheck = false;
    bool doDump = false;
    bool doNames = false;
    bool doQNames = false;
    bool doCompareNames = false;
    bool printAsInput = false;
    bool printCorr = false;
    bool dumpMetaDataForCPU = false;
    bool dumpMetaDataForBins = false;
    bool dumpSysErrorUsage = false;
    bool dumpSysErrors = false;
    bool dumpLinage = false;

    bool sawFlag = false;
    for (unsigned int i = 0; i < otherFlags.size(); i++) {
      if (otherFlags[i] == "check") {
        doCheck = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "names") {
        doNames = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "qnames") {
        doQNames = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "linage") {
        dumpLinage = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "cnames") {
        if (useInputFile) {
          doCompareNames = true;
          sawFlag = true;
        }
        else {
          cerr << "Can't run " << otherFlags[i] << " without an input file!!" << endl;
          Usage();
          return 1;
        }
      }
      else if (otherFlags[i] == "asInput") {
        printAsInput = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "corr") {
        printCorr = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "meta") {
        dumpMetaDataForCPU = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "metaBins") {
        dumpMetaDataForBins = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "sysErrorTable") {
        dumpSysErrorUsage = true;
        sawFlag = true;
      }
      else if (otherFlags[i] == "sysErrors") {
        dumpSysErrors = true;
        sawFlag = true;
      }
      else {
        if (otherFlags[i].find("inputfile") == string::npos) {
          cerr << "Unknown command line option --" << otherFlags[i] << endl;
          Usage();
          return 1;
        }
      }
    }

    if (!sawFlag)
      doDump = true;

    // Do the output file
    ostream *output(&cout);
    if (outputFilename.size() > 0) {
      output = new ofstream(outputFilename.c_str());
    }

    const vector<CalibrationAnalysis> &calibs(info.Analyses);

    // Print out the correlation stuff
    if (printCorr) {
      for (size_t i_c = 0; i_c < info.Correlations.size(); i_c++) {
        const AnalysisCorrelation &c(info.Correlations[i_c]);
        for (size_t i_b = 0; i_b < c.bins.size(); i_b++) {
          BinCorrelation b(c.bins[i_b]);
          (*output)
            << c.analysis1Name
            << ", " << c.analysis2Name
            << ", " << c.flavor
            << ", " << c.tagger
            << ", " << c.operatingPoint
            << ", " << c.jetAlgorithm
            << ", " << OPBinName(b.binSpec)
            << ", " << b.statCorrelation
            << endl;
        }
      }
      return 0;
    }

    // If we need to dump systematic errors as a table

    if (dumpSysErrorUsage) {
      map<string, set<string>, CaseInsensitiveCompare> syserror_by_ana;
      set<string> analyses;

      for (size_t i_a = 0; i_a < calibs.size(); i_a++) {
        const CalibrationAnalysis &c(calibs[i_a]);
        analyses.insert(c.name);
        for (size_t i_b = 0; i_b < c.bins.size(); i_b++) {
          const CalibrationBin &b(c.bins[i_b]);
          for (size_t i_e = 0; i_e < b.systematicErrors.size(); i_e++) {
            const SystematicError &e(b.systematicErrors[i_e]);
            syserror_by_ana[e.name].insert(c.name);
          }
        }
      }

      html_table t(*output);
      t.emit_header("Sys Error", analyses.begin(), analyses.end());
      for (map<string, set<string> >::const_iterator i_err = syserror_by_ana.begin(); i_err != syserror_by_ana.end(); i_err++) {
        vector<string> usage;
        for (set<string>::const_iterator i_a = analyses.begin(); i_a != analyses.end(); i_a++) {
          if (i_err->second.find(*i_a) == i_err->second.end()) {
            usage.push_back("");
          }
          else {
            usage.push_back(*i_a);
          }
        }
        t.emit_line(i_err->first, usage.begin(), usage.end());
      }
    }

    // Dump a table (for each analysis) of the systematic errors
    if (dumpSysErrors) {
      for (auto &c : calibs) {
        *output << OPByCalibName(c) << endl;

        set<string> all_error_names;
        vector<string> bin_names;
        map<string, map<string, double>> error_values;
        for (auto &b : c.bins) {
          string bname(OPBinName(b.binSpec));
          bin_names.push_back(bname);
          for (auto &e : b.systematicErrors) {
            all_error_names.insert(e.name);
            error_values[bname][e.name] = e.value;
          }
        }

        *output << "Error";
        for (auto &bname : bin_names) {
          *output << "," << bname;
        }
        *output << endl;
        for (auto &ename : all_error_names) {
          *output << ename;
          for (auto &bname : bin_names) {
            auto eptr = error_values[bname].find(ename);
            *output << ",";
            if (eptr != error_values[bname].end()) {
              *output << eptr->second;
            }
          }
          *output << endl;
        }
        *output << endl;

      }
    }

    // Dump the meta data for the analysis that has run. We do this
    // just one at a time. This is meant to be ready in by someone else
    // and processed appropriately.

    if (dumpMetaDataForCPU) {
      for (size_t i_a = 0; i_a < calibs.size(); i_a++) {
        const CalibrationAnalysis &c(calibs[i_a]);

        stringstream bname;
        bname << c.name << " ** " << c.flavor << " ** " << c.tagger << " ** " << c.operatingPoint << " ** " << c.jetAlgorithm << " ** ";

        const map<string, vector<double> > &md(c.metadata);
        for (map<string, vector<double> >::const_iterator i_md = md.begin(); i_md != md.end(); i_md++) {
          (*output) << bname.str() << i_md->first << " ** ";
          for (vector<double>::const_iterator i_val = i_md->second.begin(); i_val != i_md->second.end(); i_val++)
            (*output) << *i_val << " ";
          (*output) << endl;
        }
      }
    }

    // Dump the meta data for the bins that have run. We do this
    // just one at a time. This is meant to be ready in by someone else
    // and processed appropriately.

    if (dumpMetaDataForBins) {
      for (size_t i_a = 0; i_a < calibs.size(); i_a++) {
        const CalibrationAnalysis &c(calibs[i_a]);

        stringstream bname;
        bname << c.name << " ** " << c.flavor << " ** " << c.tagger << " ** " << c.operatingPoint << " ** " << c.jetAlgorithm << " ** ";

        for (size_t i_b = 0; i_b < c.bins.size(); i_b++) {
          const CalibrationBin &b(c.bins[i_b]);
          string binname(OPBinName(b));
          for (map<string, pair<double, double> >::const_iterator i_m = b.metadata.begin(); i_m != b.metadata.end(); i_m++) {
            (*output) << bname.str()
              << i_m->first << " [from " << binname << "]"
              << " ** " << i_m->second.first
              << " " << i_m->second.second
              << endl;
          }
        }
      }
    }

    //
    // Dump out everything in our normalized format.
    //

    if (printAsInput) {
      (*output) << info << endl;
      return 0;
    }

    // Dump out a list of comma separated values
    if (doDump)
      DumpEverything(calibs, *output);

    // Check to see if there are overlapping bins
    if (doCheck) {
      CheckEverythingFullyCorrelated(calibs);
      checkForValidCorrelations(info);
    }

    if (doNames)
      PrintNames(info, *output);

    if (doQNames)
      PrintQNames(info, *output);

    if (doCompareNames) {
      bool isGood = true;
      for (size_t i_a = 0; i_a < calibs.size(); i_a++) {
        const CalibrationAnalysis &c(calibs[i_a]);
        if (!(CompareNames(c, *output, inputfile)))
          isGood = false;
      }
      return isGood ? 0 : 1;
    }

    if (dumpLinage)
      PrintLinage(info.Analyses, *output);

    // Check to see if the bin specifications are consistent.
    return 0;

  }
  catch (exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }

  return 0;
}

//
// Hold onto a single ana/bin - makes sorting and otherwise
// dealing with this a bit simpler in the code.
//
class holder
{
public:
  holder(const CalibrationAnalysis &ana, const CalibrationBin &bin)
    : _ana(ana), _bin(bin) {}

  inline string name() const { return OPFullName(_ana); }
  inline string binName() const { return OPBinName(_bin); }
  inline vector<string> sysErrorNames() const
  {
    vector<string> r;
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      r.push_back(_bin.systematicErrors[i].name);
    return r;
  }
  inline bool hasSysError(const string &name) const
  {
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      if (_bin.systematicErrors[i].name == name)
        return true;
    return false;
  }
  inline double sysError(const string &name) const
  {
    for (unsigned int i = 0; i < _bin.systematicErrors.size(); i++)
      if (_bin.systematicErrors[i].name == name)
        return _bin.systematicErrors[i].value;
    throw runtime_error(string("sys error '") + name + "' not known!");
  }
private:
  CalibrationAnalysis _ana;
  CalibrationBin _bin;
};

// Dump the linage in a nice easy-to-read way.
void PrintLinage(const vector<CalibrationAnalysis> &calibs, ostream &output)
{
  // Order by flavor, tagger, jet algorithm, operating point
  map<string, map<string, map<string, map<string, vector<CalibrationAnalysis> > > > > all;
  for (vector<CalibrationAnalysis>::const_iterator itr = calibs.begin(); itr != calibs.end(); itr++) {
    all[itr->flavor][itr->tagger][itr->jetAlgorithm][itr->operatingPoint].push_back(*itr);
  }

  for (map<string, map<string, map<string, map<string, vector<CalibrationAnalysis> > > > >::const_iterator i_flavor = all.begin(); i_flavor != all.end(); i_flavor++) {
    output << i_flavor->first << ":" << endl;
    for (map<string, map<string, map<string, vector<CalibrationAnalysis> > > >::const_iterator i_tagger = i_flavor->second.begin(); i_tagger != i_flavor->second.end(); i_tagger++) {
      output << "  " << i_tagger->first << ":" << endl;
      for (map<string, map<string, vector<CalibrationAnalysis> > >::const_iterator i_jet = i_tagger->second.begin(); i_jet != i_tagger->second.end(); i_jet++) {
        output << "    " << i_jet->first << ":" << endl;
        for (map<string, vector<CalibrationAnalysis> >::const_iterator i_op = i_jet->second.begin(); i_op != i_jet->second.end(); i_op++) {
          output << "      " << i_op->first << ":" << endl;
          for (vector<CalibrationAnalysis>::const_iterator i_ana = i_op->second.begin(); i_ana != i_op->second.end(); i_ana++) {
            output << "        " << i_ana->name << ": " << Linage(*i_ana) << endl;
          }
        }
      }
    }
  }
}

//
// Generate a comma seperated list of csv values.
//
void DumpEverything(const vector<CalibrationAnalysis> &calibs, ostream &output)
{
  vector<holder> held;
  for (unsigned int i = 0; i < calibs.size(); i++)
    for (unsigned int b = 0; b < calibs[i].bins.size(); b++)
      held.push_back(holder(calibs[i], calibs[i].bins[b]));

  // Get a complete list of all systematic errors!
  set<string> allsyserrors;
  for (unsigned int i = 0; i < held.size(); i++) {
    vector<string> binerr(held[i].sysErrorNames());
    allsyserrors.insert(binerr.begin(), binerr.end());
  }

  // Now that we are parsed, dump a comma seperated output to stdout...
  // hopefully this can be c/p into a excel file for nicer formatting.

  // Line1: analysis name headers
  for (unsigned int i = 0; i < held.size(); i++) {
    output << "," << held[i].name() << " " << held[i].binName();
  }
  output << endl;

  // Do a line for everything now...
  for (set<string>::const_iterator i = allsyserrors.begin(); i != allsyserrors.end(); i++) {
    output << *i;
    for (unsigned int h = 0; h < held.size(); h++) {
      output << ",";
      if (held[h].hasSysError(*i))
        output << held[h].sysError(*i);
    }
    output << endl;
  }
}

//
// Make sure there are no overlapping bins, etc. for each
// analysis.
//
void CheckEverythingFullyCorrelated(const vector<CalibrationAnalysis> &calibs)
{
  //
  // Split up everything by the analysis we are going to be done
  //

  typedef map<string, vector<CalibrationAnalysis> > t_CalibList;
  t_CalibList byBin(BinAnalysesByJetTagFlavOp(calibs));
  for (t_CalibList::const_iterator itr = byBin.begin(); itr != byBin.end(); itr++) {
    const vector<CalibrationAnalysis> anas(itr->second);

    // Check binning

    checkForConsistenBoundariesBinByBin(anas);

    // See if the various calibratoins are consistent for other reasons...

    checkForConsistentAnalyses(anas);
  }
}

void PrintNames(const vector<CalibrationAnalysis> &calibs, ostream &output, bool ignoreFormat = true)
{
  for (unsigned int i = 0; i < calibs.size(); i++) {
    for (unsigned int b = 0; b < calibs[i].bins.size(); b++) {
      if (ignoreFormat) {
        output << OPIgnoreFormat(calibs[i], calibs[i].bins[b]) << endl;
      }
      else {
        output << OPComputerFormat(calibs[i], calibs[i].bins[b]) << endl;
      }
    }
  }
}

void PrintNames(const vector<AnalysisCorrelation> &cors, ostream &output)
{
  for (size_t i = 0; i < cors.size(); i++) {
    for (size_t b = 0; b < cors[i].bins.size(); b++) {
      output << OPIgnoreFormat(cors[i], cors[i].bins[b]) << endl;
    }
  }
}

void PrintNames(const CalibrationInfo &info, ostream &output)
{
  PrintNames(info.Analyses, output);
  PrintNames(info.Correlations, output);
}

void PrintQNames(const CalibrationInfo &info, ostream &output)
{
  PrintNames(info.Analyses, output, false);
}

bool CompareNames(const CalibrationAnalysis &ana, ostream &output, string &inputfile)
{
  ifstream inputdata;
  inputdata.open(inputfile.c_str());
  bool isGood = true;

  output << " -> " << ana.name << "-" << ana.tagger << "-" << ana.operatingPoint << "-" << ana.jetAlgorithm << endl;

  string name = "", jet = "", tagger = "", op = "";
  vector<string> m_names, m_jets, m_taggers, m_wps, m_hadronizations;
  map <string, string> m_ops;

  string token;
  while (inputdata) {
    inputdata >> token;
    if (token == "calibration")  {
      inputdata >> name; m_names.push_back(name);
    }
    else if (token == "jet") {
      inputdata >> jet; m_jets.push_back(jet);
    }
    else if (token == "tagger") {
      inputdata >> tagger; m_taggers.push_back(tagger);
    }
  }

  bool isFound = false;
  for (unsigned int i = 0; i < m_names.size(); i++) {
    if (ana.name == m_names.at(i)) {
      isFound = true; break;
    }
  }
  if (!isFound) {
    output << "ERROR! Calibration method " << ana.name << " is unknown. Known calibration methods are: " << endl;
    isGood = false;
    for (unsigned int i = 0; i < m_names.size(); i++) output << "'" << m_names.at(i) << "' ";
    output << endl;
  }

  isFound = false;
  for (unsigned int i = 0; i < m_taggers.size(); i++) {
    if (ana.tagger == m_taggers.at(i)) {
      isFound = true; break;
    }
  }
  if (!isFound) {
    output << "ERROR! Tagger " << ana.tagger << " is unknown. Known taggers are: " << endl;
    isGood = false;
    for (unsigned int i = 0; i < m_taggers.size(); i++) output << "'" << m_taggers.at(i) << "' ";
    output << endl;
  }

  isFound = false;
  for (unsigned int i = 0; i < m_jets.size(); i++) {
    if (ana.jetAlgorithm == m_jets.at(i)) {
      isFound = true; break;
    }
  }
  if (!isFound) {
    output << "ERROR! Jet algorithm " << ana.jetAlgorithm << " is unknown. Known jet algorithms are: " << endl;
    isGood = false;
    for (unsigned int i = 0; i < m_jets.size(); i++) output << "'" << m_jets.at(i) << "' ";
    output << endl;
  }

  inputdata.clear();
  inputdata.seekg(0, ios::beg);

  string line;

  while (getline(inputdata, line)) {
    if (line.find("op") != string::npos) {
      std::istringstream iss(line);
      vector<string> words;
      for (int n = 0; n < 5; n++) {
        string word;
        iss >> word;
        if (n == 1)      words.push_back(word);
        else if (n == 3) words.push_back(word);
        else if (n == 4) words.push_back(word);
      }
      if (words.at(0) == ana.tagger) {
        if (ana.jetAlgorithm.find(words.at(1)) != string::npos) {
          m_wps.push_back(words.at(2));
        }
      }
    }
  }

  isFound = false;
  for (unsigned int i = 0; i < m_wps.size(); i++) {
    if (ana.operatingPoint == m_wps.at(i)) {
      isFound = true; break;
    }
  }
  if (!isFound) {
    output << "ERROR! OP " << ana.operatingPoint << " for tagger " << ana.tagger << " and jet collection " << ana.jetAlgorithm
      << " unknown. Known OPs for tagger " << ana.tagger << " are: " << endl;
    isGood = false;
    for (unsigned int i = 0; i < m_wps.size(); i++) output << "'" << m_wps.at(i) << "' ";
    output << endl;
  }

  inputdata.clear();
  inputdata.seekg(0, ios::beg);

  while (getline(inputdata, line)) {
    if (line.find("hadronization") != string::npos) {
      std::istringstream iss(line);
      vector<string> words;
      for (int n = 0; n < 3; n++) {
        string word;
        iss >> word;
        if (n == 1)      words.push_back(word);
        else if (n == 2) words.push_back(word);
      }
      string hadronization;
      std::map<std::string, std::string> meta = ana.metadata_s;
      for (std::map<std::string, std::string>::const_iterator itr = ana.metadata_s.begin(); itr != ana.metadata_s.end(); itr++) {
        if (words.at(0) == ana.name) {
          m_hadronizations.push_back(words.at(1));
        }
      }
    }
  }

  isFound = false;
  string hadronization;
  for (unsigned int i = 0; i < m_hadronizations.size(); i++) {
    std::map<std::string, std::string> meta = ana.metadata_s;
    for (std::map<std::string, std::string>::const_iterator itr = ana.metadata_s.begin(); itr != ana.metadata_s.end(); itr++) {
      if (itr->first == "Hadronization") {
        hadronization = itr->second;
        if (hadronization == m_hadronizations.at(i)) {
          isFound = true; break;
        }
      }
    }
  }
  if (!isFound && ana.name != "MCcalib") {
    output << "ERROR! Hadronization for calibration " << ana.name
      << " unknown. Should be: " << endl;
    isGood = false;
    for (unsigned int i = 0; i < m_hadronizations.size(); i++) output << "'" << m_hadronizations.at(i) << "' ";
    output << endl;
  }

  return isGood;
}

void Usage(void)
{
  cout << "FTDump <file-list-and-options>" << endl;
  cout << "  --check - check if the binning of the input is self consistent" << endl;
  cout << "  --names - print out the names used for the --ignore command of everything" << endl;
  cout << "  --qnames - print out the names used in a fully qualified, and easily computer parsable format" << endl;
  cout << "  --cnames - parse the code and compares variables to a list of known values (needs input file)" << endl;
  cout << "  --asInput - print out the inputs as a single file after applying all command line options" << endl;
  cout << "  --corr - print out the correlation inputs in a CSV command format" << endl;
  cout << "  --inputfile - to be used together with cnames flag (input files are located inside the directory inputdata)" << endl;
  cout << "  --linage - print out the linage for all input analyses" << endl;
  cout << "  output <fname> - all output is sent to fname" << endl;
}
