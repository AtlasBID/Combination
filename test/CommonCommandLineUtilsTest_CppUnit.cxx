///
/// CppUnit tests for the argument processor
///

#include "Combination/CommonCommandLineUtils.h"

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

class CommonCommandLineUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CommonCommandLineUtilsTest );

  CPPUNIT_TEST( testEmptyCommandLine );
  CPPUNIT_TEST( testUnknownFlag );
  CPPUNIT_TEST( testInputFromFile );
  CPPUNIT_TEST_EXCEPTION( testInputFromBadFile, std::runtime_error );

  CPPUNIT_TEST_SUITE_END();

  void testEmptyCommandLine()
  {
    vector<CalibrationAnalysis> results;
    vector<string> unknown;
    const char *argv[] = {"hi", "there"};
    
    ParseOPInputArgs(argv, 0, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t)0, results.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, unknown.size());
  }

  void testUnknownFlag()
  {
    vector<CalibrationAnalysis> results;
    vector<string> unknown;
    const char *argv[] = {"--doit"};

    ParseOPInputArgs(argv, 1, results, unknown);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("#results", (size_t)0, results.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("#unknowns", (size_t)1, unknown.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unknown flag name", string("doit"), unknown[0]);
  }

  void testInputFromFile()
  {
    vector<CalibrationAnalysis> results;
    vector<string> unknown;
    const char *argv[] = {"../testdata/JetFitcnn_eff60.txt"};

    ParseOPInputArgs(argv, 1, results, unknown);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, results.size());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, unknown.size());

    CalibrationAnalysis ana = results[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE("calibration analysis name", string("ttbar_kin_ljets"), ana.name);
  }

  void testInputFromBadFile()
  {
    vector<CalibrationAnalysis> results;
    vector<string> unknown;
    const char *argv[] = {"simpleInputBogus.txt"};

    ParseOPInputArgs(argv, 1, results, unknown);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CommonCommandLineUtilsTest);

// The common atlas test driver
//#include <TestPolicy/CppUnit_testdriver.cxx>
