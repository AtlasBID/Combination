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

  CPPUNIT_TEST( testCovarM1TwoNegC );
  CPPUNIT_TEST( testCovarM2TwoNegC );

  CPPUNIT_TEST( testCovarM1TwoNeg2C );
  CPPUNIT_TEST( testCovarM2TwoNeg2C );

  CPPUNIT_TEST( testCovarM1TwoCPartial );
  CPPUNIT_TEST( testCovarM2TwoCPartial );

  CPPUNIT_TEST( testCovarM1TwoNegCPartial );
  CPPUNIT_TEST( testCovarM2TwoNegCPartial );

  CPPUNIT_TEST(calcChi2Nothing);
  CPPUNIT_TEST(calcChi2OnlyMeasurment);
  CPPUNIT_TEST(calcChi2OnlyFitResults);
  CPPUNIT_TEST(calcChi2FitNamesDontMatch);
  CPPUNIT_TEST(calcChi2FitNamedTwice);
  CPPUNIT_TEST(calcChi2SameFitAndMeasurementSame);
  CPPUNIT_TEST(calcChi2TwoFitAndMeasurmentsOneOff);
  CPPUNIT_TEST(calcChi2SameFitAndMeasurementWithSys);
  CPPUNIT_TEST(calcChi2TwoMeasurementsOffBySys);

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

  void testCovarM1TwoNegC()
  {
    // Two guys, with 100% anti-correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e", -0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingRho (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(0,1), 0.01);
  }

  void testCovarM2TwoNegC()
  {
    // Two guys, with 100% anti-correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e", -0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingComposition (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(0,1), 0.01);
  }

  void testCovarM1TwoNeg2C()
  {
    // Two guys, with 100% anti-correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e", -0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e", -0.25);

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

  void testCovarM2TwoNeg2C()
  {
    // Two guys, with 100% anti-correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e", -0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e", -0.25);

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

  void testCovarM1TwoCPartial()
  {
    // Two guys, with 50% correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e1", 0.5);
    m1->addSystematicAbs("e2", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e1", 0.25);
    m2->addSystematicAbs("e3", 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingRho (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5*2.0, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25*2.0, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(0,1), 0.01);
  }

  void testCovarM2TwoCPartial()
  {
    // Two guys, with 50% correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e1", 0.5);
    m1->addSystematicAbs("e2", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e1", 0.25);
    m2->addSystematicAbs("e3", 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingComposition (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5*2.0, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25*2.0, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.25, r(0,1), 0.01);
  }

  void testCovarM1TwoNegCPartial()
  {
    // Two guys, with 50% negative correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e1", 0.5);
    m1->addSystematicAbs("e2", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e1", -0.25);
    m2->addSystematicAbs("e3", 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingRho (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5*2.0, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25*2.0, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(0,1), 0.01);
  }

  void testCovarM2TwoNegCPartial()
  {
    // Two guys, with 50% negative correlation.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m1->addSystematicAbs("e1", 0.5);
    m1->addSystematicAbs("e2", 0.5);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.0);
    m2->addSystematicAbs("e1", -0.25);
    m2->addSystematicAbs("e3", 0.25);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);

    TMatrixTSym<double> r (CalcCovarMatrixUsingComposition (mlist));

    CPPUNIT_ASSERT_EQUAL (2, r.GetNrows());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5*0.5*2.0, r(0,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25*0.25*2.0, r(1,1), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(1,0), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5*0.25, r(0,1), 0.01);
  }

  void calcChi2Nothing()
  {
    vector<Measurement*> mlist;
    double r(CalcChi2(mlist, mlist));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, r, 0.01);
  }

  void calcChi2OnlyMeasurment()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement("average", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m);
    vector<Measurement*> flist;

    CPPUNIT_ASSERT_THROW(CalcChi2(mlist, flist), std::runtime_error);
  }

  void calcChi2OnlyFitResults()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement("average", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    vector<Measurement*> flist;
    flist.push_back(m);

    CPPUNIT_ASSERT_THROW(CalcChi2(mlist, flist), std::runtime_error);
  }

  void calcChi2FitNamesDontMatch()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    Measurement *mf = c.AddMeasurement("v2", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m);
    vector<Measurement*> flist;
    flist.push_back(mf);

    CPPUNIT_ASSERT_THROW(CalcChi2(mlist, flist), std::runtime_error);
  }

  void calcChi2FitNamedTwice() {
    CombinationContext c;
    Measurement *m = c.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    Measurement *mf1 = c.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    Measurement *mf2 = c.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m);
    vector<Measurement*> flist;
    flist.push_back(mf1);
    flist.push_back(mf2);

    CPPUNIT_ASSERT_THROW(CalcChi2(mlist, flist), std::runtime_error);
  }

  void calcChi2SameFitAndMeasurementSame()
  {
    cout << "Starting calcChi2SameFitAndMeasurementSame" << endl;
    CombinationContext c;
    Measurement *m = c.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m);
    vector<Measurement*> flist;
    flist.push_back(m);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, CalcChi2(mlist, flist), 0.01);
  }

  void calcChi2TwoFitAndMeasurmentsOneOff()
  {
    cout << "Starting calcChi2TwoFitAndMeasurmentsOneOff" << endl;
    CombinationContext cm;
    Measurement *m1 = cm.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    Measurement *m2 = cm.AddMeasurement("v2", -10.0, 10.0, 7.0, 0.5);

    CombinationContext cf;
    Measurement *f1 = cm.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    Measurement *f2 = cm.AddMeasurement("v2", -10.0, 10.0, 7.5, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);
    vector<Measurement*> flist;
    flist.push_back(f1);
    flist.push_back(f2);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, CalcChi2(mlist, flist), 0.01);
  }

  void calcChi2SameFitAndMeasurementWithSys()
  {
    CombinationContext c;
    Measurement *m = c.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    m->addSystematicAbs("dork", 0.1);

    vector<Measurement*> mlist;
    mlist.push_back(m);
    vector<Measurement*> flist;
    flist.push_back(m);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, CalcChi2(mlist, flist), 0.01);
  }

  void calcChi2TwoMeasurementsOffBySys()
  {
    cout << "Starting calcChi2TwoMeasurementsOffBySys" << endl;
    CombinationContext cm;
    Measurement *m1 = cm.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);
    m1->addSystematicAbs("sys1", 0.1);
    Measurement *m2 = cm.AddMeasurement("v1", -10.0, 10.0, 5.1, 0.5);
    m2->addSystematicAbs("sys1", 0.1);

    CombinationContext cf;
    Measurement *f1 = cm.AddMeasurement("v1", -10.0, 10.0, 5.0, 0.5);

    vector<Measurement*> mlist;
    mlist.push_back(m1);
    mlist.push_back(m2);
    vector<Measurement*> flist;
    flist.push_back(f1);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1*0.1/0.5/0.5, CalcChi2(mlist, flist), 0.01);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MeasurementUtilsTest);

#ifdef ROOTCORE
// The common atlas test driver
#include <TestPolicy/CppUnit_testdriver.cxx>
#endif
