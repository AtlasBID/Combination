//
// Utility routines for measurements
//

#include "Combination/MeasurementUtils.h"
#include "Combination/Measurement.h"

using namespace std;

namespace BTagCombination {

  TMatrixTSym<double> CalcCovarMatrixUsingRho (const vector<Measurement*> measurements)
  {
    TMatrixTSym<double> W(measurements.size());
    int i_meas_row = 0;
    for (vector<Measurement*>::const_iterator imeas = measurements.begin(); imeas != measurements.end(); imeas++, i_meas_row++) {
      Measurement *m(*imeas);
      int i_meas_row2 = i_meas_row;
      for (vector<Measurement*>::const_iterator imeas2 = imeas; imeas2 != measurements.end(); imeas2++, i_meas_row2++) {
	Measurement *m2(*imeas2);
	W(i_meas_row, i_meas_row2) = m->Covar(m2);
	W(i_meas_row2, i_meas_row) = W(i_meas_row, i_meas_row2);
      }
    }
    return W;
  }

}
