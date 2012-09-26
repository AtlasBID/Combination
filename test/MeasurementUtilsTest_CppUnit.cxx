///
/// CppUnit tests for the measurement object.
///

#include "Combination/MeasurementUtils.h"
#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"

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

class MeasurementUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( MeasurementUtilsTest );

  CPPUNIT_TEST( testCovarM1One );
  CPPUNIT_TEST( testCovarM2One );

  CPPUNIT_TEST( testCovarM1Two );
  CPPUNIT_TEST( testCovarM2Two );

  CPPUNIT_TEST( testCovarM1TwoC );
  CPPUNIT_TEST( testCovarM2TwoC );

  CPPUNIT_TEST_SUITE_END();

  void testCovarM1One()
  {
    // Make sure nothing stupid happens
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m);

    TMatrixTSym<double> r (CalcCovarMatrixUsingRho (mlist));

    CPPUNIT_ASSERT_EQUAL (1, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
  }

  void testCovarM2One()
  {
    // Make sure nothing stupid happens
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m);

    TMatrixTSym<double> r (CalcCovarMatrixUsingComposition (mlist));

    CPPUNIT_ASSERT_EQUAL (1, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
  }

  void testCovarM1Two()
  {
    // Make sure two uncorrelated guys come out ok.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingRho (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r(0,1), 0.01);
  }

  void testCovarM2Two()
  {
    // Make sure two uncorrelated guys come out ok.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingComposition (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r(0,1), 0.01);
  }

  void testCovarM1TwoC()
  {
    // Two guys, with 100% correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e", 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingRho (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(0,1), 0.01);
  }

  void testCovarM2TwoC()
  {
    // Two guys, with 100% correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e", 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingComposition (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(0,1), 0.01);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MeasurementUtilsTest);
