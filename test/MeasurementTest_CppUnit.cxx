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

  CPPUNIT_TEST_SUITE_END();

  void testCovarSelf()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    double covar = m->Covar(m);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, covar, 0.01);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MeasurementTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
