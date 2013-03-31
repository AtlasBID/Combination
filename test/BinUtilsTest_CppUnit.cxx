///
/// CppUnit tests for the Bin utilities object.
///

#include "Combination/BinUtils.h"
#include "Combination/Parser.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

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

  CPPUNIT_TEST( testRemoveAllButBinMetadata );
  CPPUNIT_TEST( testRemoveBinMetadata );
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
