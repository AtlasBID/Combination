//
//  Helper functiosn for dealing with bin boundary lists. Not something
// that folks outside this package will probably find useful.
//
#ifndef COMBINATION_BINBOUNDARYUTILS
#define COMBINATION_BINBOUNDARYUTILS

#include "Combination/Parser.h"

#include <stdexcept>
#include <vector>
#include <map>

namespace BTagCombination {
  class BinBoundaryUtils {
  public:
    // Given a bin boundary list, find the specification for the particular coordinate given as name.
    static inline CalibrationBinBoundary find_spec (const std::vector<CalibrationBinBoundary> &bin_spec, const std::string &name) {
      for (unsigned int i = 0; i < bin_spec.size(); i++) {
	if (bin_spec[i].variable == name)
	  return bin_spec[i];
      }
      throw std::runtime_error (("Unable to find bin with name '" + name + ".").c_str());
    }

    // return true if the two lists are the same. False otherwise.
    static inline bool compare_spec (const std::vector<CalibrationBinBoundary> &b1, const std::vector<CalibrationBinBoundary> &b2) {
      if (b1.size() != b2.size())
	return false;
      for (unsigned int ispec = 0; ispec < b1.size(); ispec++) {
	CalibrationBinBoundary b2b = find_spec(b2, b1[ispec].variable);
	if (b2b.lowvalue != b1[ispec].lowvalue)
	  return false;
	if (b2b.highvalue != b1[ispec].highvalue)
	  return false;
      }
      return true;
    }
  };

  // Helper class to hold onto info about each axis.
  class bin_boundaries {
  public:
    inline bin_boundaries()
      {}
    inline bin_boundaries(const bin_boundaries &old)
      : _axes(old._axes)
      {}

    // Save another axis in our list.
    void add_axis (const std::string &axis_name, const std::vector<double> &bin_edges)
    {
      _axes[axis_name] = bin_edges;
    }
    int size() const {return _axes.size();}
    
  protected:
    // Axis names and bin boundaries
    std::map<std::string, std::vector<double> > _axes;

    inline const std::pair<std::string, std::vector<double> > get_xaxis() const
    { return *(_axes.begin());}
    inline const std::pair<std::string, std::vector<double> > get_yaxis() const
    { return *(++_axes.begin());}

    int get_xaxis_bin (const std::vector<CalibrationBinBoundary> &bin_spec) const;
    int get_yaxis_bin (const std::vector<CalibrationBinBoundary> &bin_spec) const;
    int find_bin (const std::pair<std::string, std::vector<double> > &axis_info,
		  const std::vector<CalibrationBinBoundary> &bin_spec) const;
  };

  // Calculate the bin boundaries for a set of bins...
  bin_boundaries calcBoundaries (const CalibrationAnalysis &ana);
}

#endif
