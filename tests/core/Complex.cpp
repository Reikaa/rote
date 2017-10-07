/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
// NOTE: It is possible to simply include "rote.hpp" instead
#include "rote.hpp"
using namespace rote;

int 
main( int argc, char* argv[] )
{
    Initialize( argc, argv );
    mpi::Comm comm = mpi::COMM_WORLD;
    const Int commRank = mpi::CommRank( comm );

    try 
    {
        Complex<double> w( 0, 0 );
        if( commRank == 0 )
            std::cout << "sqrt of " << w << " is " << Sqrt(w) << std::endl;

        double maxLocalError = 0;
        double maxLocalRelError = 0;
        const Int numTests = 1000;
        for( Int j=0; j<numTests; ++j )
        {
            w = SampleBall<Complex<double>>();
            Complex<double> sqrtW = Sqrt(w);
            const double error = Abs(sqrtW*sqrtW-w);
            const double relError = error/Abs(w);

            maxLocalError = std::max( maxLocalError, error );
            maxLocalRelError = std::max( maxLocalRelError, relError );
        }

        double maxError, maxRelError;
        mpi::Reduce( &maxLocalError, &maxError, 1, mpi::SUM, 0, comm );
        mpi::Reduce( &maxLocalRelError, &maxRelError, 1, mpi::SUM, 0, comm );
        if( commRank == 0 )
            std::cout << "Maximum error and relative error from " << numTests 
                      << " tests was " << maxError << " and " << maxRelError
                      << std::endl;
    }
    catch( std::exception& e ) { ReportException(e); }

    Finalize();
    return 0;
}

