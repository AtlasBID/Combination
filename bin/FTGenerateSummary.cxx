//
// Simpmle app to read in a clibration file and then
// will generate some twiki tables and a few plots
// (in a root file) and their eps/png matching files.
//

#include "Combination/Parser.h"

#include <iostream>
using namespace std;
#include <boost/spirit/version.hpp>

int main()
{
  ParseMe();
  cout << "hi " << SPIRIT_VERSION  <<endl;
  return 0;
}
