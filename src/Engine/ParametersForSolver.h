#ifndef PARAMETERSFORSOLVER_H
#define PARAMETERSFORSOLVER_H
#include "Vector.h"

namespace Mera {

struct ParametersForSolver {

	typedef PsimagLite::Vector<SizeType>::Type VectorSizeType;

	ParametersForSolver(VectorSizeType& hTerms,
	                    SizeType h1,
	                    SizeType m1)
	    : hamiltonianTerm(hTerms),h(h1),m(m1)
	{}

	VectorSizeType hamiltonianTerm;
	SizeType h;
	SizeType m;
}; // struct ParametersForSolver
} // namespace Mera
#endif // PARAMETERSFORSOLVER_H
