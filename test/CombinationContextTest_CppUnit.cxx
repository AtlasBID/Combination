///
/// CppUnit tests for the for the combination context object - which runs everything
///

#include "Combination/CombinationContext.h"
#include "Combination/Measurement.h"

#include <RooRealVar.h>
#include <RooMsgService.h>

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
  CPPUNIT_TEST ( testFitTwoDataOneMeasurement2 );
  CPPUNIT_TEST ( testFitTwoDataOneMeasurement3 );

  CPPUNIT_TEST ( testFitTwoDataOneMeasurement4 );
  CPPUNIT_TEST ( testFitTwoDataOneMeasurementSys );

  CPPUNIT_TEST ( testFitTwoDataTwoMeasurement );
  CPPUNIT_TEST ( testFitTwoDataOneMeasurementNoUse );

  CPPUNIT_TEST ( testFitOneDataTwoMeasurement );

  CPPUNIT_TEST ( testFitOneDataOneMeasurementSys );
  CPPUNIT_TEST ( testFitOneDataOneMeasurementSys2 );
  CPPUNIT_TEST ( testFitOneDataOneMeasurementSys3 );

  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys2 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys3 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys4 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys5 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys6 );
  CPPUNIT_TEST ( testFitOneDataTwoMeasurementSys7 );

  CPPUNIT_TEST ( testFitWeirdMatches );
  CPPUNIT_TEST ( testFitWeirdMatches2 );

  CPPUNIT_TEST ( testFitCorrelatedResults );
  CPPUNIT_TEST ( testFitCorrelatedResults2 );
  CPPUNIT_TEST ( testFitCorrelatedResults3 );
  CPPUNIT_TEST ( testFitCorrelatedResults4 );
  CPPUNIT_TEST ( testFitCorrelatedResults5 );
  CPPUNIT_TEST ( testFitCorrelatedResults6 );

  CPPUNIT_TEST ( testFitParameterNameLength );
  CPPUNIT_TEST_EXCEPTION ( testFitParameterNameLength1, std::runtime_error );

  CPPUNIT_TEST ( testMeasurementSharedError );
  CPPUNIT_TEST ( testMeasurementSharedError2 );
  CPPUNIT_TEST ( testMeasurementSharedError3 );

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

  void setupRoo()
  {
    RooMsgService::instance().setSilentMode(true);
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  }

  void setupRooTurnOn()
  {
    RooMsgService::instance().setSilentMode(false);
    RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
  }

  void testFitOneMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, fr["average"].centralValue, 0.01);
  }

  void testFitOneZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].statisticalError, 0.01);
  }

  void testFitOneNonZeroMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.5);
    setupRoo();
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

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["average"].statisticalError, 0.01);
  }

  void testFitWeirdMatches()
  {
    cout << "Starting testFitWeirdMatches" << endl;

    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.2);
    c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.1);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["a2"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0/sqrt(1.0/(0.1*0.1)+1.0/(0.2*0.2)), fr["a1"].statisticalError, 0.01);
  }

  void testFitWeirdMatches2()
  {
    cout << "Starting testFitWeirdMatches2" << endl;

    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    m1->addSystematicAbs("s1", 0.1);

    m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s2", 0.1);

    m1 = c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.1);
    m1->addSystematicAbs("s1", 0.1);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();
    cout << "Fit result for a1:" << fr["a1"] << endl;
    cout << "Fit result for a2:" << fr["a2"] << endl;

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["a2"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0/sqrt(1.0/(0.1*0.1)+1.0/(0.1*0.1)), fr["a1"].statisticalError, 0.01);
  }

  void testFitTwoDataOneMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("average", -10.0, 10.0, 0.0, 0.1);

    setupRoo();
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

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    // Weighted average:
    double w1 = 1.0/(e1*e1);
    double w2 = 1.0/(e2*e2);

    double expected = (w1*1.0 + w2*0.0)/(w1+w2);
    //double wtExpected =sqrt(w1*w1 + w2*w2);
    //double errExpected = sqrt(1.0/wtExpected);

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

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["a2"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);
  }

  void testFitTwoDataOneMeasurementNoUse()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    Measurement *m = c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    m->setDoNotUse(true);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);
  }

  void testFitTwoDataTwoMeasurement()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    c.AddMeasurement ("a2", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.1);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a2"].centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a2"].statisticalError, 0.01);
  }

  void testFitTwoDataOneMeasurement4()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.1);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["a2"].centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);
  }

  void DumpFitResult (const CombinationContext::FitResult &f)
  {
    cout << "Central value " << f.centralValue
	 << " +- " << f.statisticalError << endl;
    for (map<string, double>::const_iterator itr = f.sysErrors.begin(); itr != f.sysErrors.end(); itr++) {
      cout << "  Sys " << itr->first << " +- " << itr->second << endl;
    }
  }

  void testFitTwoDataOneMeasurementSys()
  {
    // Two different measurements.
    // With two unconnected systematic errors.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement ("a2", -10.0, 10.0, 0.0, 0.2);

    m1->addSystematicAbs("s1", 0.2);
    m2->addSystematicAbs("s2", 0.4);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0, fr["a2"].centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a2"].statisticalError, 0.01);

    //DumpFitResult (fr["a1"]);
    //DumpFitResult (fr["a2"]);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a1"].sysErrors["s1"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a2"].sysErrors["s2"], 0.001);

    CPPUNIT_ASSERT_EQUAL (size_t(1), fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_EQUAL (size_t(1), fr["a2"].sysErrors.size());

  }

  void testFitTwoDataOneMeasurement2()
  {
    // Two measurements, common systematic error.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.2);
    Measurement *m2 = c.AddMeasurement ("a2", -10.0, 10.0, 1.0, 0.1);
    m2->addSystematicAbs("s1", 0.2);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["a2"].centralValue, 0.01);

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a1"].statisticalError, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["a2"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL (size_t(1), fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_EQUAL (size_t(1), fr["a2"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT(fr["a2"].sysErrors.find("s1") != fr["a2"].sysErrors.end());
    // This one comes back nan
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2, fr["a2"].sysErrors["s1"], 0.01);
  }

  void testFitOneDataOneMeasurementSys()
  {
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 5.0, 0.1);
    m->addSystematicAbs("s1", 0.1);
    
    setupRoo();
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

  void testFitOneDataOneMeasurementSys3()
  {
    cout << "Starting testFitOneDataOneMeasurementSys3" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m = c.AddMeasurement ("average", -10.0, 10.0, 2.0, 0.4);
    m->addSystematicAbs("s1", 0.5748);
    m->addSystematicAbs("s2", 0.7322);
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (2.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["average"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (2.0*0.2874, fr["average"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (2.0*0.3661, fr["average"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataOneMeasurementSys3" << endl;
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
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)1, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s1"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys" << endl;
  }

  void testFitOneDataTwoMeasurementSys3()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys3" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m1->addSystematicAbs("s1", 0.4);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m2->addSystematicAbs("s1", 0.4);
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)1, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.4, fr["a1"].sysErrors["s1"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys3" << endl;
  }

  void testFitOneDataTwoMeasurementSys4()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys4" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m1->addSystematicAbs("s1", 0.4);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.5, 0.1);
    m2->addSystematicAbs("s2", 0.4);
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.5, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2748, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.2748, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSy4" << endl;
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
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    // The sys errors are not correlated since they have
    // different names.

    double w1 = 1.0/(e1*e1 + s1*s1);
    double w2 = 1.0/(e2*e2 + s2*s2);

    double expected = (w1*1.0 + w2*0.0)/(w1+w2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (expected, fr["a1"].centralValue, 0.01);

    //double w1stat = 1.0/(e1*e1);
    //double w2stat = 1.0/(e2*e2);
    //double wtExpectedStat =sqrt(w1stat+w2stat);
    //double errExpectedStat = sqrt(1.0/wtExpectedStat);
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
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.22, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1708, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.174, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys2" << endl;
  }

  void testFitOneDataTwoMeasurementSys7()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys7" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.00631882);
    m1->addSystematicAbs("s1", 0.0998002);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 2.0, 0.00631882);
    m2->addSystematicAbs("s1", 0.0495991);
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (2.0, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.00631882, fr["a1"].statisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, fr["a1"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0495991, fr["a1"].sysErrors["s1"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys7" << endl;
  }

  void testFitOneDataTwoMeasurementSys5()
  {
    cout << "Starting testFitOneDataTwoMeasurementSys5" << endl;
    // Garbage in, garbage out.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.2);
    Measurement *m2 = c.AddMeasurement ("a1", -10.0, 10.0, 0.0, 0.1);
    m2->addSystematicAbs("s2", 0.4);
    
    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.772, fr["a1"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (sqrt(0.1*0.1/2.0), fr["a1"].statisticalError, 0.01);

    CPPUNIT_ASSERT_EQUAL((size_t)2, fr["a1"].sysErrors.size());

    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1708, fr["a1"].sysErrors["s1"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.174, fr["a1"].sysErrors["s2"], 0.01);
    cout << "Finishing testFitOneDataTwoMeasurementSys5" << endl;
  }

  void testFitCorrelatedResults()
  {
    // one data pont, two measurements, with their statistical error 100% correlated.
    cout << "Starting test testFitCorrelatedResults" << endl;
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddCorrelation ("statistical", m1, m2, 0);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["average"].centralValue, 0.01);
    // Proper stat error calculated using the cov matrix method (see notes).
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0707107, fr["average"].statisticalError, 0.01);
    cout << "Starting test testFitCorrelatedResults" << endl;
  }

  void testFitCorrelatedResults1()
  {
    // one data pont, two measurements, with their statistical error 100% correlated.
    cout << "Starting test testFitCorrelatedResults" << endl;
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddCorrelation ("statistical", m1, m2, 0.25);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["average"].centralValue, 0.01);
    // Proper stat error calculated using the cov matrix method (see notes).
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0790569, fr["average"].statisticalError, 0.01);
    cout << "Starting test testFitCorrelatedResults" << endl;
  }

  void testFitCorrelatedResults2()
  {
    // one data pont, two measurements, with their statistical error 100% correlated.
    cout << "Starting test testFitCorrelatedResults" << endl;
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddCorrelation ("statistical", m1, m2, 0.50);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["average"].centralValue, 0.01);
    // Proper stat error calculated using the cov matrix method (see notes).
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0866025, fr["average"].statisticalError, 0.01);
    cout << "Starting test testFitCorrelatedResults" << endl;
  }

  void testFitCorrelatedResults3()
  {
    // one data pont, two measurements, with their statistical error 100% correlated.
    cout << "Starting test testFitCorrelatedResults" << endl;
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddCorrelation ("statistical", m1, m2, 0.75);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["average"].centralValue, 0.01);
    // Proper stat error calculated using the cov matrix method (see notes).
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0935414, fr["average"].statisticalError, 0.01);
    cout << "Starting test testFitCorrelatedResults" << endl;
  }

  void testFitCorrelatedResults4()
  {
    // one data pont, two measurements, with their statistical error 0% correlated.
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 1.0, 0.1);
    c.AddCorrelation ("statistical", m1, m2, 1.0);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, fr["average"].centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.1, fr["average"].statisticalError, 0.01);
  }

  void testFitCorrelatedResults5()
  {
    // This comes from the actual data, and the result was making
    // no sense.

    // New protection code should now bounce this - and take only the best of the
    // two measurements.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 0.8789, 0.0243);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 0.9334, 0.1322);
    c.AddCorrelation ("statistical", m1, m2, 0.717724);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.8789, fr["average"].centralValue, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0243, fr["average"].statisticalError, 0.001);
  }

  void testFitCorrelatedResults6()
  {
    // This comes from the actual data, and the result was making
    // no sense.

    CombinationContext c;
    Measurement *m1 = c.AddMeasurement ("average", -10.0, 10.0, 0.8789, 0.01);
    Measurement *m2 = c.AddMeasurement ("average", -10.0, 10.0, 0.9334, 0.1);
    c.AddCorrelation ("statistical", m1, m2, 0.8);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.8789, fr["average"].centralValue, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.01, fr["average"].statisticalError, 0.001);
  }

  void testFitParameterNameLength()
  {
    // This comes from the actual data, and the result was making
    // no sense.

    CombinationContext c;
    string longname;
    for (int i = 0; i < 70; i++){
      longname += "h";
    }

    Measurement *m1 = c.AddMeasurement (longname, "average", -10.0, 10.0, 0.985215, 0.0351802);
    longname += "1";
    Measurement *m2 = c.AddMeasurement (longname, "average", -10.0, 10.0, 0.9367, 0.0178);
    c.AddCorrelation ("statistical", m1, m2, 0.496615);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    //CPPUNIT_ASSERT_DOUBLES_EQUAL (0.871953, fr["average"].centralValue, 0.001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0192838, fr["average"].statisticalError, 0.001);
  }

  void testFitParameterNameLength1()
  {
    // This comes from the actual data, and the result was making
    // no sense.

    CombinationContext c;
    string longname;
    for (int i = 0; i < 100; i++){
      longname += "h";
    }

    Measurement *m1 = c.AddMeasurement (longname, "average", -10.0, 10.0, 0.985215, 0.0351802);
    longname += "1";
    Measurement *m2 = c.AddMeasurement (longname, "average", -10.0, 10.0, 0.9367, 0.0178);
    c.AddCorrelation ("statistical", m1, m2, 0.496615);

    setupRoo();
    map<string, CombinationContext::FitResult> fr = c.Fit();

    CPPUNIT_ASSERT_EQUAL (size_t(0), fr["average"].sysErrors.size());
    //CPPUNIT_ASSERT_DOUBLES_EQUAL (0.871953, fr["average"].centralValue, 0.001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL (0.0192838, fr["average"].statisticalError, 0.001);
  }

  void testMeasurementSharedError()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    Measurement *m2 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    pair<double, double> s = m1->SharedError(m2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, s.first, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, s.second, 0.001);
  }

  void testMeasurementSharedError1()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.1);
    Measurement *m2 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    pair<double, double> s = m1->SharedError(m2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1*sqrt(2), s.first, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, s.second, 0.001);
  }

  void testMeasurementSharedError2()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.1);
    Measurement *m2 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    m2->addSystematicAbs("s1", 0.1);
    pair<double, double> s = m1->SharedError(m2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, s.first, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, s.second, 0.001);
  }

  void testMeasurementSharedError3()
  {
    CombinationContext c;
    Measurement *m1 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    m1->addSystematicAbs("s1", 0.1);
    m1->addSystematicAbs("s2", 0.1);
    Measurement *m2 = c.AddMeasurement("a1", -10.0, 10.0, 1.0, 0.1);
    m2->addSystematicAbs("s1", 0.1);
    m2->addSystematicAbs("s2", 0.1);
    pair<double, double> s = m1->SharedError(m2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, s.first, 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1*sqrt(2), s.second, 0.001);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CombinationContextTest);
//#include <TestPolicy/CppUnit_testdriver.cxx>
