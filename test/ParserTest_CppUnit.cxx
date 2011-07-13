///
/// CppUnit tests for the parser
///
///  This is copmlex enough I feel like we need a parser around to make sure nothing gets
/// screwed up as development evolves over time.
///
///  This is black-box testing of the parser.
///

#include "Combination/Parser.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace BTagCombination;

//
// Test harness/fixture for the parser
//

class ParserTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( ParserTest );

  //CPPUNIT_TEST( testSourceComments );

  //CPPUNIT_TEST_EXCEPTION( testParseSyntaxBasicErrorThrows, std::runtime_error );

  CPPUNIT_TEST( testParseEmptyAnalysisString );
  CPPUNIT_TEST( testParseSimpleAnalysis );
  //CPPUNIT_TEST( testParseTwoAnalyses );
  //CPPUNIT_TEST( testParseSimpleAnalysisBadFlavor );

  CPPUNIT_TEST_SUITE_END();

  void testSourceComments()
  {
    // Test that comments in the code can happen anywhere! :-)
    CPPUNIT_ASSERT_MESSAGE("Not written yet", false);
  }

  void testParseEmptyAnalysisString()
  {
    vector<CalibrationAnalysis> result (Parse(""));
    CPPUNIT_ASSERT(result.size() == 0);
  }

  // Nothing good can come of this.
  void testParseSyntaxBasicErrorThrows()
  {
    vector<CalibrationAnalysis> result (Parse("AAANNNnalysis(ptrel, bottom, SV050){}"));
  }

  void testParseSimpleAnalysis()
  {
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){}"));
    cout << "Result size is " << result.size() << "!" << endl;
    CPPUNIT_ASSERT(result.size() == 1);
    CPPUNIT_ASSERT(result[0].name == "ptrel");
    CPPUNIT_ASSERT_MESSAGE(result[0].operatingPoint, result[0].operatingPoint == "SV050");
    //CPPUNIT_ASSERT(result[0].flavor == FBottom);
    CPPUNIT_ASSERT(result[0].flavor == "bottom");
    CPPUNIT_ASSERT(result[0].bins.size() == 0);
  }

  void testParseSimpleAnalysisBadFlavor()
  {
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, botttom, SV050){}"));
    CPPUNIT_ASSERT(false);
  }

  void testParseTwoAnalyses()
  {
    CPPUNIT_ASSERT_MESSAGE("Not written yet", false);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);

// The common atlas test driver
#include <TestPolicy/CppUnit_testdriver.cxx>
