#ifndef MERA_TENSOREVAL_H
#define MERA_TENSOREVAL_H

#include "TensorSrep.h"
#include "Tensor.h"

namespace Mera {

template<typename ComplexOrRealType>
class TensorEval {

	typedef TensorSrep TensorSrepType;

public:

	typedef Tensor<ComplexOrRealType> TensorType;
	typedef typename TensorType::VectorSizeType VectorSizeType;
	typedef typename PsimagLite::Vector<TensorType*>::Type VectorTensorType;
	typedef std::pair<SizeType, SizeType> PairSizeType;
	typedef PsimagLite::Vector<PairSizeType>::Type VectorPairSizeType;
	typedef typename PsimagLite::Vector<ComplexOrRealType>::Type VectorType;

	TensorEval(PsimagLite::String srep,const VectorTensorType& vt)
	    : tensorSrep_(srep), data_(vt)
	{}

	ComplexOrRealType eval() const
	{
		SizeType total = tensorSrep_.maxTag('s') + 1;
		VectorSizeType summed(total,0);
		VectorPairSizeType whereSummed(total,PairSizeType(0,0));
		VectorSizeType dimensions(total,0);

		prepare(whereSummed,dimensions);

		VectorSizeType free;
		ComplexOrRealType sum = 0.0;
		do {
			sum += evalInternal(summed,whereSummed,free);
			std::cerr<<summed;
			std::cerr<<"---------\n";
		} while (nextIndex(summed,dimensions));

		return sum;
	}

	static bool nextIndex(VectorSizeType& summed,
	                      const VectorSizeType& dimensions)
	{
		for (SizeType i = 0; i < summed.size(); ++i) {
			summed[i]++;
			if (summed[i] < dimensions[i]) break;
			if (i +1 == summed.size()) return false;
			summed[i] = 0;
		}

		return true;
	}

private:

	void prepare(VectorPairSizeType& whereSummed,
	             VectorSizeType& dimmensions) const
	{
		SizeType ntensors = tensorSrep_.size();
		for (SizeType i = 0; i < ntensors; ++i) {
			SizeType id = tensorSrep_(i).id();
			SizeType mid = idNameToIndex(tensorSrep_(i).name(),id);
			SizeType ins = tensorSrep_(i).ins();
			for (SizeType j = 0; j < ins; ++j) {
				if (tensorSrep_(i).legType(j,TensorStanza::INDEX_DIR_IN) !=
				        TensorStanza::INDEX_TYPE_SUMMED) continue;
				SizeType sIndex = tensorSrep_(i).legTag(j,TensorStanza::INDEX_DIR_IN);
				assert(sIndex < whereSummed.size());
				whereSummed[sIndex] = PairSizeType(i,j);
				assert(sIndex < data_[mid]->args());
				dimmensions[sIndex] = data_[mid]->argSize(j);
			}

			SizeType outs = tensorSrep_(i).outs();
			for (SizeType j = 0; j < outs; ++j) {
				if (tensorSrep_(i).legType(j,TensorStanza::INDEX_DIR_OUT) !=
				        TensorStanza::INDEX_TYPE_SUMMED) continue;
				SizeType sIndex = tensorSrep_(i).legTag(j,TensorStanza::INDEX_DIR_OUT);
				whereSummed[sIndex] = PairSizeType(i,j+ins);
				dimmensions[sIndex] = data_[mid]->argSize(j+ins);
			}
		}
	}

	ComplexOrRealType evalInternal(const VectorSizeType& summed,
	                               const VectorPairSizeType& whereSummed,
	                               const VectorSizeType& free) const
	{
		ComplexOrRealType prod = 1.0;
		SizeType ntensors = tensorSrep_.size();
		for (SizeType i = 0; i < ntensors; ++i) {
			prod *= evalThisTensor(tensorSrep_(i),summed,whereSummed,free);
		}

		return prod;
	}

	ComplexOrRealType evalThisTensor(const TensorStanza& ts,
	                                 const VectorSizeType& summed,
	                                 const VectorPairSizeType& whereSummed,
	                                 const VectorSizeType& free) const
	{
		SizeType id = ts.id();
		SizeType mid = idNameToIndex(ts.name(),id);
		SizeType ins = ts.ins();
		VectorSizeType args(data_[mid]->args());
		for (SizeType j = 0; j < ins; ++j) {
			SizeType index = ts.legTag(j,TensorStanza::INDEX_DIR_IN);

			switch (ts.legType(j,TensorStanza::INDEX_DIR_IN)) {

			case TensorStanza::INDEX_TYPE_SUMMED:
				assert(index < summed.size());
				assert(j < args.size());
				args[j] = summed[index];
				break;

			case TensorStanza::INDEX_TYPE_FREE:
				assert(index < free.size());
				assert(j < args.size());
				args[j] = free[index];
				break;

			case  TensorStanza::INDEX_TYPE_DUMMY:
				assert(j < args.size());
				args[j] = 0;
				break;
			}
		}

		return data_[mid]->operator()(args);
	}

	SizeType idNameToIndex(PsimagLite::String name, SizeType id) const
	{
		if (name != "u")
			throw PsimagLite::RuntimeError("idNameToIndex: only us for now, sorry\n");

		return id;
	}

	TensorSrepType tensorSrep_;
	const VectorTensorType& data_;
};
}
#endif // MERA_TENSOREVAL_H
