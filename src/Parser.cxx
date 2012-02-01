//
// Parser.cxx
//
// ATLAS - is currently using spirit 2.2
//

#include "Combination/Parser.h"

#include <vector>
#include <stdexcept>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

//
// All the boost libraries
//
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

using namespace BTagCombination;
using namespace std;

//////////////////////////
// Disable warnings caused by using the ">" operator in an
// overloaded way. I know this is ugly.
//
#pragma GCC diagnostic ignored "-Wparentheses"

///////////////
// The parser code itself
//
//////////////

// Help with debugging - so we have a place to set a break point!
// If you want to track the parser in the debugger, put a break in the funciton below. It
// gets called before and after each rule is parsed that you've attached it too (see the
// debug statement below). I would recomment only turning on one debugging for a rule at a time
// otherwise output can get pretty confusing!
//
namespace {
  class my_handler
  {
  public:
    template <typename Iterator, typename Context, typename State>
    void operator()(
		    Iterator const& first, Iterator const& last, Context const& context, State state, std::string const& rule_name) const
    {
    }
    
  };
}

//
// Parse a bin boundary "25 < pt < 30"
//
BOOST_FUSION_ADAPT_STRUCT(
			  CalibrationBinBoundary,
			  (double, lowvalue)
			  (std::string, variable)
			  (double, highvalue)
			  )

  template <typename Iterator>
struct CalibrationBinBoundaryParser :
  qi::grammar<Iterator, CalibrationBinBoundary(), ascii::space_type>
{
  CalibrationBinBoundaryParser() : CalibrationBinBoundaryParser::base_type(start, "Bin Boundary") {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using boost::spirit::qi::alpha;
    using qi::double_;

    name_string %= lexeme[+alpha];
    name_string.name("variable name");

    start %= 
      double_ 
      >> '<'
      >> name_string >> '<'
      >> double_
      ;
    start.name("Boundary");

    // This will write to std::out the before and after invokation of the start
    // rule.
    //debug(start, my_handler());
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, CalibrationBinBoundary(), ascii::space_type> start;
};
//
// An error can be relative or not. If relative is true, it is relative w.r.t. some central value.
// The central value can be found by looking up at whatever is holding onto this guy. Most errors
// have a name, but not all.
//
//  a "0.83" is an aboslute number, and "0.83%" is a relative number. You can add a <space> between
// the two.
//
struct ErrorValue
{
  ErrorValue(double err = 0.0)
  {
    error = err;
    relative = false;
    uncorrelated = false;
  }

  void MakeRelative (void)
  {
    relative = true;
  }

  void MakeUncorrelated (void)
  {
    uncorrelated = true;
  }

  void MakeCorrelated (void)
  {
    uncorrelated = false;
  }

  void SetName (const std::string &n)
  {
    name = n;
    const size_t endStr = n.find_last_not_of(" \t");
    name = name.substr(0, endStr+1);
  }

  void CopyErrorAndRelative(const ErrorValue &cp)
  {
    error = cp.error;
    relative = cp.relative;
  }

  string name;
  double error;
  bool relative;
  bool uncorrelated;
};

ErrorValue EV_relative (const ErrorValue &ev)
{
  ErrorValue v (ev);
  v.relative = true;
  return v;
}

//
// Parse a systematic error - these look like "sys(JES, 0.1)" or "sys(JER, 0.2%)".
//
template <typename Iterator>
struct ErrorValueParser : qi::grammar<Iterator, ErrorValue(), ascii::space_type>
{
  ErrorValueParser() : ErrorValueParser::base_type(start, "Error") {
    using qi::lit;
    using qi::double_;
    using qi::_val;
    using qi::_1;

    start = double_[_val = _1]
      >> -(lit("%")[boost::phoenix::bind(&ErrorValue::MakeRelative, _val)]);
		
    start.name("error");
  }

    qi::rule<Iterator, ErrorValue(), ascii::space_type> start;
};

//
// Parse  asystematic error ("sys(JES, 0.01)", "sys(JER, 0.1%)"). We parse
// into a special struct b/c we can't do the % vs non-% until later.
//
template <typename Iterator>
struct SystematicErrorParser : qi::grammar<Iterator, ErrorValue(), ascii::space_type>
{
  SystematicErrorParser() : SystematicErrorParser::base_type(start, "Systematic Error") {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using qi::double_;
    using qi::_val;
    using qi::labels::_1;

    name_string %= lexeme[+(char_ - ',' - '"')];
    name_string.name("Systematic Error Name");

    start = (
	     lit("sys")[boost::phoenix::bind(&ErrorValue::MakeCorrelated, _val)]
	     |lit("usys")[boost::phoenix::bind(&ErrorValue::MakeUncorrelated, _val)]
	     )
      > '('
      > name_string[boost::phoenix::bind(&ErrorValue::SetName, _val, _1)] > ','
      > errParser[boost::phoenix::bind(&ErrorValue::CopyErrorAndRelative, _val, _1)]
      > ')';

    start.name("Systematic Error");
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    ErrorValueParser<Iterator> errParser;
    qi::rule<Iterator, ErrorValue(), ascii::space_type> start;
};

struct centralvalue
{
  double value;
  double error;

  void SetError (const ErrorValue &e)
  {
    error = e.error;
    if (e.relative)
      error = error / 100.0 * value;
  }
  void SetValue (const double v)
  {
    value = v;
  }
};

//
// Parse the central value
template <typename Iterator>
struct CentralValueParser : qi::grammar<Iterator, centralvalue(), ascii::space_type>
{
  CentralValueParser() : CentralValueParser::base_type(start, "Central Value") {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using qi::double_;
    using qi::_val;
    using qi::labels::_1;

    start = lit("central_value")
      > '('
      > double_[boost::phoenix::bind(&centralvalue::SetValue, _val, _1)]
      > ','
      > errParser[boost::phoenix::bind(&centralvalue::SetError, _val, _1)]
      > ')';

    start.name("Central Value");
    errParser.name("central value error");
  }

    ErrorValueParser<Iterator> errParser;
    qi::rule<Iterator, centralvalue(), ascii::space_type> start;
};


//
// Parse the bin spec bin(30 < pt < 40, 2.5 < eta < 5.5) {xxx}. There should be only one central
// value, but to make the parsing flexible we need to do this.
//
struct localCalibBin
{
  std::vector<BTagCombination::CalibrationBinBoundary> binSpec;
  std::vector<ErrorValue> sysErrors;
  std::vector<centralvalue> centralvalues;

  void Convert (CalibrationBin &result)
  {
    result.binSpec = binSpec;
    if (centralvalues.size() != 1)
      {
	throw std::runtime_error("One and only one central value must be present in each bin");
      }
    result.centralValue = centralvalues[0].value;
    result.centralValueStatisticalError = centralvalues[0].error;

    for (unsigned int i = 0; i < sysErrors.size(); i++)
      {
	SystematicError e;
	e.name = sysErrors[i].name;
	e.value = sysErrors[i].error;
	e.uncorrelated = sysErrors[i].uncorrelated;
	if (sysErrors[i].relative)
	  {
	    e.value *= result.centralValue / 100.0;
	  }
	result.systematicErrors.push_back(e);
      }
  }

  void AddSysError (const ErrorValue &v)
  {
    sysErrors.push_back(v);
  }
  void AddCentralValue (const centralvalue &c)
  {
    centralvalues.push_back(c);
  }

  void SetBins(const vector<BTagCombination::CalibrationBinBoundary> &b)
  {
    binSpec = b;
  }
};

BOOST_FUSION_ADAPT_STRUCT(
			  localCalibBin,
			  (std::vector<BTagCombination::CalibrationBinBoundary>, binSpec)
			  (std::vector<ErrorValue>, sysErrors)
			  (std::vector<centralvalue>, centralvalues)
			  )

  template <typename Iterator>
struct CalibrationBinParser : qi::grammar<Iterator, CalibrationBin(), ascii::space_type>
{
  CalibrationBinParser() : CalibrationBinParser::base_type(converter, "Bin")
    {
      using qi::lit;
      using namespace qi::labels;
      using boost::phoenix::bind;

      boundary_list %= (boundary % ',');
      boundary_list.name("List of Bin Boundaries");

      sysErrors.name("Systematic Errors");
      cvFinder.name("Central Value");

      localBinFinder.name("Bin finder");

      localBinFinder = lit("bin") 
	> '(' > boundary_list [boost::phoenix::bind(&localCalibBin::SetBins, _val, _1)] > ')'
	> '{'
	> *(sysErrors[bind(&localCalibBin::AddSysError, _val, _1)] | cvFinder[bind(&localCalibBin::AddCentralValue, _val, _1)])
	> '}';

      converter = localBinFinder[bind(&localCalibBin::Convert, _1, _val)];

      converter.name("Bin");

      // Turn on debugging to sort out what is going on during development
      //debug(start);
      //debug(boundary_list);
    }

    CalibrationBinBoundaryParser<Iterator> boundary;
    qi::rule<Iterator, std::vector<BTagCombination::CalibrationBinBoundary>(), ascii::space_type>  boundary_list;
    SystematicErrorParser<Iterator> sysErrors;
    CentralValueParser<Iterator> cvFinder;
    qi::rule<Iterator, localCalibBin(), ascii::space_type> localBinFinder;
    qi::rule<Iterator, CalibrationBin(), ascii::space_type> converter;
};

//
// Parse the top level analysis info
//
BOOST_FUSION_ADAPT_STRUCT(
			  BTagCombination::CalibrationAnalysis,
			  (std::string, name)
			  (std::string, flavor)
			  (std::string, tagger)
			  (std::string, operatingPoint)
			  (std::string, jetAlgorithm)
			  (std::vector<CalibrationBin>, bins)
			  )

  template <typename Iterator>
struct CalibrationAnalysisParser : qi::grammar<Iterator, CalibrationAnalysis(), ascii::space_type>
{
  CalibrationAnalysisParser() : CalibrationAnalysisParser::base_type(start, "Analysis") {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;

    name_string %= lexeme[+(char_ - ',' - '"' - '}' - '{' - ')' - '(')];

    start %= lit("Analysis") > lit('(') > name_string > ',' > name_string > ',' > name_string > ',' > name_string > ',' > name_string > ')'
      > '{'
      > *binParser
      > '}';
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, CalibrationAnalysis(), ascii::space_type> start;
    CalibrationBinParser<Iterator> binParser;
};

//
// Parse a file - so this is a sequence of Analysis guys, with nothing
// else in them.
//
template<typename Iterator>
struct CalibrationAnalysisFileParser : qi::grammar<Iterator, vector<CalibrationAnalysis>(), ascii::space_type>
{
  CalibrationAnalysisFileParser() : CalibrationAnalysisFileParser::base_type(start, "Calibration Analysis File")
    {
      using boost::spirit::qi::eoi;
      anaParser.name("Analyis");
      start %= *anaParser > eoi;

      //
      // Error handling - This shoudl show the user exactly where it is that our parser got jammed up on their
      // file. Currnetly just sent to std::out.
      //

      using qi::on_error;
      using qi::fail;
      using boost::phoenix::val;
      using boost::phoenix::construct;
      using namespace qi::labels;

      on_error<fail>
	(
	 start,
	 std::cout
	 << val("Error! Expecting ")
	 << _4                               // what failed?
	 << val(" here: \"")
	 << construct<std::string>(_3, _2)   // iterators to error-pos, end
	 << val("\"")
	 << std::endl
	 );
    }

    qi::rule<Iterator, vector<CalibrationAnalysis>(), ascii::space_type> start;
    CalibrationAnalysisParser<Iterator> anaParser;
};

#ifdef notyet
BOOST_FUSION_ADAPT_STRUCT(
			  SystematicError,
			  (std::string, name)
			  (double, val)
			  )

  BOOST_FUSION_ADAPT_STRUCT(
			    CentralValue,
			    (double, Value)
			    (double, Error)
			    )

#endif

//
//
#ifdef notyet

/////////////////
// Parser primiatives. They parse combo info things
////////////////

// Parse  the central value "(central_value(0.9, 0.1%)".
  template <typename Iterator>
struct CentralValueParser : qi::grammar<Iterator, CentralValue(), ascii::space_type>
{
  CentralValueParser() : CentralValueParser::base_type(start) {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using qi::double_;

    name_string %= lexeme[+(char_ - ',' - '"')];

    start %= lit("central_value")
      >> '('
      >> double_ >> ","
      >> double_
      >> ')'
      ;
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, CentralValue(), ascii::space_type> start;
};

// Some debugging code incase we need it again...
vector<CalibrationBinBoundary> bb;
	
CalibrationBinBoundaryParser<string::const_iterator> boundary;
qi::rule<string::const_iterator, vector<CalibrationBinBoundary>(), ascii::space_type>  boundary_list;
string input = "20 < pt < 25";
boundary_list %= boundary % ",";
auto bogus = boundary % ",";
bool r1 = phrase_parse(input.begin(), input.end(),
		       boundary_list,
		       ascii::space,
		       bb);

string input = "sys (JES, 0.55)";
SystematicErrorParser<string::const_iterator> errParser;
ErrorValue mr;
bool r1 = phrase_parse(input.begin(), input.end(),
		       errParser,
		       ascii::space,
		       mr);


#endif

///////////////////////////////////////


namespace BTagCombination
{

  // Declare the printout guy.
  unsigned int CalibrationBin::gForNextPrinting =
    CalibrationBin::kFullInfo;

  CalibrationBinBoundary::BinBoundaryFormatEnum CalibrationBinBoundary::gFormatForNextBoundary =
    CalibrationBinBoundary::kNormal;

  //
  // Parse the input text as a list of calibration inputs
  //
  vector<CalibrationAnalysis> Parse(const string &inputText)
  {
    string::const_iterator iter = inputText.begin();
    string::const_iterator end = inputText.end();

    vector<CalibrationAnalysis> result;
    CalibrationAnalysisFileParser<string::const_iterator> caParser;
    bool didit = phrase_parse(iter, end,
			      caParser,
			      ascii::space, result);

    //
    // See if there were any errors doing the parse.
    //

    if (!didit)
      throw runtime_error ("Unable to parse!");

    return result;
  }

  //
  // Parse an input file
  //
  vector<CalibrationAnalysis> Parse (istream &input)
  {

    ostringstream text;
    while (!input.eof()) {
      string line;
      getline(input, line);
      text << line << endl;
    }

    return Parse(text.str());
  }
  
#ifdef notyet

  namespace client
  {
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    ///////////////////////////////////////////////////////////////////////////
    //  Our number list parser
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    bool parse_numbers(Iterator first, Iterator last)
    {
      using qi::double_;
      using qi::phrase_parse;
      using ascii::space;

      bool r = phrase_parse(
			    first,                          /*< start iterator >*/
			    last,                           /*< end iterator >*/
			    double_ >> *(',' >> double_),   /*< the parser >*/
			    space                           /*< the skip-parser >*/
			    );
      if (first != last) // fail if we did not get a full match
	return false;
      return r;
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  //  Main program
  ////////////////////////////////////////////////////////////////////////////
  int
  main()
  {
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\tA comma separated list parser for Spirit...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";

    std::cout << "Give me a comma separated list of numbers.\n";
    std::cout << "Type [q or Q] to quit\n\n";

    std::string str;
    while (getline(std::cin, str))
      {
	if (str.empty() || str[0] == 'q' || str[0] == 'Q')
	  break;

	if (client::parse_numbers(str.begin(), str.end()))
	  {
	    std::cout << "-------------------------\n";
	    std::cout << "Parsing succeeded\n";
	    std::cout << str << " Parses OK: " << std::endl;
	  }
	else
	  {
	    std::cout << "-------------------------\n";
	    std::cout << "Parsing failed\n";
	    std::cout << "-------------------------\n";
	  }
      }

    std::cout << "Bye... :-) \n\n";
    return 0;
  }
#endif
}
