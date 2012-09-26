//
// Utility routines for measurements
//

#include "Combination/MeasurementUtils.h"
#include "Combination/Measurement.h"

#include <iostream>

using namespace std;

namespace BTagCombination {

  //
  // Calculate the covariance matrix for a list of measurements by calculating the explicit "overlap"
  // between the errors of two measurements.
  //
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

  //
  // Calc the covar matrix one systematic error at a time. Check each one to make sure it can
  // be inverted.
  //
  TMatrixTSym<double> CalcCovarMatrixUsingComposition (const vector<Measurement*> measurements)
  {
    //
    // Get a list of all the systematic errors. Create a symmetric matrix to hold the covariance for each
    // of these systematic errors. Include one for the systematic error as well (it makes it easy to add it in
    // in the next step).
    //

    set<string> sysErrorNames;
    for (vector<Measurement*>::const_iterator imeas = measurements.begin(); imeas != measurements.end(); imeas++) {
      vector<string> errs ((*imeas)->GetSystematicErrorNames());
      sysErrorNames.insert(errs.begin(), errs.end());
    }

    map<string, TMatrixTSym<double> > sysLookup;
    for (set<string>::const_iterator itr = sysErrorNames.begin(); itr != sysErrorNames.end(); itr++) {
      // Funny insertion because default matrix has 0,0 elements, and can't deal with the "=" sign here.
      // Hopefully it won't occur later in the code that that is needed!
      sysLookup.insert(make_pair(*itr, TMatrixTSym<double>(measurements.size())));
    }

    sysLookup.insert(make_pair(string("stat"), TMatrixTSym<double>(measurements.size())));

    //
    // Go through all the measurements and build up each individual cov matrix.
    //

    int i_meas_row = 0;
    for (vector<Measurement*>::const_iterator imeas = measurements.begin(); imeas != measurements.end(); imeas++, i_meas_row++) {
      Measurement *m(*imeas);

      //
      // The statistical error is put straight into the stat matrix, no off-diagonal elements.
      //

      sysLookup["stat"](i_meas_row, i_meas_row) = m->statError() * m->statError();

      //
      // For each systeamtic error that this measurement knows about, fill in the slots in all
      // the sys matrices.
      //

      const vector<string> errs (m->GetSystematicErrorNames());
      for (vector<string>::const_iterator i_err = errs.begin(); i_err != errs.end(); i_err++) {

	TMatrixTSym<double> &covar(sysLookup[*i_err]);
	double sigma_m1 = m->GetSystematicErrorWidth(*i_err);

	//
	// Set the diagonal element for this measurement. This is just the error squared.
	//

	covar(i_meas_row, i_meas_row) = sigma_m1*sigma_m1;

	//
	// Now do the correlations. THis means going through all the other measurements and looking for any
	// time they have a shared error. If there is one, set the elements (symmetrically!).
	//

	int i_meas_row2 = i_meas_row;
	for (vector<Measurement*>::const_iterator imeas2 = imeas; imeas2 != measurements.end(); imeas2++, i_meas_row2++) {
	  Measurement *m2(*imeas2);
	  if (m2->hasSysError(*i_err)) {
	    double sigma_m2 = m2->GetSystematicErrorWidth(*i_err);
	    covar(i_meas_row, i_meas_row2) = sigma_m1*sigma_m2;
	    covar(i_meas_row2, i_meas_row) = sigma_m1*sigma_m2;
	  }
	}
      }
    }

    //
    // Check each matrix to make sure it is invertable, and sum them to get the total determinate.
    //

    TMatrixTSym<double> result (measurements.size());
    bool isfirst = true;
    for (map<string, TMatrixTSym<double> >::const_iterator i_c = sysLookup.begin(); i_c != sysLookup.end(); i_c++) {
      double d = i_c->second.Determinant();
      cout << "Determinate for sys error '" << i_c->first << "': " << d << endl;
      if (isfirst) {
	i_c->second.Print();
	isfirst = false;
      }
      if (d < 0) {
	cout << "Determinate for sys error '" << i_c->first << "' is less than zero: " << d << endl;
      }
      result = result + i_c->second;
    }

    return result;
  }
}
