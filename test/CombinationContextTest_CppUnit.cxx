///
/// CppUnit tests for the for the combination context object - which runs everything
///

#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"

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
  CPPUNIT_TEST ( testFitTwoZeroMeasurement );
  CPPUNIT_TEST ( testFitOneNonZeroMeasurement );
  CPPUNIT_TEST ( testFitTwoDataOneMeasurement );

  CPPUNIT_TEST ( testFitOneDataTwoMeasurement );
  CPPUNIT_TEST ( testFitTwoDataTwoMeasurement );

  CPPUNIT_TEST ( testFitOneDataOneMeasurementSys );

  CPPUNIT_TEST_SUITE_END();

  void testCTor()
  {
    CombinationContext *c = new CombinationContext();
    delete c;
  }

  void testAddMeasurement()
  {
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
  }

  void testFitOneMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, fr["average"].centralValue, 0.01);
  }

  void testFitOneZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].statisticalError, 0.01);
  }

  void testFitOneNonZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (5.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["average"].statisticalError, 0.01);
  }

  void testFitTwoZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);

    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["average"].statisticalError, 0.01);
  }

  void testFitTwoDataOneMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);

    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["average"].statisticalError, 0.01);
  }

  void testFitOneDataTwoMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.1);

    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["a2"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);
  }

  void testFitTwoDataTwoMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    c.AddMeasurement ("a2", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.1);

    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a2"].centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a2"].statisticalError, 0.01);
  }

  void testFitOneDataOneMeasurementSys()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.1);
    m->addSystematicAbs("s1", 0.1);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (5.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1*sqrt(2.0), fr["a1"].statisticalError, 0.01);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationContextTest);
