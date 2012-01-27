///
/// CppUnit tests for the Bin boundary utils.
///
///

#include "Combination/BinBoundaryUtils.h"

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

class BinBoundaryUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( BinBoundaryUtilsTest );

  CPPUNIT_TEST( testSimpleOneBins );
  CPPUNIT_TEST_EXCEPTION ( TestOverlapNegEta, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestOverlapBins, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestGappedBins, std::runtime_error );

  CPPUNIT_TEST (TestBBOK1D);
  CPPUNIT_TEST (TestBBOK1D2);
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D2, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D3, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D4, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D5, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D6, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBOverlap1D7, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBMissingAxes1D, std::runtime_error );
  CPPUNIT_TEST_EXCEPTION ( TestBBMissingAxes1D2, std::runtime_error );

  CPPUNIT_TEST_SUITE_END();

  void testSimpleOneBins()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;
    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);
    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestOverlapBins()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);

    CalibrationBinBoundary bb2;
    bb2.lowvalue = 0.5;
    bb2.highvalue = 1.5;
    bb2.variable = "pt";
    b1.binSpec.push_back(bb2);

    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestOverlapNegEta()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 2.5;
    bb1.variable = "eta";
    b1.binSpec.push_back(bb1);

    CalibrationBinBoundary bb2;
    bb2.lowvalue = -2.5;
    bb2.highvalue = 2.5;
    bb2.variable = "eta";
    b1.binSpec.push_back(bb2);

    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestGappedBins ()
  {
    CalibrationAnalysis ana;
    CalibrationBin b1;

    CalibrationBinBoundary bb1;
    bb1.lowvalue = 0.0;
    bb1.highvalue = 1.0;
    bb1.variable = "pt";
    b1.binSpec.push_back(bb1);

    CalibrationBinBoundary bb2;
    bb2.lowvalue = 2.0;
    bb2.highvalue = 3.0;
    bb2.variable = "pt";
    b1.binSpec.push_back(bb2);

    ana.bins.push_back(b1);

    bin_boundaries result (calcBoundaries(ana));
  }

  void TestBBOK1D()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    b2.add_axis("e1", bounds);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOK1D2()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    bounds.push_back(5.0);
    b2.add_axis("e1", bounds);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(-2.5);
    bounds2.push_back(2.5);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D2()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D3()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(2.5);
    bounds.push_back(4.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D4()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(4.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D5()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(4.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D6()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBOverlap1D7()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(1.0);
    bounds.push_back(5.0);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(0.0);
    bounds2.push_back(5.0);
    b2.add_axis("e1", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b2);
    bb.push_back(b1);

    checkForConsitentBoundaries(bb);
  }

  void TestBBMissingAxes1D()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);

    bin_boundaries b2;
    vector<double> bounds2;
    bounds2.push_back(-2.5);
    bounds2.push_back(2.5);
    b2.add_axis("e2", bounds2);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }

  void TestBBMissingAxes1D2()
  {
    // A set of analyses with bin boundaries that are "ok".
    bin_boundaries b1;
    vector<double> bounds;
    bounds.push_back(0.0);
    bounds.push_back(2.5);
    b1.add_axis("e1", bounds);
    b1.add_axis("e2", bounds);

    bin_boundaries b2;
    b2.add_axis("e1", bounds);

    vector<bin_boundaries> bb;
    bb.push_back(b1);
    bb.push_back(b2);

    checkForConsitentBoundaries(bb);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BinBoundaryUtilsTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
