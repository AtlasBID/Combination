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
#include <sstream>

using namespace std;
using namespace BTagCombination;

//
// Test harness/fixture for the parser
//

class ParserTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( ParserTest );

  //CPPUNIT_TEST( testSourceComments );

  CPPUNIT_TEST_EXCEPTION( testParseSyntaxBasicErrorThrows, std::runtime_error );
  CPPUNIT_TEST( testParseEmptyAnalysisString );
  CPPUNIT_TEST( testParseSimpleAnalysis );
  CPPUNIT_TEST( testParseSimpleAnalysisWithOneBinOneArg );
  //CPPUNIT_TEST( testParseSimpleAnalysisWithOneBinTwoArg );
  CPPUNIT_TEST( testParseTwoAnalyses );
  //CPPUNIT_TEST( testParseSimpleAnalysisBadFlavor );
  CPPUNIT_TEST(testParseSimpleAnalysisWithSys);
  CPPUNIT_TEST(testParseSimpleAnalysisWithUSys);
  CPPUNIT_TEST(testParseRoundTrip);
  CPPUNIT_TEST(testParseRoundTrip2);
  CPPUNIT_TEST_SUITE_END();

  void testSourceComments()
  {
    cout << "Test testSourceComments" << endl;
    // Test that comments in the code can happen anywhere! :-)
    CPPUNIT_ASSERT_MESSAGE("Not written yet", false);
  }

  void testParseEmptyAnalysisString()
  {
    cout << "Test testParseEmptyAnalysisString" << endl;
    vector<CalibrationAnalysis> result (Parse(""));
    CPPUNIT_ASSERT(result.size() == 0);
  }

  // Nothing good can come of this.
  void testParseSyntaxBasicErrorThrows()
  {
    cout << "Test testparseSyntaxBasicErrorThrows" << endl;
    vector<CalibrationAnalysis> result (Parse("AAANNNnalysis(ptrel, bottom, SV050){}"));
  }

  void testParseSimpleAnalysis()
  {
    cout << "Test testParseSimpleAnalysis" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){}"));
    stringstream str;
    str << "Result size is " << result.size() << "!" << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), result.size() == 1);
    CPPUNIT_ASSERT(result[0].name == "ptrel");
    CPPUNIT_ASSERT_MESSAGE(result[0].operatingPoint, result[0].operatingPoint == "0.50");
    //CPPUNIT_ASSERT(result[0].flavor == FBottom);
    CPPUNIT_ASSERT(result[0].tagger == "SV0");
    CPPUNIT_ASSERT(result[0].jetAlgorithm == "MyJets");
    CPPUNIT_ASSERT(result[0].flavor == "bottom");
    CPPUNIT_ASSERT(result[0].bins.size() == 0);
  }

  void testParseTwoAnalyses()
  {
    cout << "Test testParseTwoAnalyses" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){} Analysis(system8, bottom, SV0, 0.50, MyJets) {}"));
    stringstream str;
    str << "Result size is " << result.size() << "!" << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), result.size() == 2);
    CPPUNIT_ASSERT(result[0].name == "ptrel");
    CPPUNIT_ASSERT(result[1].name == "system8");
  }

  void testParseSimpleAnalysisBadFlavor()
  {
    cout << "Test testParseSimpleAnalysisBadFlavor" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, botttom, SV050){}"));
    CPPUNIT_ASSERT(false);
  }

  void testParseSimpleAnalysisWithOneBinOneArg()
  {
    cout << "Test testParseSimpleAnalysisWithOneBinOneArg" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01)}}"));
    
    CPPUNIT_ASSERT(result.size() == 1);
    CalibrationAnalysis ana = result[0];
    stringstream str;
    str << "  Found " << ana.bins.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 1);
    CalibrationBin bin0 = ana.bins[0];
    cout << "  Found bins to do spec in: " << bin0.binSpec.size() << endl;
    stringstream str1;
    str1 << "The number of bin boundaries is " << bin0.binSpec.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str1.str(), bin0.binSpec.size() == 1);
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);
  }

  void testParseSimpleAnalysisWithOneBinTwoArg()
  {
    cout << "Test testParseSimpleAnalysisWithOneBinTwoArg" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV050){bin(20<pt<30, 1.1<eta<5.5)}"));
    
    CPPUNIT_ASSERT(result.size() == 1);
    CalibrationAnalysis ana = result[0];
    stringstream str;
    str << "  Found " << ana.bins.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str.str(), ana.bins.size() == 1);
    CalibrationBin bin0 = ana.bins[0];
    cout << "  Found bins to do spec in: " << bin0.binSpec.size() << endl;
    stringstream str1;
    str1 << "The number of bin boundaries is " << bin0.binSpec.size() << endl;
    CPPUNIT_ASSERT_MESSAGE(str1.str(), bin0.binSpec.size() == 2);
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    bb = bin0.binSpec[1];
    CPPUNIT_ASSERT(bb.lowvalue == 1.1);
    CPPUNIT_ASSERT(bb.variable == "eta");
    CPPUNIT_ASSERT(bb.highvalue == 5.5);
  }

  void testParseSimpleAnalysisWithSys()
  {
    cout << "Test testParseSimpleAnalysisWithOneBinOneArg" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    

    CalibrationAnalysis ana = result[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseSimpleAnalysisWithUSys()
  {
    cout << "Test testParseSimpleAnalysisWithOneBinOneArg" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) usys(dude, 0.1%)}}"));
    

    CalibrationAnalysis ana = result[0];
    CalibrationBin bin0 = ana.bins[0];

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseRoundTrip()
  {
    cout << "Test testParseRoundTrip" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    
    ostringstream buffer;
    cout << result[0] << endl;
    buffer << result[0] << endl;

    vector<CalibrationAnalysis> result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result2.size());
    CalibrationAnalysis ana = result2[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, ana.bins.size());
    CalibrationBin bin0 = ana.bins[0];
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, bin0.centralValue, 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01, bin0.centralValueStatisticalError, 0.001);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.binSpec.size());
    CalibrationBinBoundary bb = bin0.binSpec[0];
    CPPUNIT_ASSERT(bb.lowvalue == 20);
    CPPUNIT_ASSERT(bb.variable == "pt");
    CPPUNIT_ASSERT(bb.highvalue == 30);

    CPPUNIT_ASSERT_EQUAL((size_t)1, bin0.systematicErrors.size());
    SystematicError e(bin0.systematicErrors[0]);
    CPPUNIT_ASSERT_EQUAL(string("dude"), e.name);
    CPPUNIT_ASSERT_DOUBLES_EQUAL (0.05*0.1/100.0, e.value, 0.001);
  }

  void testParseRoundTrip2()
  {
    cout << "Test testParseRoundTrip2" << endl;
    vector<CalibrationAnalysis> result (Parse("Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}} Analysis(ptrel, bottom, SV0, 0.50, MyJets){bin(20<pt<30){central_value(0.5,0.01) sys(dude, 0.1%)}}"));
    
    ostringstream buffer;
    cout << result[0] << result[1] << endl;
    buffer << result[0] << result[1] << endl;

    vector<CalibrationAnalysis> result2 (Parse(buffer.str()));

    CPPUNIT_ASSERT_EQUAL((size_t)2, result2.size());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);

// The common atlas test driver
#include <TestPolicy/CppUnit_testdriver.cxx>
