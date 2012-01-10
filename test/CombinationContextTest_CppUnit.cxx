///
/// CppUnit tests for the for the combination context object - which runs everything
///

#include "Combination/CombinationContext.h"

#include <RooRealVar.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>

#include <stdexcept>
#include <cmath>

using namespace std;
using namespace BTagCombination;

class CombinationContextTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CombinationContextTest );

  CPPUNIT_TEST ( testCTor );
  CPPUNIT_TEST ( testAddMeasurement );
  CPPUNIT_TEST ( testFitOneZeroMeasurement );
  CPPUNIT_TEST ( testFitOneNonZeroMeasurement );

  CPPUNIT_TEST_SUITE_END();

  void testCTor()
  {
    CombinationContext *c = new CombinationContext();
    delete c;
  }

  void testAddMeasurement()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
  }

  void testFitOneMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
    c.Fit();
    RooRealVar result = c.GetFitValue("average");

    CPPUNIT_ASSERT(result.getVal() == 0.0);
  }

  void testFitOneZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
    c.Fit();
    RooRealVar result = c.GetFitValue("average");

    CPPUNIT_ASSERT(result.getVal() == 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, result.getError(), 0.01);
  }

  void testFitOneNonZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    c.Fit();
    RooRealVar result = c.GetFitValue("average");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, result.getVal(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, result.getError(), 0.01);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationContextTest);
