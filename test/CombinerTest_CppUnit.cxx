///
/// CppUnit tests for the parser
///
///  Some pretty simple combinations - we know the results for these
/// cases, so make sure the code works as expected
///

#include "Combination/Combiner.h"

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
  CPPUNIT_TEST ( testOBTwoBinIn );
  CPPUNIT_TEST ( testOBTwoBinInWithSys );

  CPPUNIT_TEST ( testAnaOne );
  CPPUNIT_TEST ( testAnaTwoDifBins );
  //CPPUNIT_TEST_EXCPETION ( testAnaTwoBinsBad1, std::runtime_error );
  CPPUNIT_TEST ( testAnaTwoSamBins );

  CPPUNIT_TEST_SUITE_END();

  void testOBZeroBinIn()
  {
    // Fail if no bins input - nothing to combine!
    vector<CalibrationBin> bins;
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
    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.centralValue == 0.5);
    CPPUNIT_ASSERT(result.centralValueStatisticalError == 0.1);
    CPPUNIT_ASSERT(result.binSpec.size() == 1);
    CPPUNIT_ASSERT(result.binSpec[0].variable == "eta");
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
    b1.systematicErrors.push_back(s1);
    
    vector<CalibrationBin> bins;
    bins.push_back(b1);
    bins.push_back(b1);

    CalibrationBin result = CombineBin(bins);

    CPPUNIT_ASSERT(result.binSpec.size() == 1);
    CPPUNIT_ASSERT(result.binSpec[0].variable == "eta");

    CPPUNIT_ASSERT(result.systematicErrors.size() == 1);
    SystematicError s1r (result.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL (string("s1"), s1r.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, s1r.value, 0.01);

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
    ana2.bins[0].binSpec[0].highvalue = 2.5;

    vector<CalibrationAnalysis> inputs;
    inputs.push_back (ana1);
    inputs.push_back (ana2);

    CalibrationAnalysis result (CombineSimilarAnalyses(inputs));
    CPPUNIT_ASSERT_EQUAL (string("combined"), result.name);
    CPPUNIT_ASSERT_EQUAL (string("bottom"), result.flavor);
    CPPUNIT_ASSERT_EQUAL (string("comb"), result.tagger);
    CPPUNIT_ASSERT_EQUAL (string("0.50"), result.operatingPoint);
    CPPUNIT_ASSERT_EQUAL (string("AntiKt4Topo"), result.jetAlgorithm);
    CPPUNIT_ASSERT_EQUAL (size_t(2), result.bins.size());
    CalibrationBin b (result.bins[0]);
    CPPUNIT_ASSERT_EQUAL (size_t(1), b.binSpec.size());

    // We know a bit of how the code is tructured - if the bin came over, everything under it
    // came over otherwise other tests in this file would have failed.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, b.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, b.centralValueStatisticalError, 0.001);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinerTest);
