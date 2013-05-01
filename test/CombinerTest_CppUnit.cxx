///
/// CppUnit tests for the parser
///
///  Some pretty simple combinations - we know the results for these
/// cases, so make sure the code works as expected
///

#include "Combination/Combiner.h"
#include "Combination/BinUtils.h"

#include <RooMsgService.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>

#include <stdexcept>
#include <cmath>
#include <iostream>
#include <sstream>

using namespace std;
using namespace BTagCombination;

class CombinerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CombinerTest );

  CPPUNIT_TEST_EXCEPTION ( testOBZeroBinIn, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( testOBTwoDiffBins, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( testOBTwoDiffComplexBins, std::runtime_error );

  CPPUNIT_TEST ( testOBOneBinIn );
  CPPUNIT_TEST ( testOBOneBinInWithSys );
  CPPUNIT_TEST ( testOBTwoBinIn );
  CPPUNIT_TEST ( testOBTwoBinInWithSys );
  CPPUNIT_TEST ( testOBTwoBinInWithUnCorSys );

  CPPUNIT_TEST ( testAnaOne );
  CPPUNIT_TEST ( testAnaTwoDifBins );
  CPPUNIT_TEST ( testAnaTwoDifBinsDiffSys );
  //CPPUNIT_TEST_EXCPETION ( testAnaTwoBinsBad1, std::runtime_error );
  CPPUNIT_TEST ( testAnaTwoSamBins );
  CPPUNIT_TEST ( testAnaTwoDiffFitMetaData );
  CPPUNIT_TEST ( testAnaTwoWeirdBins );

  CPPUNIT_TEST ( testAnaDifOne );
  CPPUNIT_TEST ( testAnaDifOneByBin );
  CPPUNIT_TEST ( testAnaTwoBinsByBin );
  //CPPUNIT_TEST ( testAnaDifTwoSameBins );
  //CPPUNIT_TEST ( testAnaDifTwoDifAndSameBins );
  CPPUNIT_TEST ( testAnaTwoSameBinsUnCor );
  CPPUNIT_TEST ( testAnaTwoSameBinsUnCor2 );
  CPPUNIT_TEST ( testAnaTwoSameBinsUnCor3 );

  CPPUNIT_TEST ( testAnaTwoSameBinsStatCor );
  CPPUNIT_TEST ( testAnaTwoSameBinsStatCorByBin );
  CPPUNIT_TEST ( testAnaTwoSameBinsStatCorBad );

  CPPUNIT_TEST ( testAnaTwoDifBinsNoFit );

  CPPUNIT_TEST ( testAnaTrackMetadataByBin );
  CPPUNIT_TEST ( testAnaTrackMetadata );

  CPPUNIT_TEST_EXCEPTION ( rebinEmpty, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( rebinToEmpty, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( rebinMissingBin, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( rebinOverlappingBin, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( rebinTwoToOneWithGap, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( rebinThreeToOneWithOverlap, std::runtime_error );
  CPPUNIT_TEST ( rebinOneToOne );
  CPPUNIT_TEST ( rebinTwoToOne );
  CPPUNIT_TEST ( rebinThreeToOne );

  CPPUNIT_TEST_SUITE_END();

  void setupRoo()
  {
    RooMsgService::instance().setSilentMode(true);
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  }

  void setupRooTurnOn()
  {
    RooMsgService::instance().setSilentMode(false);
    RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
  }

  void testOBZeroBinIn()
  {
    // Fail if no bins input - nothing to combine!
    vector<CalibrationBin> bins;
    
    setupRoo();
    CalibrationBin b = CombineBin(bins);
  }

  void testOBOneBinIn()
  {
    // We send in one bin to be combined, and it comes back out!
    CalibrationBin b;
    b.centralValue = 0.5;
    b.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    
    vector<CalibrationBin> bins;
    bins.push_back(b);
    setupRoo();
    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.centralValue == 0.5);
    CPPUNIT_ASSERT(result.centralValueStatisticalError == 0.1);
    CPPUNIT_ASSERT(result.binSpec.size() == 1);
    CPPUNIT_ASSERT(result.binSpec[0].variable == "eta");
  }

  void testOBOneBinInWithSys()
  {
    // We send in one bin to be combined, and it comes back out!
    CalibrationBin b;
    b.centralValue = 2.0;
    b.centralValueStatisticalError = 0.4;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    
    SystematicError e1;
    e1.name = "s1";
    e1.value = 0.5748;
    e1.uncorrelated = false;
    b.systematicErrors.push_back(e1);
    e1.name = "s2";
    e1.value = 0.7322;
    e1.uncorrelated = false;
    b.systematicErrors.push_back(e1);

    vector<CalibrationBin> bins;
    bins.push_back(b);
    setupRoo();
    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.centralValue == 2.0);
    CPPUNIT_ASSERT(result.centralValueStatisticalError == 0.4);
  }

  void testOBTwoDiffBins()
  {
    // Try to combine two different bins, but the bins are different sized.
    // We send in one bin to be combined, and it comes back out!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    
    CalibrationBin b2;
    b2.centralValue = 0.5;
    b2.centralValueStatisticalError = 0.1;
    bound.lowvalue = 1.0;
    b2.binSpec.push_back(bound);

    vector<CalibrationBin> bins;
    bins.push_back(b1);
    bins.push_back(b2);

    setupRoo();
    CalibrationBin result = CombineBin(bins);
  }

  // Look to see if two doubles are close - we are dealing with rounding
  // errors, etc., for unit tests here.
  bool isCloseTo (double actual, double expected)
  {
    if (expected == 0)
      return actual == 0;

    return abs((actual - expected)/expected) < 0.001;
  }

  void testOBTwoBinIn()
  {
    // Combine two identical bins with no sys errors
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    
    vector<CalibrationBin> bins;
    bins.push_back(b1);
    bins.push_back(b1);

    setupRoo();
    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.binSpec.size() == 1);
    CPPUNIT_ASSERT(result.binSpec[0].variable == "eta");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, result.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(0.1*0.1/2.0), result.centralValueStatisticalError, 0.001);
  }

  void testOBTwoBinInWithSys()
  {
    // Combine two identical bins with no sys errors
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    s1.uncorrelated = false;
    b1.systematicErrors.push_back(s1);
    
    vector<CalibrationBin> bins;
    bins.push_back(b1);
    bins.push_back(b1);

    setupRoo();
    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.binSpec.size() == 1);
    CPPUNIT_ASSERT(result.binSpec[0].variable == "eta");

    CPPUNIT_ASSERT(result.systematicErrors.size() == 1);
    SystematicError s1r (result.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL (string("s1"), s1r.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, s1r.value, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (false, s1r.uncorrelated, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, result.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(0.1*0.1/2.0), result.centralValueStatisticalError, 0.001);
  }

  void testOBTwoBinInWithUnCorSys()
  {
    // Combine two identical bins with no sys errors
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    s1.uncorrelated = true;
    b1.systematicErrors.push_back(s1);
    
    vector<CalibrationBin> bins;
    bins.push_back(b1);
    bins.push_back(b1);

    setupRoo();
    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.binSpec.size() == 1);
    CPPUNIT_ASSERT(result.binSpec[0].variable == "eta");

    CPPUNIT_ASSERT(result.systematicErrors.size() == 1);
    SystematicError s1r (result.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL (string("s1"), s1r.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, s1r.value, 0.01);
    CPPUNIT_ASSERT_EQUAL (true, s1r.uncorrelated);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, result.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(0.1*0.1/2.0), result.centralValueStatisticalError, 0.001);
  }

  void testOBTwoDiffComplexBins()
  {
    // Try to combine two different bins, but the bins are different sized.
    // We send in one bin to be combined, and it comes back out!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound1;
    bound1.variable = "eta";
    bound1.lowvalue = 0.0;
    bound1.highvalue = 2.5;
    CalibrationBinBoundary bound2;
    bound2.variable = "pt";
    bound2.lowvalue = 0.0;
    bound2.highvalue = 100.0;

    b1.binSpec.push_back(bound1);
    b1.binSpec.push_back(bound2);
    
    CalibrationBin b2;
    b2.centralValue = 0.5;
    b2.centralValueStatisticalError = 0.1;
    bound1.lowvalue = 1.0;
    b2.binSpec.push_back(bound2);
    b2.binSpec.push_back(bound1);

    vector<CalibrationBin> bins;
    bins.push_back(b1);
    bins.push_back(b2);

    setupRoo();
    CalibrationBin result = CombineBin(bins);
  }

  void testAnaOne()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana;
    ana.name = "s8";
    ana.flavor = "bottom";
    ana.tagger = "comb";
    ana.operatingPoint = "0.50";
    ana.jetAlgorithm = "AntiKt4Topo";
    ana.bins.push_back(b1);

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    CPPUNIT_ASSERT_EQUAL (string("combined"), result.name);
    CPPUNIT_ASSERT_EQUAL (string("bottom"), result.flavor);
    CPPUNIT_ASSERT_EQUAL (string("comb"), result.tagger);
    CPPUNIT_ASSERT_EQUAL (string("0.50"), result.operatingPoint);
    CPPUNIT_ASSERT_EQUAL (string("AntiKt4Topo"), result.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL (size_t(1), result.bins.size());
    CalibrationBin b (result.bins[0]);
    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, b.centralValue, 0.01);
  }

  void testAnaTwoSamBins()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    CPPUNIT_ASSERT_EQUAL (string("combined"), result.name);
    CPPUNIT_ASSERT_EQUAL (string("bottom"), result.flavor);
    CPPUNIT_ASSERT_EQUAL (string("comb"), result.tagger);
    CPPUNIT_ASSERT_EQUAL (string("0.50"), result.operatingPoint);
    CPPUNIT_ASSERT_EQUAL (string("AntiKt4Topo"), result.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL (size_t(1), result.bins.size());
    CalibrationBin b (result.bins[0]);
    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, b.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(0.1*0.1/2.0), b.centralValueStatisticalError, 0.001);
  }

  void testAnaTwoSameBinsUnCor()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 1.0;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    s1.uncorrelated = true;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana1.operatingPoint = "0.50";

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));

    CalibrationBin b (result.bins[0]);

    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, b.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(0.1*0.1/2.0), b.centralValueStatisticalError, 0.001);

    // Make sure the systematic errors that came back are "good" too.
    CPPUNIT_ASSERT_EQUAL((size_t)1, b.systematicErrors.size());
    SystematicError e(b.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(true, e.uncorrelated);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, e.value, 0.001);
  }

  void testAnaTwoSameBinsUnCor2()
  {
    // Two ana, sys errors that are uncorrelated, but different bins...
    CalibrationBin b1;
    b1.centralValue = 1.0;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.2;
    s1.uncorrelated = true;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.40";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana2.bins[0].binSpec[0].variable = "pt";
    ana2.bins[0].systematicErrors[0].value = 0.5;

    cout << "ana1:" << endl << ana1;
    cout << "ana2:" << endl << ana2;

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    cout << "Result: " << endl << result;

    CPPUNIT_ASSERT_EQUAL(size_t(2), result.bins.size());

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), b_1.systematicErrors.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), b_2.systematicErrors.size());

    SystematicError e1(b_1.systematicErrors[0]);
    SystematicError e2(b_2.systematicErrors[0]);

    CPPUNIT_ASSERT_EQUAL(true, e1.uncorrelated);
    CPPUNIT_ASSERT_EQUAL(true, e2.uncorrelated);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, e1.value, 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, e2.value, 0.05);
  }

  void testAnaTwoSameBinsUnCor3()
  {
    // Same as the UnCor2 test, but we do different systeatmic errors...
    // Make sure it gives teh same result.

    CalibrationBin b1;
    b1.centralValue = 1.0;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.2;
    s1.uncorrelated = false;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.40";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana2.bins[0].binSpec[0].variable = "pt";
    ana2.bins[0].systematicErrors[0].value = 0.5;
    ana2.bins[0].systematicErrors[0].name = "s2";

    cout << "ana1:" << endl << ana1;
    cout << "ana2:" << endl << ana2;

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    cout << "Result: " << endl << result;

    CPPUNIT_ASSERT_EQUAL(size_t(2), result.bins.size());

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);

    CPPUNIT_ASSERT_EQUAL(size_t(1), b_1.systematicErrors.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), b_2.systematicErrors.size());

    SystematicError e1(b_1.systematicErrors[0]);
    SystematicError e2(b_2.systematicErrors[0]);

    CPPUNIT_ASSERT_EQUAL(false, e1.uncorrelated);
    CPPUNIT_ASSERT_EQUAL(false, e2.uncorrelated);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, e1.value, 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, e2.value, 0.05);
  }

  void testAnaTwoSameBinsStatCor()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 1.0;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";

    AnalysisCorrelation c1;
    c1.analysis1Name = "s8";
    c1.analysis2Name = "ptrel";
    c1.flavor = "bottom";
    c1.tagger = "comb";
    c1.operatingPoint = "0.50";
    c1.jetAlgorithm = "AntiKt4Topo";

    BinCorrelation bc1;
    bc1.hasStatCorrelation = true;
    bc1.statCorrelation = 1.0;
    bc1.binSpec.push_back(bound);
    c1.bins.push_back(bc1);

    CalibrationInfo info;
    info.Analyses.push_back (ana1);
    info.Analyses.push_back (ana2);
    info.Correlations.push_back(c1);

    setupRoo();
    vector<CalibrationAnalysis> result (CombineAnalyses(info));

    CPPUNIT_ASSERT_EQUAL (size_t(1), result.size());

    CalibrationBin b (result[0].bins[0]);

    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, b.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, b.centralValueStatisticalError, 0.001);

    // Make sure the systematic errors that came back are "good" too.
    CPPUNIT_ASSERT_EQUAL((size_t)0, b.systematicErrors.size());
  }

  void testAnaTwoSameBinsStatCorByBin()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 1.0;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";

    AnalysisCorrelation c1;
    c1.analysis1Name = "s8";
    c1.analysis2Name = "ptrel";
    c1.flavor = "bottom";
    c1.tagger = "comb";
    c1.operatingPoint = "0.50";
    c1.jetAlgorithm = "AntiKt4Topo";

    BinCorrelation bc1;
    bc1.hasStatCorrelation = true;
    bc1.statCorrelation = 1.0;
    bc1.binSpec.push_back(bound);
    c1.bins.push_back(bc1);

    CalibrationInfo info;
    info.Analyses.push_back (ana1);
    info.Analyses.push_back (ana2);
    info.Correlations.push_back(c1);

    setupRoo();
    vector<CalibrationAnalysis> result (CombineAnalyses(info, true, kCombineBySingleBin));

    CPPUNIT_ASSERT_EQUAL (size_t(1), result.size());

    CalibrationBin b (result[0].bins[0]);

    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, b.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, b.centralValueStatisticalError, 0.001);

    // Make sure the systematic errors that came back are "good" too.
    CPPUNIT_ASSERT_EQUAL((size_t)0, b.systematicErrors.size());
  }

  void testAnaTwoSameBinsStatCorBad()
  {
    // Combine with statistical correlation - but the correlation is bad.
    // Bad, that is, in the sense that we are dealing with a correlation that
    // pushes the weights negative.

    CalibrationBin b1;
    b1.centralValue = 1.0;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana2.bins[0].centralValueStatisticalError = 0.05;

    AnalysisCorrelation c1;
    c1.analysis1Name = "s8";
    c1.analysis2Name = "ptrel";
    c1.flavor = "bottom";
    c1.tagger = "comb";
    c1.operatingPoint = "0.50";
    c1.jetAlgorithm = "AntiKt4Topo";

    BinCorrelation bc1;
    bc1.hasStatCorrelation = true;
    bc1.statCorrelation = 1.0;
    bc1.binSpec.push_back(bound);
    c1.bins.push_back(bc1);

    CalibrationInfo info;
    info.Analyses.push_back (ana1);
    info.Analyses.push_back (ana2);
    info.Correlations.push_back(c1);

    setupRoo();
    info.CombinationAnalysisName = "dork";
    vector<CalibrationAnalysis> result (CombineAnalyses(info));

    CPPUNIT_ASSERT_EQUAL (size_t(1), result.size());
    CPPUNIT_ASSERT_EQUAL (string("dork"), result[0].name);

    CalibrationBin b (result[0].bins[0]);

    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, b.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.05, b.centralValueStatisticalError, 0.001);

    // Make sure the systematic errors that came back are "good" too.
    CPPUNIT_ASSERT_EQUAL((size_t)0, b.systematicErrors.size());
  }

  void testAnaTwoDifBins()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana2.bins[0].binSpec[0].lowvalue = 2.5;
    ana2.bins[0].binSpec[0].highvalue = 4.5;
    ana2.bins[0].centralValue = 1.0;
    ana2.bins[0].centralValueStatisticalError = 0.2;
    ana2.bins[0].systematicErrors[0].value = 0.2;

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    CPPUNIT_ASSERT_EQUAL (string("combined"), result.name);
    CPPUNIT_ASSERT_EQUAL (string("bottom"), result.flavor);
    CPPUNIT_ASSERT_EQUAL (string("comb"), result.tagger);
    CPPUNIT_ASSERT_EQUAL (string("0.50"), result.operatingPoint);
    CPPUNIT_ASSERT_EQUAL (string("AntiKt4Topo"), result.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL (size_t(2), result.bins.size());

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);
    CPPUNIT_ASSERT_EQUAL (size_t(1), b_1.binSpec.size());
    CPPUNIT_ASSERT_EQUAL (size_t(1), b_2.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, b_1.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, b_1.centralValueStatisticalError, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, b_2.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, b_2.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL(size_t(1), b_1.systematicErrors.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), b_2.systematicErrors.size());
    SystematicError e1 (b_1.systematicErrors[0]);
    SystematicError e2 (b_2.systematicErrors[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, e1.value, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, e2.value, 0.01);
  }


  void testAnaTwoDiffFitMetaData()
  {
    cout << "Starting testAnaTwoDiffFitMetaData" << endl;
    // Simple fit, but make sure the meta data comes out as we expect.
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana2.bins[0].binSpec[0].lowvalue = 2.5;
    ana2.bins[0].binSpec[0].highvalue = 4.5;
    ana2.bins[0].centralValue = 1.0;
    ana2.bins[0].centralValueStatisticalError = 0.2;
    ana2.bins[0].systematicErrors[0].value = 0.2;

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));

    cout << result << endl;

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, b_1.metadata["CV Shift s1"].first, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, b_2.metadata["CV Shift s1"].first, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, b_1.metadata["CV Shift s1"].second, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, b_2.metadata["CV Shift s1"].second, 0.001);

    //CPPUNIT_ASSERT_EQUAL(size_t(1), result.metadata["Pull s1"].size());
    //CPPUNIT_ASSERT_EQUAL(size_t(2), result.metadata["Nuisance s1"].size());
  }

  void testAnaTwoBinsByBin()
  {
    // Two analyses, and 2 bins in each.
    // Combine by bin, shoudl ignore any correlation
    // with sys errors.

    cout << "Starting testAnaTwoBinsByBin" << endl;

    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationBin b2;
    b2.centralValue = 0.75;
    b2.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound2;
    bound2.variable = "eta";
    bound2.lowvalue = 2.5;
    bound2.highvalue = 5.0;
    b2.binSpec.push_back(bound2);

    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);
    b2.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);
    ana1.bins.push_back(b2);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";

    CalibrationInfo inputs;
    inputs.Analyses.push_back (ana1);
    inputs.Analyses.push_back (ana2);

    inputs.CombinationAnalysisName = "combined";

    setupRoo();
    vector<CalibrationAnalysis> results (CombineAnalyses(inputs, true, kCombineBySingleBin));
    CPPUNIT_ASSERT_EQUAL((size_t)1, results.size());
    CalibrationAnalysis &result(results[0]);

    cout << "Result: " << endl;
    cout << result << endl;

    CPPUNIT_ASSERT_EQUAL (string("combined"), result.name);
    CPPUNIT_ASSERT_EQUAL (string("bottom"), result.flavor);
    CPPUNIT_ASSERT_EQUAL (string("comb"), result.tagger);
    CPPUNIT_ASSERT_EQUAL (string("0.50"), result.operatingPoint);
    CPPUNIT_ASSERT_EQUAL (string("AntiKt4Topo"), result.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL (size_t(2), result.bins.size());

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);
    CPPUNIT_ASSERT_EQUAL (size_t(1), b_1.binSpec.size());
    CPPUNIT_ASSERT_EQUAL (size_t(1), b_2.binSpec.size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, b_1.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.75, b_2.centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1/sqrt(2), b_1.centralValueStatisticalError, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1/sqrt(2), b_2.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL(size_t(1), b_1.systematicErrors.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), b_2.systematicErrors.size());
    SystematicError e1 (b_1.systematicErrors[0]);
    SystematicError e2 (b_2.systematicErrors[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, e1.value, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, e2.value, 0.001);
    cout << "Finishing testAnaTwoBinsByBin" << endl;

  }

  void testAnaTrackMetadataByBin()
  {
    // Two analyses, and 2 bins in each.
    // Combine by bin, shoudl ignore any correlation
    // with sys errors. Make sure that meta data flows correclty.

    cout << "Starting testAnaTrackMetadataByBin" << endl;

    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationBin b2;
    b2.centralValue = 0.75;
    b2.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound2;
    bound2.variable = "eta";
    bound2.lowvalue = 2.5;
    bound2.highvalue = 5.0;
    b2.binSpec.push_back(bound2);

    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);
    b2.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);
    ana1.bins.push_back(b2);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";

    ana1.metadata["m1"].push_back(5.0);
    ana1.metadata["m3"].push_back(6.0);
    ana2.metadata["m2"].push_back(7.0);
    ana2.metadata["m3"].push_back(10.0);

    CalibrationInfo inputs;
    inputs.Analyses.push_back (ana1);
    inputs.Analyses.push_back (ana2);

    inputs.CombinationAnalysisName = "combined";

    setupRoo();
    vector<CalibrationAnalysis> results (CombineAnalyses(inputs, true, kCombineBySingleBin));
    CPPUNIT_ASSERT_EQUAL((size_t)1, results.size());
    CalibrationAnalysis &result(results[0]);

    cout << "Result: " << endl;
    cout << result << endl;

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m1 [from s8] [from 0-eta-2.5]"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m3 [from s8] [from 0-eta-2.5]"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m2 [from ptrel] [from 0-eta-2.5]"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m3 [from ptrel] [from 0-eta-2.5]"].size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result.metadata["m1 [from s8] [from 0-eta-2.5]"][0], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result.metadata["m3 [from s8] [from 0-eta-2.5]"][0], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result.metadata["m2 [from ptrel] [from 0-eta-2.5]"][0], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, result.metadata["m3 [from ptrel] [from 0-eta-2.5]"][0], 0.001);
  }

  void testAnaTrackMetadata()
  {
    // Two analyses, and 2 bins in each.
    // Combine by bin, shoudl ignore any correlation
    // with sys errors. Make sure that meta data flows correclty.

    cout << "Starting testAnaTrackMetadataByBin" << endl;

    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationBin b2;
    b2.centralValue = 0.75;
    b2.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound2;
    bound2.variable = "eta";
    bound2.lowvalue = 2.5;
    bound2.highvalue = 5.0;
    b2.binSpec.push_back(bound2);

    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);
    b2.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);
    ana1.bins.push_back(b2);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";

    ana1.metadata["m1"].push_back(5.0);
    ana1.metadata["m3"].push_back(6.0);
    ana2.metadata["m2"].push_back(7.0);
    ana2.metadata["m3"].push_back(10.0);

    CalibrationInfo inputs;
    inputs.Analyses.push_back (ana1);
    inputs.Analyses.push_back (ana2);

    inputs.CombinationAnalysisName = "combined";

    setupRoo();
    vector<CalibrationAnalysis> results (CombineAnalyses(inputs));
    CPPUNIT_ASSERT_EQUAL((size_t)1, results.size());
    CalibrationAnalysis &result(results[0]);

    cout << "Result: " << endl;
    cout << result << endl;

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m1 [from s8]"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m3 [from s8]"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m2 [from ptrel]"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.metadata["m3 [from ptrel]"].size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result.metadata["m1 [from s8]"][0], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.0, result.metadata["m3 [from s8]"][0], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, result.metadata["m2 [from ptrel]"][0], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, result.metadata["m3 [from ptrel]"][0], 0.001);
  }

  void testAnaTwoWeirdBins()
  {
    cout << "Starting fit testAnaTwoWeirdBins" << endl;

    // Two ana, mis-matched bins, some overlap, some not...

    CalibrationBin b1;
    b1.centralValue = 0.0;
    b1.centralValueStatisticalError = 0.1;

    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);

    b1.centralValue = 0.0;
    b1.binSpec[0].lowvalue = 2.5;
    b1.binSpec[0].highvalue = 5.0;
    ana1.bins.push_back(b1);

    ana2.name = "ptrel";
    ana2.bins[0].centralValue = 1.0;
    ana2.bins[0].centralValueStatisticalError = 0.2;

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    cout << "Result of the weird bin test:" << endl;
    cout << result << endl;

    CPPUNIT_ASSERT_EQUAL (size_t(2), result.bins.size());

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, b_1.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, b_2.centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, b_2.centralValueStatisticalError, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0/sqrt(1.0/(0.1*0.1)+1.0/(0.2*0.2)), b_1.centralValueStatisticalError, 0.001);
  }

  void testAnaTwoDifBinsDiffSys()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana1;
    ana1.name = "s8";
    ana1.flavor = "bottom";
    ana1.tagger = "comb";
    ana1.operatingPoint = "0.50";
    ana1.jetAlgorithm = "AntiKt4Topo";
    ana1.bins.push_back(b1);

    CalibrationAnalysis ana2 (ana1);
    ana2.name = "ptrel";
    ana2.bins[0].binSpec[0].lowvalue = 2.5;
    ana2.bins[0].binSpec[0].highvalue = 4.5;
    ana2.bins[0].centralValue = 1.0;
    ana2.bins[0].centralValueStatisticalError = 0.2;
    ana2.bins[0].systematicErrors[0].value = 0.2;
    ana2.bins[0].systematicErrors[0].name = "s2";

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    setupRoo();
    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    cout << "Output from test" << endl << result << endl;

    CPPUNIT_ASSERT_EQUAL (string("combined"), result.name);
    CPPUNIT_ASSERT_EQUAL (string("bottom"), result.flavor);
    CPPUNIT_ASSERT_EQUAL (string("comb"), result.tagger);
    CPPUNIT_ASSERT_EQUAL (string("0.50"), result.operatingPoint);
    CPPUNIT_ASSERT_EQUAL (string("AntiKt4Topo"), result.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL (size_t(2), result.bins.size());

    CalibrationBin b_1 (result.bins[0]);
    CalibrationBin b_2 (result.bins[1]);
    CPPUNIT_ASSERT_EQUAL (size_t(1), b_1.binSpec.size());
    CPPUNIT_ASSERT_EQUAL (size_t(1), b_2.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, b_1.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, b_1.centralValueStatisticalError, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, b_2.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, b_2.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL(size_t(1), b_1.systematicErrors.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), b_2.systematicErrors.size());
    SystematicError e1 (b_1.systematicErrors[0]);
    SystematicError e2 (b_2.systematicErrors[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, e1.value, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, e2.value, 0.01);
  }

  void testAnaDifOne()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana;
    ana.name = "s8";
    ana.flavor = "bottom";
    ana.tagger = "comb";
    ana.operatingPoint = "0.50";
    ana.jetAlgorithm = "AntiKt4Topo";
    ana.bins.push_back(b1);

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana);

    CalibrationInfo info;
    info.Analyses = inputs;

    setupRoo();
    vector<CalibrationAnalysis> results (CombineAnalyses(info));
    CPPUNIT_ASSERT_EQUAL (size_t(0), results.size());
  }

  void testAnaDifOneByBin()
  {
    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana;
    ana.name = "s8";
    ana.flavor = "bottom";
    ana.tagger = "comb";
    ana.operatingPoint = "0.50";
    ana.jetAlgorithm = "AntiKt4Topo";
    ana.bins.push_back(b1);

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana);

    CalibrationInfo info;
    info.Analyses = inputs;

    setupRoo();
    vector<CalibrationAnalysis> results (CombineAnalyses(info, true, kCombineBySingleBin));
    CPPUNIT_ASSERT_EQUAL (size_t(0), results.size());
  }

  void testAnaTwoDifBinsNoFit()
  {
    // Two analyese, different flavors, so they shouldn't be fit together,
    // and thus no fitting should occur (i.e. no results).

    // Single analysis - test simple case!
    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    SystematicError s1;
    s1.name = "s1";
    s1.value = 0.1;
    b1.systematicErrors.push_back(s1);

    CalibrationAnalysis ana;
    ana.name = "s8";
    ana.flavor = "bottom";
    ana.tagger = "comb";
    ana.operatingPoint = "0.50";
    ana.jetAlgorithm = "AntiKt4Topo";
    ana.bins.push_back(b1);

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana);
    ana.flavor = "charm";
    inputs.push_back (ana);

    CalibrationInfo info;
    info.Analyses = inputs;

    setupRoo();
    vector<CalibrationAnalysis> results (CombineAnalyses(info));
    CPPUNIT_ASSERT_EQUAL (size_t(0), results.size());
  }

  CalibrationAnalysis SimpleAna(bool addSys = true)
  {
    CalibrationAnalysis result;

    CalibrationBin b1;
    b1.centralValue = 0.5;
    b1.centralValueStatisticalError = 0.1;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b1.binSpec.push_back(bound);
    if (addSys) {
      SystematicError s1;
      s1.name = "s1";
      s1.value = 0.1;
      b1.systematicErrors.push_back(s1);
    }

    CalibrationAnalysis ana;
    result.name = "s8";
    result.flavor = "bottom";
    result.tagger = "comb";
    result.operatingPoint = "0.50";
    result.jetAlgorithm = "AntiKt4Topo";
    result.bins.push_back(b1);

    return result;
  }

  void rebinEmpty()
  {
    CalibrationAnalysis ana(SimpleAna());
    RebinAnalysis (set<set<CalibrationBinBoundary> > (), ana);
  }

  void rebinToEmpty()
  {
    CalibrationAnalysis ana(SimpleAna());
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));
    RebinAnalysis (atemp, CalibrationAnalysis());
  }

  void rebinMissingBin()
  {
    CalibrationAnalysis ana(SimpleAna());
    ana.bins[0].binSpec[0].highvalue = 5.0; // Now eta is 0 to 5 in one bin
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));
    
    RebinAnalysis (atemp, SimpleAna());
  }

  void rebinOverlappingBin()
  {
    CalibrationAnalysis ana(SimpleAna());
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));

    ana.bins[0].binSpec[0].highvalue = 5.0; // Now eta is 0 to 5 in one bin
    RebinAnalysis (atemp, ana);
  }

  void rebinOneToOne()
  {
    CalibrationAnalysis ana(SimpleAna());
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));
    CalibrationAnalysis result (RebinAnalysis (atemp, ana));

    CPPUNIT_ASSERT_EQUAL (ana, result);
  }

  // Add a bin to an existing analysis.
  void AddBin (CalibrationAnalysis &ana,
	       const string &binCoordName,
	       double low_v, double high_v,
	       double v, double statError) {
    CalibrationBinBoundary bound;
    bound.variable = binCoordName;
    bound.lowvalue = low_v;
    bound.highvalue = high_v;
    CalibrationBin b;
    b.binSpec.push_back(bound);
    b.centralValueStatisticalError = statError;
    b.centralValue = v;
    ana.bins.push_back(b);
  }

  void rebinTwoToOne()
  {
    cout << "rebinToTwoOne" << endl;
    CalibrationAnalysis ana(SimpleAna());
    ana.bins[0].binSpec[0].highvalue = 5.0; // Now eta is 0 to 5 in one bin
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));

    ana = SimpleAna(false);
    AddBin (ana,
	    "eta",
	    2.5, 5.0,
	    0.5, 0.1);

    CalibrationAnalysis result (RebinAnalysis (atemp, ana));

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, result.bins[0].centralValue, 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1/sqrt(2), result.bins[0].centralValueStatisticalError, 0.0001);
  }

  void rebinThreeToOne()
  {
    // Make sure that the low and high vale tests for bins being adjacent works
    // correctly. Logic is a little tricky. :-)
    CalibrationAnalysis ana(SimpleAna());
    ana.bins[0].binSpec[0].highvalue = 5.0; // Now eta is 0 to 5 in one bin
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));

    ana = SimpleAna(false);
    ana.bins[0].binSpec[0].lowvalue = 1.0;
    AddBin (ana,
	    "eta",
	    2.5, 5.0,
	    0.5, 0.1);
    AddBin (ana,
	    "eta",
	    0.0, 1.0,
	    0.5, 0.1);

    CalibrationAnalysis result (RebinAnalysis (atemp, ana));

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, result.bins[0].centralValue, 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1/sqrt(3), result.bins[0].centralValueStatisticalError, 0.0001);
  }

  void rebinThreeToOneWithOverlap()
  {
    // Make sure that the low and high vale tests for bins being adjacent works
    // correctly. Logic is a little tricky. :-)
    CalibrationAnalysis ana(SimpleAna());
    ana.bins[0].binSpec[0].highvalue = 5.0; // Now eta is 0 to 5 in one bin
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));

    ana = SimpleAna(false);
    ana.bins[0].binSpec[0].lowvalue = 1.1;
    ana.bins[0].binSpec[0].highvalue = 2.6;
    AddBin (ana,
	    "eta",
	    2.5, 5.0,
	    0.5, 0.1);
    AddBin (ana,
	    "eta",
	    0.0, 1.0,
	    0.5, 0.1);

    CalibrationAnalysis result (RebinAnalysis (atemp, ana));
  }

  void rebinTwoToOneWithGap()
  {
    // Make sure that the low and high vale tests for bins being adjacent works
    // correctly. Logic is a little tricky. :-)
    CalibrationAnalysis ana(SimpleAna());
    ana.bins[0].binSpec[0].highvalue = 5.0; // Now eta is 0 to 5 in one bin
    set<set<CalibrationBinBoundary> > atemp (listAnalysisBins(ana));

    ana = SimpleAna(false);
    ana.bins.clear();
    AddBin (ana,
	    "eta",
	    2.5, 5.0,
	    0.5, 0.1);
    AddBin (ana,
	    "eta",
	    0.0, 1.0,
	    0.5, 0.1);

    CalibrationAnalysis result (RebinAnalysis (atemp, ana));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinerTest);
