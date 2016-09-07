#ifndef MERALAYER_H
#define MERALAYER_H
#include <cassert>
#include "Vector.h"
#include "Matrix.h"
#include "TensorLeg.h"

namespace Mera {

template<typename ParametersForSolverType>
class MeraLayer {

	typedef int TensorType;
	typedef TensorLeg TensorLegType_;
	typedef PsimagLite::Matrix<TensorLegType_*> MatrixTensorLegType;

	enum TensorTypeEnum {TENSOR_TYPE_W,TENSOR_TYPE_U};

	enum MeraArchitectureEnum {MERA_ARCH_1D_BINARY,MERA_ARCH_1D_TERNARY,
		                       MERA_ARCH_2D_2X2,MERA_ARCH_2D_3X3};

public:

	typedef TensorLegType_ TensorLegType;
	typedef std::pair<SizeType,TensorTypeEnum> PairSizeType;

	MeraLayer(const ParametersForSolverType& params, SizeType tau, SizeType sitesInLayer)
	    : params_(params),
	      tau_(tau),
	      sitesInLayer_(sitesInLayer),
	      outputSites_(0),
	      srep_("")
	{
		setMeraArchitecture(MERA_ARCH_1D_TERNARY);
		setUpdateOrder();
		createSrep(srep_);
	}

	~MeraLayer()
	{
		for (SizeType i = 0; i < mv_.n_row(); ++i) {
			for (SizeType j = 0; j < mv_.n_col(); ++j) {
				delete mv_(i,j);
				mv_(i,j) = 0;
			}
		}

		for (SizeType i = 0; i < mu_.n_row(); ++i) {
			for (SizeType j = 0; j < mu_.n_col(); ++j) {
				delete mu_(i,j);
				mu_(i,j) = 0;
			}
		}
	}

	SizeType size() const
	{
		return vecForUpdateOrder_.size();
	}

	SizeType outputSites() const { return outputSites_; }

	const PairSizeType& tensorToOptimize(SizeType i) const
	{
		assert(i < vecForUpdateOrder_.size());
		return vecForUpdateOrder_[i];
	}

	template<typename P>
	friend std::ostream& operator<<(std::ostream& os, const MeraLayer<P>& m)
	{
		os<<"tau="<<m.tau_<<"\n";
		os<<"sitesInThisLayer="<<m.sitesInLayer_<<"\n";
		os<<"vs="<<m.mv_.n_row()<<"\n";
		for (SizeType i = 0; i < m.mv_.n_row(); ++i) {
			os<<"isometry number "<<i<<" has sites ";
			for (SizeType j = 0; j < m.mv_.n_col(); ++j) {
				TensorLegType* ptr = m.mv_(i,j);
				if (!ptr) continue;
				PsimagLite::String val = (ptr->inOrOut == TensorLegType::OUT) ? " *" : " ";
				if (ptr->inOrOut == TensorLegType::IN && ptr->site >= m.sitesInLayer_)
					os<<val<<" o";
				else
					os<<val<<ptr->site;
			}

			os<<"\n";
		}

		os<<"us="<<m.mu_.n_row()<<"\n";
		for (SizeType i = 0; i < m.mu_.n_row(); ++i) {
			os<<"disentangler number "<<i<<" has sites ";
			for (SizeType j = 0; j < m.mu_.n_col(); ++j) {
				TensorLegType* ptr = m.mu_(i,j);
				if (!ptr) continue;
				PsimagLite::String val = (ptr->inOrOut == TensorLegType::OUT) ? " *" : " ";
				if (ptr->inOrOut == TensorLegType::IN && ptr->site >= m.sitesInLayer_)
					os<<val<<" o";
				else
					os<<val<<ptr->site;
			}

			os<<"\n";
		}

		os<<"srep="<<m.srep_<<"\n";
		os<<"-------------------\n";
		return os;
	}

private:

	MeraLayer(const MeraLayer&);

	MeraLayer& operator=(const MeraLayer&);

	void setUpdateOrder()
	{
		SizeType n = std::min(mu_.n_row(), mv_.n_row());
		vecForUpdateOrder_.resize(mu_.n_row() + mv_.n_row());

		for (SizeType i=0; i<n; ++i) {
			vecForUpdateOrder_[2*i] = PairSizeType(i,TENSOR_TYPE_W);
			vecForUpdateOrder_[2*i+1] = PairSizeType(i,TENSOR_TYPE_U);
		}

		int x = mu_.n_row() - mv_.n_row();
		if (x == 0) return;

		SizeType m = std::abs(x);
		TensorTypeEnum type = (x < 0) ? TENSOR_TYPE_W : TENSOR_TYPE_U;
		for (SizeType i=n; i<n+m; ++i)
			vecForUpdateOrder_[n+i] =  PairSizeType(i,type);
	}

	void setMeraArchitecture(MeraArchitectureEnum what)
	{
		if (what != MERA_ARCH_1D_TERNARY)
			throw PsimagLite::RuntimeError("MERA architecture not implemented\n");

		setMeraArchitecture1dTernary();
	}

	void setMeraArchitecture1dTernary()
	{
		SizeType type = (tau_ % 3);
		SizeType offset = 0;
		if (type == 0) offset = 1;
		if (type == 2) offset = 2;
		SizeType totalUs = (sitesInLayer_ + 1)/3;

		mu_.resize(totalUs,4); // (2,2) == (ins, outs)
		for (SizeType i = 0; i < totalUs; ++i) {
			SizeType first = 3*i + offset - 1;
			if (i == 0 && type == 1) first = sitesInLayer_;
			mu_(i,0) = new TensorLeg(first,TensorLeg::TensorMeraType::IN);
			mu_(i,1) = new TensorLeg(3*i + offset,TensorLeg::TensorMeraType::IN);
			mu_(i,2) = new TensorLeg(first,TensorLeg::TensorMeraType::OUT);
			mu_(i,3) = new TensorLeg(3*i + offset,TensorLeg::TensorMeraType::OUT);
		}

		SizeType totalVs = (sitesInLayer_ + 1)/3 + type - 1;
		offset = 2 - type;
		mv_.resize(totalVs,4); // (3,1) == (ins, outs)
		for (SizeType i = 0; i < totalVs; ++i) {
			if (3*i + offset >= sitesInLayer_) break;
			SizeType first = 3*i + offset - 1;
			if (i == 0 && type == 2) first = sitesInLayer_;
			mv_(i,0) = new TensorLeg(first,TensorLeg::TensorMeraType::IN);
			mv_(i,1) = new TensorLeg(3*i + offset,TensorLeg::TensorMeraType::IN);
			mv_(i,2) = new TensorLeg(3*i + offset + 1,TensorLeg::TensorMeraType::IN);
			mv_(i,3) = new TensorLeg(i,TensorLeg::TensorMeraType::OUT);
			outputSites_++;
		}
	}

	void createSrep(PsimagLite::String& srep) const
	{
		for (SizeType i = 0; i < vecForUpdateOrder_.size(); ++i) {
			SizeType s = vecForUpdateOrder_[i].first;
			TensorTypeEnum t = vecForUpdateOrder_[i].second;
			PsimagLite::String ts = (t == TENSOR_TYPE_U) ? "u" :"w";
			srep += ts + ":" + ttos(tau_) + ":" + ttos(s);
			const MatrixTensorLegType& m = (t == TENSOR_TYPE_U) ? mu_ : mv_;
			SizeType ins = findInsOrOuts(m,s,TensorLegType::IN);
			SizeType outs = findInsOrOuts(m,s,TensorLegType::OUT);
			srep += ":" + ttos(ins) + ":" + ttos(outs);
			for (SizeType j = 0; j < ins; ++j)
				srep += ":I" + ttos(j);
			for (SizeType j = 0; j < outs; ++j)
				srep += ":O" + ttos(j);
			srep += ";";
		}
	}

	SizeType findInsOrOuts(const MatrixTensorLegType& m,
	                       SizeType ind,
	                       TensorLegType::TensorMeraType type) const
	{
		SizeType sum = 0;
		SizeType cols = m.n_col();
		for (SizeType j = 0; j < cols; ++j) {
			if (m(ind,j)->inOrOut == type) sum++;
		}

		return sum;
	}

	const ParametersForSolverType& params_;
	SizeType tau_;
	SizeType sitesInLayer_;
	SizeType outputSites_;
	PsimagLite::String srep_;
	std::vector<TensorType> u_; // disentagler
//	std::vector<TensorType> w_; // isometries
//	std::vector<TensorType> rho_; // density matrices
	std::vector<PairSizeType> vecForUpdateOrder_;
	MatrixTensorLegType mu_;
	MatrixTensorLegType mv_;
}; //class

} //namespace

#endif // MERALAYER_H
