///
/// CppUnit tests for the Bin boundary utils.
///
///

#include "Combination/BinBoundaryUtils.h"

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

class BinBoundaryUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( BinBoundaryUtilsTest );

  CPPUNIT_TEST( testSimpleOneBins );
  CPPUNIT_TEST_EXCEPTION ( TestOverlapNegEta, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestOverlapBins, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestGappedBins, std::runtime_error );

  CPPUNIT_TEST (TestBBOK1D);
  CPPUNIT_TEST (TestBBOK1D2);
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D2, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D3, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D4, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D5, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D6, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D7, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBMissingAxes1D, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBMissingAxes1D2, std::runtime_error );

  CPPUNIT_TEST_EXCEPTION ( TestSystematicErrorsInconsistent, std::runtime_error );
  CPPUNIT_TEST ( TestSystematicErrorsMatching);

  CPPUNIT_TEST ( testCorrelations );
  CPPUNIT_TEST_EXCEPTION ( testCorrelations2, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( testCorrelations3, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( testCorrelations4, std::runtime_error );

  CPPUNIT_TEST_SUITE_END();

  void testSimpleOneBins()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestOverlapBins()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);

    CalibrationBinBoundary bb2;
    bb2.lowvalue = 0.5;
    bb2.highvalue = 1.5;
    bb2.variable = "pt";
    b1.binSpec.push_back(bb2);

    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestOverlapNegEta()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "eta";
    b1.binSpec.push_back(bb1);

    CalibrationBinBoundary bb2;
    bb2.lowvalue = -2.5;
    bb2.highvalue = 2.5;
    bb2.variable = "eta";
    b1.binSpec.push_back(bb2);

    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestGappedBins ()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);

    CalibrationBinBoundary bb2;
    bb2.lowvalue = 2.0;
    bb2.highvalue = 3.0;
    bb2.variable = "pt";
    b1.binSpec.push_back(bb2);

    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestBBOK1D()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    b2.add_axis("e1", bounds);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOK1D2()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    bounds.push_back(5.0);
    b2.add_axis("e1", bounds);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(-2.5);
    bounds2.push_back(2.5);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D2()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D3()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(2.5);
    bounds.push_back(4.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D4()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(4.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D5()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(4.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D6()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D7()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b2);
    bb.push_back(b1);

    checkForConsitentBoundaries(bb);
  }

  void TestBBMissingAxes1D()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(-2.5);
    bounds2.push_back(2.5);
    b2.add_axis("e2", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBMissingAxes1D2()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);
    b1.add_axis("e2", bounds);

    bin_boundaries b2;
    b2.add_axis("e1", bounds);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestSystematicErrorsInconsistent()
  {
    CalibrationAnalysis ana1;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "eta";
    b1.binSpec.push_back(bb1);

    SystematicError e1;
    e1.name = "hi";
    e1.value = 1.0;
    e1.uncorrelated = true;
    b1.systematicErrors.push_back(e1);
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2;
    b1.systematicErrors[0].uncorrelated = false;
    ana2.bins.push_back(b1);

    vector<CalibrationAnalysis> list;
    list.push_back(ana1);
    list.push_back(ana2);

    checkForConsistentAnalyses(list);
  }

  void TestSystematicErrorsMatching()
  {
    CalibrationAnalysis ana1;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "eta";
    b1.binSpec.push_back(bb1);

    SystematicError e1;
    e1.name = "hi";
    e1.value = 1.0;
    e1.uncorrelated = true;
    b1.systematicErrors.push_back(e1);
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2;
    ana2.bins.push_back(b1);

    vector<CalibrationAnalysis> list;
    list.push_back(ana1);
    list.push_back(ana2);

    checkForConsistentAnalyses(list);
  }

  void testCorrelations()
  {
    CalibrationInfo info;
    CalibrationAnalysis ana1, ana2;
    ana1.name = "ptrel";
    ana1.flavor = "bottom";
    ana1.tagger = "MV1";
    ana1.operatingPoint = "0.2";
    ana1.jetAlgorithm = "antikt";
    ana2.name = "s8";
    ana2.flavor = "bottom";
    ana2.tagger = "MV1";
    ana2.operatingPoint = "0.2";
    ana2.jetAlgorithm = "antikt";

    CalibrationBin b;
    CalibrationBinBoundary bb;
    bb.variable = "pt";
    bb.lowvalue = 0.0;
    bb.highvalue = 2.5;
    b.binSpec.push_back(bb);
    ana1.bins.push_back(b);
    ana2.bins.push_back(b);

    info.Analyses.push_back(ana1);
    info.Analyses.push_back(ana2);

    AnalysisCorrelation cor;
    cor.analysis1Name = "s8";
    cor.analysis2Name = "ptrel";
    cor.flavor = "bottom";
    cor.tagger = "MV1";
    cor.operatingPoint = "0.2";
    cor.jetAlgorithm = "antikt";

    BinCorrelation bc;
    bc.binSpec.push_back(bb);
    cor.bins.push_back(bc);
    
    info.Correlations.push_back(cor);

    checkForValidCorrelations(info);
  }

  void testCorrelations2()
  {
    CalibrationInfo info;
    CalibrationAnalysis ana1, ana2;
    ana1.name = "ptrel";
    ana1.flavor = "bottom";
    ana1.tagger = "MV1";
    ana1.operatingPoint = "0.2";
    ana1.jetAlgorithm = "antikt";
    ana2.name = "s8";
    ana2.flavor = "bottom";
    ana2.tagger = "MV1";
    ana2.operatingPoint = "0.2";
    ana2.jetAlgorithm = "antikt";

    CalibrationBin b;
    CalibrationBinBoundary bb;
    bb.variable = "pt";
    bb.lowvalue = 0.0;
    bb.highvalue = 2.5;
    b.binSpec.push_back(bb);
    ana1.bins.push_back(b);

    CalibrationBin b1;
    bb.highvalue = 2.1;
    b1.binSpec.push_back(bb);
    ana1.bins.push_back(b1);

    info.Analyses.push_back(ana1);
    info.Analyses.push_back(ana2);

    AnalysisCorrelation cor;
    cor.analysis1Name = "s8";
    cor.analysis2Name = "ptrel";
    cor.flavor = "bottom";
    cor.tagger = "MV1";
    cor.operatingPoint = "0.2";
    cor.jetAlgorithm = "antikt";

    BinCorrelation bc;
    bc.binSpec.push_back(bb);
    cor.bins.push_back(bc);
    
    info.Correlations.push_back(cor);

    checkForValidCorrelations(info);
  }

  void testCorrelations3()
  {
    CalibrationInfo info;
    CalibrationAnalysis ana1, ana2;
    ana1.name = "ptrel";
    ana1.flavor = "bottom";
    ana1.tagger = "MV1";
    ana1.operatingPoint = "0.2";
    ana1.jetAlgorithm = "antikt";
    ana2.name = "s8";
    ana2.flavor = "bottom";
    ana2.tagger = "MV1";
    ana2.operatingPoint = "0.2";
    ana2.jetAlgorithm = "antikt";

    CalibrationBin b;
    CalibrationBinBoundary bb;
    bb.variable = "pt";
    bb.lowvalue = 0.0;
    bb.highvalue = 2.5;
    b.binSpec.push_back(bb);
    ana1.bins.push_back(b);
    ana1.bins.push_back(b);

    info.Analyses.push_back(ana1);
    info.Analyses.push_back(ana2);

    AnalysisCorrelation cor;
    cor.analysis1Name = "s8";
    cor.analysis2Name = "ptr";
    cor.flavor = "bottom";
    cor.tagger = "MV1";
    cor.operatingPoint = "0.2";
    cor.jetAlgorithm = "antikt";

    BinCorrelation bc;
    bc.binSpec.push_back(bb);
    cor.bins.push_back(bc);
    
    info.Correlations.push_back(cor);

    checkForValidCorrelations(info);
  }

  void testCorrelations4()
  {
    CalibrationInfo info;
    CalibrationAnalysis ana1, ana2;
    ana1.name = "ptrel";
    ana1.flavor = "bottom";
    ana1.tagger = "MV1";
    ana1.operatingPoint = "0.2";
    ana1.jetAlgorithm = "antikt";
    ana2.name = "s8";
    ana2.flavor = "bottom";
    ana2.tagger = "MV1";
    ana2.operatingPoint = "0.2";
    ana2.jetAlgorithm = "antikt";

    CalibrationBin b;
    CalibrationBinBoundary bb;
    bb.variable = "pt";
    bb.lowvalue = 0.0;
    bb.highvalue = 2.5;
    b.binSpec.push_back(bb);
    ana1.bins.push_back(b);
    ana1.bins.push_back(b);

    info.Analyses.push_back(ana1);
    info.Analyses.push_back(ana2);

    AnalysisCorrelation cor;
    cor.analysis1Name = "s8";
    cor.analysis2Name = "s8";
    cor.flavor = "bottom";
    cor.tagger = "MV1";
    cor.operatingPoint = "0.2";
    cor.jetAlgorithm = "antikt";

    BinCorrelation bc;
    bc.binSpec.push_back(bb);
    cor.bins.push_back(bc);
    
    info.Correlations.push_back(cor);

    checkForValidCorrelations(info);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(BinBoundaryUtilsTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
