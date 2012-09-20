///
/// CppUnit tests for the measurement object.
///

#include "Combination/Measurement.h"
#include "Combination/CombinationContext.h"

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

class MeasurementTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( MeasurementTest );

  CPPUNIT_TEST( testCovarSelf );
  CPPUNIT_TEST( testCovarNoShare );
  CPPUNIT_TEST( testCovarAllShare );
  CPPUNIT_TEST( testCovarHalfShare );
  CPPUNIT_TEST( testCovarBustedShare );

  CPPUNIT_TEST( testSharedErrorIsWhole );
  CPPUNIT_TEST( testSharedErrorIsNoCommon );
  CPPUNIT_TEST( testSharedErrorIsCommon );

  CPPUNIT_TEST_SUITE_END();

  void testCovarSelf()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    double covar = m->Covar(m);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, covar, 0.01);
  }

  void testCovarNoShare()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.5);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.5);
    double covar = m1->Covar(m2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, covar, 0.01);
  }

  void testCovarAllShare()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.0);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("s1", 0.5);
    m2->addSystematicAbs("s1", 0.5);
    double covar = m1->Covar(m2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, covar, 0.01);
  }

  void testCovarHalfShare()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.0);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("s1", 0.5);
    m1->addSystematicAbs("s2", 0.5);
    m2->addSystematicAbs("s1", 0.5);
    m2->addSystematicAbs("s3", 0.5);
    double covar = m1->Covar(m2);
    double s = sqrt(0.5*0.5+0.5*0.5);
    double rho = 0.5;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(s*s*rho, covar, 0.01);
  }

  void testCovarBustedShare()
  {
    // Actual bug seen while running
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.0);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("s1", 0.1);
    m2->addSystematicAbs("s1", 0.2);

    double covar1 = m1->Covar(m2);
    double covar2 = m2->Covar(m1);

    // They should be symmetric!
    CPPUNIT_ASSERT_DOUBLES_EQUAL(covar1, covar2, 0.01);
  }

  void testSharedErrorIsWhole()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.5);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.5);
    pair<double, double> r = m1->SharedError(m2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, r.first, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r.second, 0.01);

  }

  void testSharedErrorIsNoCommon()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.5);
    m1->addSystematicAbs ("e1", 0.5);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.5);
    m2->addSystematicAbs ("e2", 0.5);

    pair<double, double> r = m1->SharedError(m2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*sqrt(2), r.first, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r.second, 0.01);
  }

  void testSharedErrorIsCommon()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average1", -10.0, 10.0, 5.0, 0.5);
    m1->addSystematicAbs ("e1", 0.5);
    Measurement *m2 = c.AddMeasurement ("average2", -10.0, 10.0, 5.0, 0.5);
    m2->addSystematicAbs ("e2", 0.5);

    pair<double, double> r = m1->SharedError(m2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*sqrt(2), r.first, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r.second, 0.01);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MeasurementTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
