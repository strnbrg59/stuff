#include <math.h>
#include "rcm.h"

/** The columns of candidates are what we add to the RHS of the regression.
 *  Each iteration, we add one more column.  The column we add is the one
 *  with the highest correlation (in absolute value) to the residual from 
 *  the previous regression.
 *
 *  The return value is a ptr to the first model whose R^2 exceeds arg 
 *  stopoutRsqr, or NULL if no model exceeds that R^2.
*/
Regression* stepwiseRegression( const Vd& y_, const Md& candidates_, 
                                const Regression* model_, double stopoutRsqr )
{
    Regression* model = model_->clone();

    int n = y_.size();
    int max_k = candidates_.cols();

    // Start the RHS with a column of ones.
    Md rhs = drho(n,1,Md(1.0));
    Regression* temp = model;
    model = model->clone(y_, rhs);
    delete temp;
    cout << "R^2 = " << model->r2() << '\n';

    for( int i=0;i<max_k;i++ )
    {
        // Find a next column for rhs.  Cycle through all the candidate columns.
        // FIXME: You should keep track of which ones you've already used, so
        // you don't waste time finding the correlation of the residuals with
        // those.
        double highestCorrel = -1.0;
        int indexOfMostCorrelated;
        for( int j=0;j<max_k;j++ )
        {        
            double corr = fabs(correl(model->res(), take(candidates_,j)));
            //cout << "candidate correl: " << corr << '\n';
            if( corr > highestCorrel )
            {
                indexOfMostCorrelated = j;
                highestCorrel = corr;
            }
        }
        
        cout << "Introducing column " << indexOfMostCorrelated
             << " with absolute correlation " << highestCorrel << '\n';
        rhs = catcols( rhs, take(candidates_,indexOfMostCorrelated));

        temp = model;
        model = model->clone(y_, rhs);
        delete temp;
        cout <<  "R^2 = " << model->r2() << '\n';

        if( model->r2() > stopoutRsqr )
        {
            return model;
        }
    }

    return NULL; // Gets here if no regression produced R^2 above stopoutRsqr.
                 // Calling function better check for this.
}









