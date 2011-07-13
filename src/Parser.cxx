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

///////////////
// The parser code itself
//
//////////////

// A systematic error
struct SystematicError
{
  std::string name;
  double val;
};

struct CentralValue
{
  double Value;
  double Error;
};

// info for a binning boundary (30 < pt < 56).
struct BinBoundary
{
  double lowerBound;
  std::string variableName;
  double upperBound;
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
			  )

// Parse an analysis "Analysis(ptrel, bottom, SV050)".
template <typename Iterator>
struct CalibrationAnalysisParser : qi::grammar<Iterator, CalibrationAnalysis(), ascii::space_type>
{
  CalibrationAnalysisParser() : CalibrationAnalysisParser::base_type(start) {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;

    name_string %= lexeme[+(char_ - ',' - '"' - '}' - '{' - ')' - '(')];

    start = lit("Analysis")
      >> '('
      >> name_string >> ','
      >> name_string >> ','
      >> name_string
      >> ')'
      >> lit('{')
      >> lit('}')
      ;
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, CalibrationAnalysis(), ascii::space_type> start;
};

// Parse a list of the analyses (more than one per file, man!)
template <typename Iterator>
struct CalibrationAnalysisVectorParser : qi::grammar<Iterator, vector<CalibrationAnalysis>(), ascii::space_type>
{
  CalibrationAnalysisVectorParser() : CalibrationAnalysisVectorParser::base_type(start)
    {
      CalibrationAnalysisParser<Iterator> caParser;
      start = *caParser;
    }

    qi::rule<Iterator, vector<CalibrationAnalysis>(), ascii::space_type> start;
};

//
//
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

  BOOST_FUSION_ADAPT_STRUCT(
			    BinBoundary,
			    (double, lowerBound)
			    (std::string, variableName)
			    (double, upperBound)
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

    start = lit("sys")
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

    start = lit("central_value")
      >> '('
      >> double_ >> ","
      >> double_
      >> ')'
      ;
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, CentralValue(), ascii::space_type> start;
};

// Parse  a bin boundary: "35 < pt < 50"
template <typename Iterator>
struct BinBoundaryParser : qi::grammar<Iterator, BinBoundary(), ascii::space_type>
{
  BinBoundaryParser() : BinBoundaryParser::base_type(start) {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using qi::double_;

    name_string %= lexeme[+(char_ - ',' - '"')];

    start = 
      double_
      >> name_string
      >> double_
      ;

  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, BinBoundary(), ascii::space_type> start;
};

///////////////////////////////////////


namespace BTagCombination
{

  //
  // Parse the input text as a list of calibration inputs
  //
  vector<CalibrationAnalysis> Parse(const string &inputText)
  {
    CalibrationAnalysisVectorParser<string::const_iterator> cavParser;
    //vector<CalibrationAnalysis> result;

    string::const_iterator iter = inputText.begin();
    string::const_iterator end = inputText.end();

    //bool didit = phrase_parse(iter, end,
    //cavParser, ascii::space, result);
    CalibrationAnalysisParser<string::const_iterator> caParser;
    CalibrationAnalysis result;
    bool didit = phrase_parse(iter, end,
			      caParser, ascii::space, result);

    //
    // See if there were any errors doing the parse.
    //

    if (!didit)
      throw new runtime_error ("Unable to parse!");

    //if (iter != end)
    //  throw new runtime_error ("Did not parse the complete input text!");

    vector<CalibrationAnalysis> dude;
    dude.push_back(result);
    return dude;
#ifdef notyet
    string input = "sys( JES, 22%)";
  
    SystematicErrorParser<string::const_iterator> sErrP;
    SystematicError err;
    using ascii::space;

    string::const_iterator iter = input.begin();
    string::const_iterator end = input.end();
    bool r = phrase_parse(iter, end, sErrP, space, err);

    if (!r) {
      cout << "Error parsing the input!" << endl;
    } else {
      cout << "Got a parse" << endl;
      cout << "  name: " << err.name << endl;
      cout << "  value " << err.val << endl;
    }

    cout << "hi" << endl;
#endif
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
