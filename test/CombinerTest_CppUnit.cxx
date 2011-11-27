///
/// CppUnit tests for the parser
///
///  Some pretty simple combinations - we know the results for these
/// cases, so make sure the code works as expected
///

#include "Combination/Combiner.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Exception.h>

using namespace std;
using namespace BTagCombination;

class CombinerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CombinerTest );

  CPPUNIT_TEST ( testOBOneBinIn );

  CPPUNIT_TEST_SUITE_END();

  void testOBOneBinIn()
  {
    // We send in one bin to be combined, and it comes back out!
    CalibrationBin b;
    CPPUNIT_ASSERT_MESSAGE("not writen yet", false);
  }

};
