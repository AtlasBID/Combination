///
/// CppUnit tests for the argument processor
///

#include "Combination/CommonCommandLineUtils.h"
#include "Combination/BinNameUtils.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace BTagCombination;

//
// Test harness/fixture for the parser
//

class CommonCommandLineUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CommonCommandLineUtilsTest );

  CPPUNIT_TEST( testEmptyCommandLine );
  CPPUNIT_TEST( testUnknownFlag );
  CPPUNIT_TEST( testInputFromFile );
  CPPUNIT_TEST( testCorrelation );
  CPPUNIT_TEST_EXCEPTION( testInputFromBadFile, std::runtime_error );

  CPPUNIT_TEST( testInputFromFileWithSpitAna );

  CPPUNIT_TEST( testOPName );
  CPPUNIT_TEST( testOPBin );
  CPPUNIT_TEST( testOPBinOrdering1 );
  CPPUNIT_TEST( testOPBinOrdering2 );
  CPPUNIT_TEST( testOPIgnoreCorrelatedOrdering );

  CPPUNIT_TEST ( testIgnoreFlag );
  CPPUNIT_TEST ( testIgnoreFlagWildcard );
  CPPUNIT_TEST ( testIgnoreFlagWildcard2 );
  CPPUNIT_TEST ( testIgnoreFlagFile );
  CPPUNIT_TEST ( testIgnoreAnalysis );
  CPPUNIT_TEST ( testIgnoreCorrelation );
  CPPUNIT_TEST ( testIgnoreSystematicError );

  CPPUNIT_TEST ( testCombinationAnalysisName1 );
  CPPUNIT_TEST ( testCombinationAnalysisName2 );

  CPPUNIT_TEST ( testBinByBin1 );
  CPPUNIT_TEST ( testBinByBin2 );

  CPPUNIT_TEST ( testUseOnlyFlags );
  CPPUNIT_TEST ( testUseOnlyFlags2 );
  CPPUNIT_TEST ( testUseOnlyFlags3 );
  CPPUNIT_TEST ( testUseOnlyFlags4 );
  CPPUNIT_TEST ( testUseOnlyFlags5 );
  CPPUNIT_TEST ( testUseOnlyFlags6 );
  CPPUNIT_TEST ( testUseOnlyFlags7 );
  CPPUNIT_TEST ( testUseOnlyFlags8 );
  CPPUNIT_TEST ( testUseOnlyFlags9 );
  CPPUNIT_TEST ( testUseOnlyFlags10 );
  CPPUNIT_TEST ( testUseOnlyFlags11 );

  CPPUNIT_TEST ( testCopyAnalysis);

  CPPUNIT_TEST ( testSplitAnalysis );
  CPPUNIT_TEST ( testSplitAnalysis2 );
  CPPUNIT_TEST ( testSplitAnalysis3 );

  CPPUNIT_TEST_SUITE_END();

  void testEmptyCommandLine()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"hi", "there"};
    
    ParseOPInputArgs(argv, 0, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t)0, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, results.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, unknown.size());
  }

  void testUnknownFlag()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"--doit"};

    ParseOPInputArgs(argv, 1, results, unknown);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("#results", (size_t)0, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("#unknowns", (size_t)1, unknown.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unknown flag name", string("doit"), unknown[0]);
  }

  void testInputFromFile()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt"};

    ParseOPInputArgs(argv, 1, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results.Analyses[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("calibration analysis name", string("ttbar_kin_ljets"), ana.name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of bins", (size_t) 9, ana.bins.size());
  }

  void testInputFromFileWithSpitAna()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60Split.txt"};

    ParseOPInputArgs(argv, 1, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results.Analyses[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("calibration analysis name", string("ttbar_kin_ljets"), ana.name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of bins", (size_t) 9, ana.bins.size());
  }

  void testCorrelation()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/cor.txt"};

    ParseOPInputArgs(argv, 1, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    AnalysisCorrelation ac = results.Correlations[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("analysis1name", string("pTrel"), ac.analysis1Name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("analysis2name", string("system8"), ac.analysis2Name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("flavor", string("bottom"), ac.flavor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tagger", string("MV1"), ac.tagger);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("operatingpoint", string("0.905363"), ac.operatingPoint);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("jetalgorithm", string("AntiKt4Topo"), ac.jetAlgorithm);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("n bins", size_t(1), ac.bins.size());
    BinCorrelation b(ac.bins[0]);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.498885, b.statCorrelation, 0.0001);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("stat value good", true, b.hasStatCorrelation);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# bin spec", size_t(2), b.binSpec.size());
  }

  void testIgnoreFlag()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--ignore",
			  "ttbar_kin_ljets-bottom-JetTaggerCOMBNN-0.60-AntiKt4Topo:0-eta-4.5:25-pt-40"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results.Analyses[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("calibration analysis name", string("ttbar_kin_ljets"), ana.name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of bins", (size_t) 8, ana.bins.size());
  }

  void testIgnoreSystematicError()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--ignoreSysError",
			  "QCD"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results.Analyses[0];
    for (size_t i = 0; i < ana.bins.size(); i++) {
      const CalibrationBin &b(ana.bins[i]);
      for (size_t i_sys = 0; i_sys < b.systematicErrors.size(); i_sys++) {
	const SystematicError &s(b.systematicErrors[i_sys]);
	CPPUNIT_ASSERT (s.name != "QCD");
      }
    }
  }

  void testIgnoreFlagWildcard()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--ignore",
			  "ttbar_kin_ljets-.*-JetTaggerCOMBNN-0.60-AntiKt4Topo:0-eta-4.5:25-pt-40"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results.Analyses[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("calibration analysis name", string("ttbar_kin_ljets"), ana.name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of bins", (size_t) 8, ana.bins.size());
  }

  void testIgnoreFlagWildcard2()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--ignore",
			  "ttbar_kin_ljets-.*-JetTaggerCOMBNN-0.60-AntiKt4Topo:.*"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());
  }

  void testIgnoreFlagFile()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--ignore",
			  "@../testdata/ignorefile.txt"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results.Analyses[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("calibration analysis name", string("ttbar_kin_ljets"), ana.name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of bins", (size_t) 8, ana.bins.size());
  }

  void testIgnoreAnalysis()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--ignore",
			  "@../testdata/ignoreallfile.txt"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());
  }

  void testIgnoreCorrelation()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/cor.txt",
			  "--ignore",
			  "pTrel-system8-bottom-MV1-0.905363-AntiKt4Topo:0-abseta-2.5:110-pt-140"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());

    for (unsigned int i = 0; i < results.Correlations.size(); i++) {
      for (unsigned int b = 0; b < results.Correlations[i].bins.size(); b++) {
	cout << "Found a corelation: " 
	     << OPIgnoreFormat(results.Correlations[i], results.Correlations[i].bins[b])
	     << endl;
      }
    }

    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Correlations.size());
  }

  void testInputFromBadFile()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"simpleInputBogus.txt"};

    ParseOPInputArgs(argv, 1, results, unknown);
  }

  void testOPName()
  {
    CalibrationAnalysis ana;
    ana.name = "name";
    ana.flavor = "flavor";
    ana.tagger = "tagger";
    ana.operatingPoint = "op";
    ana.jetAlgorithm = "alg";

    string name (OPFullName(ana));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("name", string("name-flavor-tagger-op-alg"), name);
  }

  void testOPBin()
  {
    CalibrationBin bin;
    CalibrationBinBoundary spec1;
    spec1.lowvalue = 0;
    spec1.highvalue = 2.5;
    spec1.variable = "eta";
    bin.binSpec.push_back(spec1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("1 bin boundary", string("0-eta-2.5"), OPBinName(bin));

    CalibrationBinBoundary spec2;
    spec2.lowvalue = 0;
    spec2.highvalue = 95.0;
    spec2.variable = "pt";
    bin.binSpec.push_back(spec2);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("2 bin boundary", string("0-eta-2.5:0-pt-95"), OPBinName(bin));
  }

  void testOPBinOrdering1()
  {
    // It shouldn't matter what order you put bin-spec into a bin.

    CalibrationBin bin1;
    CalibrationBinBoundary spec1;
    spec1.lowvalue = 0;
    spec1.highvalue = 2.5;
    spec1.variable = "eta";
    bin1.binSpec.push_back(spec1);

    CalibrationBinBoundary spec2;
    spec2.lowvalue = 0;
    spec2.highvalue = 2.5;
    spec2.variable = "pt";
    bin1.binSpec.push_back(spec2);

    string inorder (OPBinName(bin1));

    CalibrationBin bin2;
    bin2.binSpec.push_back(spec2);
    bin2.binSpec.push_back(spec1);

    string rorder (OPBinName(bin2));

    CPPUNIT_ASSERT_EQUAL (inorder, rorder);
  }

  void testOPBinOrdering2()
  {
    // It shouldn't matter what order you put bin-spec into a bin.

    CalibrationBin bin1;
    CalibrationBinBoundary spec1;
    spec1.lowvalue = 0;
    spec1.highvalue = 2.5;
    spec1.variable = "eta";
    bin1.binSpec.push_back(spec1);

    CalibrationBinBoundary spec2;
    spec2.lowvalue = 30.0;
    spec2.highvalue = 40.0;
    spec2.variable = "pt";
    bin1.binSpec.push_back(spec2);

    string inorder (OPBinName(bin1));

    CalibrationBin bin2;
    bin2.binSpec.push_back(spec2);
    bin2.binSpec.push_back(spec1);

    string rorder (OPBinName(bin2));

    CPPUNIT_ASSERT_EQUAL (inorder, rorder);
  }

  void testOPIgnoreCorrelatedOrdering()
  {
    // It shouldn't matter what order you put bin-spec into a bin.

    CalibrationBinBoundary spec1;
    spec1.lowvalue = 0;
    spec1.highvalue = 2.5;
    spec1.variable = "eta";

    CalibrationBinBoundary spec2;
    spec2.lowvalue = 30.0;
    spec2.highvalue = 40.0;
    spec2.variable = "pt";

    BinCorrelation bin1;
    bin1.binSpec.push_back(spec1);
    bin1.binSpec.push_back(spec2);

    BinCorrelation bin2;
    bin2.binSpec.push_back(spec2);
    bin2.binSpec.push_back(spec1);

    // Create a correlated analysis to "host" this thing.
    AnalysisCorrelation cor;
    cor.analysis1Name = "s8";
    cor.analysis2Name = "pTrel";
    cor.flavor = "bottom";
    cor.tagger = "MV1";
    cor.operatingPoint = "0.601517";
    cor.jetAlgorithm = "AntiKt4Topo";

    pair<string, string> p1 (OPIgnoreCorrelatedFormat(cor, bin1));
    pair<string, string> p2 (OPIgnoreCorrelatedFormat(cor, bin2));

    CPPUNIT_ASSERT_EQUAL (p1.first, p2.first);
    CPPUNIT_ASSERT_EQUAL (p1.second, p2.second);
  }

  void testSplitAnalysis()
  {
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "sv0";
    ana.operatingPoint = "0.55";
    ana.jetAlgorithm = "AntiKt";
    
    vector<CalibrationAnalysis> list;
    list.push_back(ana);
    map<string, vector<CalibrationAnalysis> > r (BinAnalysesByJetTagFlavOp(list));
    CPPUNIT_ASSERT_EQUAL((size_t) 1, r.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 1, r.begin()->second.size());
  }

  void testSplitAnalysis2()
  {
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "sv0";
    ana.operatingPoint = "0.55";
    ana.jetAlgorithm = "AntiKt";
    
    CalibrationAnalysis ana2 (ana);
    ana2.name = "fork";

    vector<CalibrationAnalysis> list;
    list.push_back(ana);
    list.push_back(ana2);
    map<string, vector<CalibrationAnalysis> > r (BinAnalysesByJetTagFlavOp(list));
    CPPUNIT_ASSERT_EQUAL((size_t) 1, r.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 2, r.begin()->second.size());
  }

  void testSplitAnalysis3()
  {
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "sv0";
    ana.operatingPoint = "0.55";
    ana.jetAlgorithm = "AntiKt";
    
    CalibrationAnalysis ana2 (ana);
    ana2.tagger = "fork";

    vector<CalibrationAnalysis> list;
    list.push_back(ana);
    list.push_back(ana2);
    map<string, vector<CalibrationAnalysis> > r (BinAnalysesByJetTagFlavOp(list));
    CPPUNIT_ASSERT_EQUAL((size_t) 2, r.size());
  }

  void testUseOnlyFlags()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--flavor",
			  "top"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
  }

  void testUseOnlyFlags1()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--flavor",
			  "bottom"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
  }


  void testUseOnlyFlags2()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--tagger",
			  "FORK"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
  }

  void testUseOnlyFlags3()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--tagger",
			  "JetTaggerCOMBNN"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
  }

  void testUseOnlyFlags4()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--operatingPoint",
			  "Dude"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
  }

  void testUseOnlyFlags5()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--operatingPoint",
			  "0.60"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
  }

  void testUseOnlyFlags6()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--jetAlgorithm",
			  "Dude"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
  }

  void testUseOnlyFlags7()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--jetAlgorithm",
			  "AntiKt4Topo"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
  }

  void testUseOnlyFlags8()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/cor.txt",
			  "--jetAlgorithm",
			  "AntiKt4Topo"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Correlations.size());
  }

  void testUseOnlyFlags9()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/cor.txt",
			  "--jetAlgorithm",
			  "freak"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Correlations.size());
  }

  void testUseOnlyFlags10()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--analysis",
			  "freak"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Analyses.size());
  }

  void testUseOnlyFlags11()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--analysis",
			  "ttbar_kin_ljets"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, results.Correlations.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.Analyses.size());
  }

  void testCombinationAnalysisName1()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--combinedName",
			  "freak"
    };

    ParseOPInputArgs(argv, 3, results, unknown);
    CPPUNIT_ASSERT_EQUAL(string("freak"), results.CombinationAnalysisName);
  }

  void testBinByBin1()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt"
    };

    ParseOPInputArgs(argv, 1, results, unknown);
    CPPUNIT_ASSERT_EQUAL(false, results.BinByBin);
  }

  void testBinByBin2()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "--binbybin"
    };

    ParseOPInputArgs(argv, 2, results, unknown);
    CPPUNIT_ASSERT_EQUAL(true, results.BinByBin);
  }


  void testCombinationAnalysisName2()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt"
    };

    ParseOPInputArgs(argv, 1, results, unknown);
    CPPUNIT_ASSERT_EQUAL(string("combined"), results.CombinationAnalysisName);
  }

  void testCopyAnalysis()
  {
    CalibrationInfo results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt",
			  "../testdata/JetFitCopy.txt"
    };

    ParseOPInputArgs(argv, 2, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 3, results.Analyses.size());
    bool a_seen = false;
    bool b_seen = false;
    for (size_t i = 0; i < 3; i++) {
      const CalibrationAnalysis &a(results.Analyses[i]);
      if (a.name == "bbb") {
	a_seen = true;
	CPPUNIT_ASSERT_EQUAL(string("bottom"), a.flavor);
	CPPUNIT_ASSERT_EQUAL(string("JetTaggerCOMBNN"), a.tagger);
	CPPUNIT_ASSERT_EQUAL(string("0.60"), a.operatingPoint);
	CPPUNIT_ASSERT_EQUAL(string("AntiKt4Topo"), a.jetAlgorithm);
      }
      if (a.name == "aaa") {
	b_seen = true;
	CPPUNIT_ASSERT_EQUAL(string("light"), a.flavor);
	CPPUNIT_ASSERT_EQUAL(string("Foo"), a.tagger);
	CPPUNIT_ASSERT_EQUAL(string("0.70"), a.operatingPoint);
	CPPUNIT_ASSERT_EQUAL(string("AntiKtTopo"), a.jetAlgorithm);
      }
    }

    CPPUNIT_ASSERT_EQUAL (true, a_seen);
    CPPUNIT_ASSERT_EQUAL (true, b_seen);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CommonCommandLineUtilsTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
