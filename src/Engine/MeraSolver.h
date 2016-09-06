#ifndef MERASOLVER_H
#define MERASOLVER_H
#include <iostream>
#include "MeraStep.h"
#include "ParametersForSolver.h"

namespace Mera {

class MeraSolver {

    typedef MeraStep<ParametersForSolver> MeraStepType;

public:

	MeraSolver(const ParametersForSolver& params)
	    : step_(params)
	{}

    void computeGroundState()
    {
        // Compute All Density Matricies - top to bottom using A or D operators
        SizeType qiter = 1;
        SizeType layers = 1;

        for (SizeType iter=0; iter<qiter; ++iter) {
            for (SizeType layer=0; layer<layers; ++layer) {
                step_.optimize(iter,layer);
            }
        }


    }

private:

    MeraStepType step_;
}; //class

} //namespace

#endif
