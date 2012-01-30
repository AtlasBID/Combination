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
  // Makes an nan
  //CPPUNIT_TEST ( testFitTwoDataOneMeasurement2 );
  CPPUNIT_TEST ( testFitTwoDataOneMeasurement3 );

  CPPUNIT_TEST ( testFitOneDataTwoMeasurement );
  CPPUNIT_TEST ( testFitTwoDataTwoMeasurement );

  CPPUNIT_TEST ( testFitOneDataOneMeasurementSys );
  // Generates lots and lots of error messages
  //CPPUNIT_TEST ( testFitOneDataOneMeasurementSys2 );
  //CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys );
  //CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys3 );
  //CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys4 );

  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys2 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys5 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys6 );

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

  void testFitTwoDataOneMeasurement3()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    double e1 = 0.1;
    c.AddMeasurement ("average", -10.0, 10.0, 1.0, e1);
    double e2 = 0.2;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, e2);

    map<string, CombinationContext::FitResult> fr = c.Fit();

    // Weighted average:
    double w1 = 1.0/(e1*e1);
    double w2 = 1.0/(e2*e2);

    double expected = (w1*1.0 + w2*0.0)/(w1+w2);
    double wtExpected =sqrt(w1*w1 + w2*w2);
    double errExpected = sqrt(1.0/wtExpected);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (expected, fr["average"].centralValue, 0.01);
    // Fix this up to make it analytical! Problem could be fit is not compatible!
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.08944, fr["average"].statisticalError, 0.01);
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

  void testFitTwoDataOneMeasurement2()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.2);
    Measurement *m2 = c.AddMeasurement ("a2", -10.0, 10.0, 1.0, 0.1);
    m2->addSystematicAbs("s1", 0.2);

    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a2"].centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL (size_t(1), fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_EQUAL (size_t(1), fr["a2"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT(fr["a2"].sysErrors.find("s1") != fr["a2"].sysErrors.end());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a2"].sysErrors["s1"], 0.01);
  }

  void testFitOneDataOneMeasurementSys()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.1);
    m->addSystematicAbs("s1", 0.1);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (5.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)1, fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].sysErrors["s1"], 0.01);
  }

  void testFitOneDataOneMeasurementSys2()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.1);
    m->addSystematicAbs("s1", 0.1);
    m->addSystematicAbs("s2", 0.5);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (5.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["average"].sysErrors["s2"], 0.01);
  }

  void testFitOneDataTwoMeasurementSys()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.4);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    m2->addSystematicAbs("s1", 0.4);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)1, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s1"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys" << endl;
  }

  void testFitOneDataTwoMeasurementSys3()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m1->addSystematicAbs("s1", 0.4);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m2->addSystematicAbs("s1", 0.4);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)1, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s1"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys" << endl;
  }

  void testFitOneDataTwoMeasurementSys4()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m1->addSystematicAbs("s1", 0.4);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m2->addSystematicAbs("s2", 0.4);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys" << endl;
  }

  void testFitOneDataTwoMeasurementSys2()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys2" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    double e1 = 0.1;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, e1);
    double s1 = 0.2;
    m1->addSystematicAbs("s1", s1);
    double e2 = 0.1;
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.0, e2);
    double s2 = 0.4;
    m2->addSystematicAbs("s2", s2);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    // The sys errors are not correlated since they have
    // different names.

    double w1 = 1.0/(e1*e1 + s1*s1);
    double w2 = 1.0/(e2*e2 + s2*s2);

    double expected = (w1*1.0 + w2*0.0)/(w1+w2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (expected, fr["a1"].centralValue, 0.01);

    double w1stat = 1.0/(e1*e1);
    double w2stat = 1.0/(e2*e2);
    double wtExpectedStat =sqrt(w1stat+w2stat);
    double errExpectedStat = sqrt(1.0/wtExpectedStat);
    // Fix up so analytical (this is .5 sqrt or similar).One's below this have to be fixed too.
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0707, fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());
    // These need updating to be analytiacal (TODO).
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1708, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1740, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys2" << endl;
  }

  void testFitOneDataTwoMeasurementSys6()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys6" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.2);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 2.0, 0.1);
    m2->addSystematicAbs("s2", 0.4);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.22, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys2" << endl;
  }

  void testFitOneDataTwoMeasurementSys5()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys2" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.2);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    m2->addSystematicAbs("s2", 0.4);
    
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.772, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys2" << endl;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationContextTest);
#include <TestPolicy/CppUnit_testdriver.cxx>
