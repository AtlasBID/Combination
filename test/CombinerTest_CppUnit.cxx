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
  //CPPUNIT_TEST ( testOBTwoComplexBinIn );
  //CPPUNIT_TEST ( testOBTwoBinInWithSys );

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
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sqrt(0.1*0.1 + 0.1*0.1), result.centralValueStatisticalError, 0.001);
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinerTest);
