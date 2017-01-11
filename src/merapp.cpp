/*
Copyright (c) 2016, UT-Battelle, LLC

MERA++, Version 0.

This file is part of MERA++.
MERA++ is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
MERA++ is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with MERA++. If not, see <http://www.gnu.org/licenses/>.
*/
/** \ingroup MERA */
/*@{*/

/*! \file main.cpp
 *
 *  The MERA main driver
 *
 */

#include <unistd.h>
#include "MeraEnviron.h"
#include <fstream>
#include "MeraToTikz.h"
#include "Version.h"
#include "DimensionSrep.h"
#include "MeraBuilder.h"

void usageMain(const PsimagLite::String& str)
{
	throw PsimagLite::RuntimeError(str);
}

template<typename ComplexOrRealType>
void main1(const Mera::MeraBuilder<ComplexOrRealType>& builder,
           const Mera::ParametersForSolver<ComplexOrRealType>& params)
{
	PsimagLite::String srep = builder();
	Mera::DimensionSrep dimSrep(srep,params.h,params.m);
	PsimagLite::String dsrep = dimSrep();
	PsimagLite::String hString = ttos(params.h);
	dsrep += "h0(" + hString + "," + hString + "|" + hString + "," + hString + ")";

	Mera::MeraEnviron<ComplexOrRealType> environ(builder,params,dsrep);
	std::cout<<params;
	std::cout<<"DimensionSrep="<<dsrep<<environ.dimensionSrep();
	// add output u1000 to be used by unitary condition checking
	std::cout<<"u1000(1,1)\n";
	SizeType iterMera = 5;
	std::cout<<"MeraOptions=none\n";
	std::cout<<"IterMera="<<iterMera<<"\n";
	std::cout<<"MERA="<<srep<<"\n";

	std::cout<<environ.environs();
}

template<typename VectorType>
void fillHamTerms(VectorType& v,
                  PsimagLite::String terms)
{
	PsimagLite::Vector<PsimagLite::String>::Type tokens;
	PsimagLite::tokenizer(terms,tokens,",");
	if (tokens.size() & 1) {
		std::cerr<<"-s site0,value0,site1,value1,... expected\n";
		throw PsimagLite::RuntimeError("fillHamTerms\n");
	}

	for (SizeType i = 0; i < tokens.size(); i += 2) {
		SizeType ind = atoi(tokens[i].c_str());
		assert(ind < v.size());
		assert(i + 1 < tokens.size());
		v[ind] = atof(tokens[i+1].c_str());;
	}
}

int main(int argc, char **argv)
{
	// check for complex or real  here FIXME
	typedef Mera::ParametersForSolver<double> MeraParametersType;
	typedef Mera::MeraBuilder<double> MeraBuilderType;

	int opt = 0;
	bool versionOnly = false;
	bool buildOnly = false;
	SizeType sites = 0;
	SizeType arity = 2;
	SizeType dimension = 1;
	MeraParametersType::VectorType hamTerms;
	SizeType h = 0;
	SizeType m = 0;
	PsimagLite::String strUsage(argv[0]);
	strUsage += " -n sites -a arity -d dimension -h hilbertSize [-m m] ";
	strUsage += "| -S srep | -V\n";
	strUsage += "-h hilbertSize is always mandatory\n";

	while ((opt = getopt(argc, argv,"n:a:d:h:m:s:bV")) != -1) {
		switch (opt) {
		case 'n':
			sites = atoi(optarg);
			assert(sites > 1);
			hamTerms.resize(sites - 1,1.0);
			break;
		case 'a':
			arity = atoi(optarg);
			break;
		case 'd':
			dimension = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
		case 'm':
			m = atoi(optarg);
			break;
		case 's':
			if (hamTerms.size() == 0) {
				std::cerr<<argv[0]<<": option -s must be after -n\n";
				return 1;
			}

			std::fill(hamTerms.begin(),hamTerms.end(),0.0);
			fillHamTerms(hamTerms,optarg);
			break;
		case 'b':
			buildOnly = true;
			break;
		case 'V':
			versionOnly = true;
			break;
		default:
			usageMain(strUsage);
			return 1;
		}
	}

	std::cerr<<"#"<<argv[0]<<" version "<<MERA_VERSION<<"\n";

	if (versionOnly)
		return 0;

	// sanity checks here
	if (h == 0 || sites*arity*dimension == 0)
		usageMain(strUsage);

	// here build srep
	MeraBuilderType meraBuilder(sites,arity,dimension,hamTerms);

	std::cout<<"Sites="<<sites<<"\n";

	if (buildOnly) {
		std::cout<<"Srep="<<meraBuilder()<<"\n";
		std::cerr<<argv[0]<<": Stoping here because of -b (build only). ";
		std::cerr<<"Not computing environments\n";
		return 1;
	}

	std::cout<<"#"<<argv[0]<<" version "<<MERA_VERSION<<"\n";

	MeraParametersType params(hamTerms,h,m);
	main1(meraBuilder,params);
}
