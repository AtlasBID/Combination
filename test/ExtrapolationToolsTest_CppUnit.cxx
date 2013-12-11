///
/// CppUnit tests for the extrapolation tools.
///

#include "Combination/ExtrapolationTools.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>

#include <stdexcept>

using namespace std;
using namespace BTagCombination;

class ExtrapolationToolsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE (ExtrapolationToolsTest );

  CPPUNIT_TEST (testExtrapolate1binPtHigh);
  CPPUNIT_TEST (testExtrapolate1binPtLow);
  CPPUNIT_TEST (testExtrapolateWithMulitpleEtaBins);
  CPPUNIT_TEST (testExtrapolateWithMulitpleEtaBinsWithSingleExtrapolationBin);
  CPPUNIT_TEST (testNoExtrapolation);
  CPPUNIT_TEST (testExtrapolate1binPtHigh2ExtBins);

  CPPUNIT_TEST_EXCEPTION (testExtrapolate1binNegative, runtime_error);
  CPPUNIT_TEST_EXCEPTION (testExtrapolate1binPtAndEta, runtime_error);
  CPPUNIT_TEST_EXCEPTION (testExtrapolate1binPtGap, runtime_error);
  CPPUNIT_TEST_EXCEPTION (testExtrapolate1binPtOverlap, runtime_error);
  CPPUNIT_TEST_EXCEPTION (testExtrapolateTwice, runtime_error);
  CPPUNIT_TEST_EXCEPTION (testExtrapolationWithSingleEtaBinWithMultipleExtrapolation, runtime_error);

  CPPUNIT_TEST_SUITE_END();

  CalibrationAnalysis generate_1bin_ana()
  {
    // Simple 1 bin analysis
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    b1.centralValue = 1.1;
    b1.centralValueStatisticalError = 0.2;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
    b1.binSpec.push_back(bb1);

    SystematicError e;
    e.name = "err";
    e.value = 0.1;
    e.uncorrelated = false;
    b1.systematicErrors.push_back(e);
    
    ana.bins.push_back(b1);
    return ana;
  }

  CalibrationAnalysis generate_2bin_extrap_in_pthigh()
  {
    // Extend the standard 1 bin in pt on the high side
    // Simple 1 bin analysis
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    b1.centralValue = 1.1;
    b1.centralValueStatisticalError = 0.2;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
    b1.binSpec.push_back(bb1);

    SystematicError e;
    e.name = "extr";
    e.value = 0.1;
    e.uncorrelated = false;
    b1.systematicErrors.push_back(e);
    ana.bins.push_back(b1);

    b1.binSpec[0].lowvalue = 100.0;
    b1.binSpec[0].highvalue = 200.0;
    b1.systematicErrors[0].value = 0.2; // x2 error size
    ana.bins.push_back(b1);

    return ana;
  }

  CalibrationAnalysis generate_2bin_extrap_in_pteta()
  {
    // Extend the standard 1 bin in pt on the high side and on the eta side.
    // This is no legal with the current CDI and "what to do" limitations.
    CalibrationAnalysis ana;
    ana.name = "ana1";
    ana.flavor = "bottom";
    ana.tagger = "SV0";
    ana.operatingPoint = "0.1";
    ana.jetAlgorithm = "AntiKt";

    CalibrationBin b1;
    b1.centralValue = 1.1;
    b1.centralValueStatisticalError = 0.2;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 100.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "abseta";
    b1.binSpec.push_back(bb1);

    SystematicError e;
    e.name = "extr";
    e.value = 0.1;
    e.uncorrelated = false;
    b1.systematicErrors.push_back(e);
    ana.bins.push_back(b1);

    b1.binSpec[0].lowvalue = 100.0;
    b1.binSpec[0].highvalue = 200.0;
    b1.systematicErrors[0].value = 0.2; // x2 error size
    ana.bins.push_back(b1);

    b1.binSpec[1].lowvalue = 2.5;
    b1.binSpec[1].highvalue = 4.0;
    b1.systematicErrors[0].value = 0.2; // x2 error size
    ana.bins.push_back(b1);

    return ana;
  }

  // Extend pt on the low side.
  CalibrationAnalysis generate_2bin_extrap_in_ptlow()
  {
    CalibrationAnalysis ana(generate_2bin_extrap_in_pthigh());
    ana.bins[1].binSpec[0].lowvalue = -100.0;
    ana.bins[1].binSpec[0].highvalue = 0.0;
    return ana;
  }

  // Do the pt extrapolation, on the high side.
  void testExtrapolate1binPtHigh()
  {
    cout << "Starting testExtrapolate1binPtHigh()" << endl;
    CalibrationAnalysis ana (generate_1bin_ana());
    CalibrationAnalysis extrap (generate_2bin_extrap_in_pthigh());

    CalibrationAnalysis result (addExtrapolation(extrap, ana));

    CPPUNIT_ASSERT_EQUAL(size_t(2), result.bins.size());
    CPPUNIT_ASSERT (result.bins[1].isExtended);

    // First bin error should remain untouched
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.bins[0].systematicErrors.size());
    SystematicError e1(result.bins[0].systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("err"), e1.name);
    CPPUNIT_ASSERT_EQUAL(double(0.1), e1.value);

    // Extrapolated bins have only one error
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.bins[1].systematicErrors.size());
    SystematicError e2(result.bins[1].systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("extrapolated"), e2.name);

    // Doubles in size from the first one
    CPPUNIT_ASSERT_EQUAL(double(0.2), e2.value);
  }

  // Do the extrapolation on the low side
  void testExtrapolate1binPtLow()
  {
    CalibrationAnalysis ana (generate_1bin_ana());
    CalibrationAnalysis extrap (generate_2bin_extrap_in_ptlow());

    CalibrationAnalysis result (addExtrapolation(extrap, ana));

    CPPUNIT_ASSERT_EQUAL(size_t(2), result.bins.size());
    CPPUNIT_ASSERT (result.bins[1].isExtended);

    // First bin error should remain untouched
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.bins[0].systematicErrors.size());
    SystematicError e1(result.bins[0].systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("err"), e1.name);
    CPPUNIT_ASSERT_EQUAL(double(0.1), e1.value);

    // Extrapolated bins have only one error
    CPPUNIT_ASSERT_EQUAL(size_t(1), result.bins[1].systematicErrors.size());
    SystematicError e2(result.bins[1].systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("extrapolated"), e2.name);

    // Doubles in size from the first one
    CPPUNIT_ASSERT_EQUAL(double(0.2), e2.value);
  }

  // 1 bin analysis, extended on the pt side, but the sys error calls for shrinking
  // (throw)
  void testExtrapolate1binNegative()
  {
    CalibrationAnalysis ana (generate_1bin_ana());
    CalibrationAnalysis extrap (generate_2bin_extrap_in_pthigh());
    extrap.bins[1].systematicErrors[0].value = 0.05;

    CalibrationAnalysis result (addExtrapolation(extrap, ana));
  }

  // 1 bin analysis, extended on the pt side, and eta size (should throw b.c. we
  // don't know how to deal with multiple axis extensions.
  void testExtrapolate1binPtAndEta()
  {
    CalibrationAnalysis ana (generate_1bin_ana());
    CalibrationAnalysis extrap (generate_2bin_extrap_in_pteta());

    CalibrationAnalysis result (addExtrapolation(extrap, ana));
  }

  // There is a gap between the last real bin and the first extrapolated bin.
  void testExtrapolate1binPtGap()
  {
    CalibrationAnalysis ana (generate_1bin_ana());
    CalibrationAnalysis extrap (generate_2bin_extrap_in_pthigh());
    extrap.bins[1].binSpec[0].lowvalue = 105;

    CalibrationAnalysis result (addExtrapolation(extrap, ana));
  }

  // The last good bin and an extrapolated bin overlap in boundaries.
  void testExtrapolate1binPtOverlap()
  {
    CalibrationAnalysis ana (generate_1bin_ana());
    CalibrationAnalysis extrap (generate_2bin_extrap_in_pthigh());
    extrap.bins[1].binSpec[0].lowvalue = 95;

    CalibrationAnalysis result (addExtrapolation(extrap, ana));
  }

  // If this guy already has extrapolations applied to it, then fail.
  void testExtrapolateTwice()
  {
  }

  // If the bins match exactly, there is no extrapolation. Shouldn't harm
  // anything.
  void testNoExtrapolation()
  {
    CPPUNIT_ASSERT (false);
  }

  // Make sure that extrapolation happens when we have two bins in eta the match our
  // extrapolation.
  void testExtrapolateWithMulitpleEtaBins()
  {
    CPPUNIT_ASSERT (false);
  }

  // The analysis has multiple bins in eta, but the extrapolation has only
  // a single bin in eta.
  void testExtrapolateWithMulitpleEtaBinsWithSingleExtrapolationBin ()
  {
    CPPUNIT_ASSERT (false);
  }

  // Single bin in eta, but the extrapolation has multiple. Since the
  // multiple will not match exactly, this should fail.
  void testExtrapolationWithSingleEtaBinWithMultipleExtrapolation()
  {
    CPPUNIT_ASSERT (false);
  }

  // Make sure if there are multiple extrapolation bins that overlap we don't really
  // care as long as the last one overlaps with ana before the jump off point.
  void testExtrapolate1binPtHigh2ExtBins()
  {
    CPPUNIT_ASSERT (false);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ExtrapolationToolsTest);
