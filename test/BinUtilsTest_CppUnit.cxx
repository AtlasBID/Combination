///
/// CppUnit tests for the Bin utilities object.
///

#include "Combination/BinUtils.h"
#include "Combination/Parser.h"
#include "Combination/CalibrationDataModelStreams.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <set>

using namespace std;
using namespace BTagCombination;

class BinUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( BinUtilsTest );

  CPPUNIT_TEST( testListNoBins );
  CPPUNIT_TEST( testListOneBins );
  CPPUNIT_TEST( testListTwoSameBins );
  CPPUNIT_TEST( testListTwoAnaBins );

  CPPUNIT_TEST( testRemoveBin );
  CPPUNIT_TEST( testRemoveOnlyBins );

  CPPUNIT_TEST ( testBinBoundarySetCompare );
  CPPUNIT_TEST ( testBinBoundarySetCompareReverse );

  CPPUNIT_TEST( testRemoveAllButBinMetadata );
  CPPUNIT_TEST( testRemoveBinMetadata );

  CPPUNIT_TEST( testFindBinWithLowEdge);

  CPPUNIT_TEST( testBinSysNone );
  CPPUNIT_TEST( testBinSysOne );
  CPPUNIT_TEST( testBinSysTwo );

  CPPUNIT_TEST_SUITE_END();

  void testListNoBins()
  {
    CalibrationAnalysis ana;
    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    set<set<CalibrationBinBoundary> > bins (listAllBins(anas));
    CPPUNIT_ASSERT_EQUAL ((size_t)0, bins.size());
  }

  void testListOneBins()
  {
    CalibrationAnalysis ana;
    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);

    set<set<CalibrationBinBoundary> > bins (listAllBins(anas));
    CPPUNIT_ASSERT_EQUAL ((size_t)1, bins.size());
    set<CalibrationBinBoundary> abin(*(bins.begin()));
    CPPUNIT_ASSERT_EQUAL ((size_t)1, abin.size());
    CPPUNIT_ASSERT_EQUAL ((string) "eta", abin.begin()->variable);
  }

  void testBinBoundarySetCompare()
  {
    // Test that a set equality works correctly. Many of these utilities depend
    // upon it.

    CalibrationBinBoundary b1;
    b1.variable = "eta";
    b1.lowvalue = 0.0;
    b1.highvalue = 2.5;
    
    CalibrationBinBoundary b2;
    b2.variable = "pt";
    b2.lowvalue = 30.0;
    b2.highvalue = 60.0;
    
    set<CalibrationBinBoundary> s1, s2;
    s1.insert(b1);
    s1.insert(b2);
    s2.insert(b1);
    s2.insert(b2);

    CPPUNIT_ASSERT (s1 == s2);
  }
  void testBinBoundarySetCompareReverse()
  {
    // Test that a set equality works correctly. Many of these utilities depend
    // upon it.

    CalibrationBinBoundary b1;
    b1.variable = "eta";
    b1.lowvalue = 0.0;
    b1.highvalue = 2.5;
    
    CalibrationBinBoundary b2;
    b2.variable = "pt";
    b2.lowvalue = 30.0;
    b2.highvalue = 60.0;
    
    set<CalibrationBinBoundary> s1, s2;
    s1.insert(b1);
    s1.insert(b2);
    s2.insert(b2);
    s2.insert(b1);

    CPPUNIT_ASSERT (s1 == s2);
  }

  void testListTwoSameBins()
  {
    CalibrationAnalysis ana;

    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    ana.bins.push_back(b);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);

    set<set<CalibrationBinBoundary> > bins (listAllBins(anas));
    CPPUNIT_ASSERT_EQUAL ((size_t)1, bins.size());
    set<CalibrationBinBoundary> abin(*(bins.begin()));
    CPPUNIT_ASSERT_EQUAL ((size_t)1, abin.size());
    CPPUNIT_ASSERT_EQUAL ((string) "eta", abin.begin()->variable);
  }

  void testListTwoAnaBins()
  {
    CalibrationAnalysis ana;

    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    anas.push_back(ana);

    set<set<CalibrationBinBoundary> > bins (listAllBins(anas));
    CPPUNIT_ASSERT_EQUAL ((size_t)1, bins.size());
    set<CalibrationBinBoundary> abin(*(bins.begin()));
    CPPUNIT_ASSERT_EQUAL ((size_t)1, abin.size());
    CPPUNIT_ASSERT_EQUAL ((string) "eta", abin.begin()->variable);
  }

  void testFindBinWithLowEdge()
  {
    CalibrationAnalysis ana;
    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    vector<CalibrationBin> bins = find_bins_with_low_edge( "eta", 0.0, ana.bins);
    CPPUNIT_ASSERT_EQUAL(size_t(1), bins.size());
    CPPUNIT_ASSERT_EQUAL(b, bins[0]);
  }

  void testBinSysNone()
  {
    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);

    double r = bin_sys(b);
    CPPUNIT_ASSERT_EQUAL(0.0, r);
  }

  void testBinSysOne()
  {
    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    SystematicError e;
    e.name = "hi";
    e.value = 0.2;
    b.systematicErrors.push_back(e);

    double r = bin_sys(b);
    CPPUNIT_ASSERT_EQUAL(0.2, r);
  }

  void testBinSysTwo()
  {
    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    SystematicError e;
    e.name = "hi";
    e.value = 2;
    b.systematicErrors.push_back(e);
    e.name = "there";
    e.value = 2;
    b.systematicErrors.push_back(e);

    double r = bin_sys(b);
    CPPUNIT_ASSERT_EQUAL(sqrt(8), r);
  }

  void testRemoveBin()
  {
    CalibrationAnalysis ana;

    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    b.binSpec[0].lowvalue = 2.5;
    b.binSpec[0].highvalue = 4.0;
    ana.bins.push_back(b);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    anas.push_back(ana);

    set<CalibrationBinBoundary> boundset;
    boundset.insert(bound);
    vector<CalibrationAnalysis> r (removeBin(anas, boundset));

    CPPUNIT_ASSERT_EQUAL ((size_t) 2, r.size());
    const CalibrationAnalysis a1 (r[0]);
    CPPUNIT_ASSERT_EQUAL ((size_t) 1, a1.bins.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL (4.0, a1.bins[0].binSpec[0].highvalue, 0.1);
  }

  void testRemoveBinMetadata()
  {
    CalibrationAnalysis ana;

    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    b.binSpec[0].lowvalue = 2.5;
    b.binSpec[0].highvalue = 4.0;
    ana.bins.push_back(b);

    ana.metadata["m1"].push_back(10.0);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    anas.push_back(ana);

    set<CalibrationBinBoundary> boundset;
    boundset.insert(bound);
    vector<CalibrationAnalysis> r (removeBin(anas, boundset));

    CPPUNIT_ASSERT_EQUAL ((size_t) 2, r.size());
    CalibrationAnalysis a1 (r[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (10.0, a1.metadata["m1"][0], 0.1);
  }

  void testRemoveAllButBinMetadata()
  {
    CalibrationAnalysis ana;

    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    b.binSpec[0].lowvalue = 2.5;
    b.binSpec[0].highvalue = 4.0;
    ana.bins.push_back(b);

    ana.metadata["m1"].push_back(10.0);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    anas.push_back(ana);

    set<CalibrationBinBoundary> boundset;
    boundset.insert(bound);
    vector<CalibrationAnalysis> r (removeAllBinsButBin(anas, boundset));

    CPPUNIT_ASSERT_EQUAL ((size_t) 2, r.size());
    CalibrationAnalysis a1 (r[0]);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (10.0, a1.metadata["m1"][0], 0.1);
  }

  void testRemoveOnlyBins()
  {
    CalibrationAnalysis ana;

    CalibrationBin b;
    CalibrationBinBoundary bound;
    bound.variable = "eta";
    bound.lowvalue = 0.0;
    bound.highvalue = 2.5;
    b.binSpec.push_back(bound);
    ana.bins.push_back(b);

    vector<CalibrationAnalysis> anas;
    anas.push_back(ana);
    anas.push_back(ana);

    set<CalibrationBinBoundary> boundset;
    boundset.insert(bound);
    vector<CalibrationAnalysis> r (removeBin(anas, boundset));

    // b/c there is only one bin, so when there are no bins, then there is no analysis!
    CPPUNIT_ASSERT_EQUAL ((size_t) 0, r.size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinUtilsTest);

#ifdef ROOTCORE
// The common atlas test driver
#include <TestPolicy/CppUnit_testdriver.cxx>
#endif
