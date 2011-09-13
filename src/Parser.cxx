//
// Parser.cxx
//
// ATLAS - is currently using spirit 2.2
//

#include "Combination/Parser.h"

#include <vector>
#include <stdexcept>

//
// All the boost libraries
//
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>


#include <iostream>
#include <string>
#include <vector>


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

//
// Parse a bin boundary "25 < pt < 30"
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

    start %= 
      double_ 
      >> '<'
      >> name_string >> '<'
      >> double_
      ;
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, CalibrationBinBoundary(), ascii::space_type> start;
};

//
// Parse the bin spec bin(30 < pt < 40, 2.5 < eta < 5.5) {xxx}
//
BOOST_FUSION_ADAPT_STRUCT(
			  CalibrationBin,
			  (std::vector<BTagCombination::CalibrationBinBoundary>, binSpec)
			  )

template <typename Iterator>
struct CalibrationBinParser : qi::grammar<Iterator, CalibrationBin(), ascii::space_type>
{
  CalibrationBinParser() : CalibrationBinParser::base_type(start, "Bin") {
    using qi::lit;

    boundary_list %= boundary % ',';
    start %= lit("bin") > '(' > boundary_list >  ')';
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    CalibrationBinBoundaryParser<Iterator> boundary;
    qi::rule<Iterator, std::vector<CalibrationBinBoundary>, ascii::space_type>  boundary_list;
    qi::rule<Iterator, CalibrationBin(), ascii::space_type> start;
};

//
// Parse the top level analysis info
//
BOOST_FUSION_ADAPT_STRUCT(
			  BTagCombination::CalibrationAnalysis,
			  (std::string, name)
			  //(BTagCombination::Flavor, flavor)
			  (std::string, flavor)
			  (std::string, operatingPoint)
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

    start %= lit("Analysis")
      > lit('(')
      > name_string
      > lit(',')
      > name_string
      > lit(',')
      > name_string
      > lit(')')
      > lit('{')
      > *binParser
      > lit('}')
      ;
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
      anaParser.name("Analyiss");
      start %= *anaParser > eoi;

      //
      // Error handling - attempt to make this a little nicer
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

//
//
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

/////////////////
// Parser primiatives. They parse combo info things
////////////////

// Parse  asystematic error ("sys(JES, 0.01)", "sys(JER, 0.1%)").
  template <typename Iterator>
struct SystematicErrorParser : qi::grammar<Iterator, SystematicError(), ascii::space_type>
{
  SystematicErrorParser() : SystematicErrorParser::base_type(start) {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using qi::double_;

    name_string %= lexeme[+(char_ - ',' - '"')];

    start %= lit("sys")
      >> '('
      >> name_string >> ','
      >> double_
      >> ')'
      ;
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, SystematicError(), ascii::space_type> start;
};

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
#endif

///////////////////////////////////////


namespace BTagCombination
{

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

    //if (iter != end)
    //  throw runtime_error ("Did not parse the complete input text!");

    return result;
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
