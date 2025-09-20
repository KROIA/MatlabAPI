#pragma once
#include "MatlabAPI_base.h"
#include "TransferFunction.h"
#include <vector>

namespace MatlabAPI
{
	class MATLAB_API MIMOSystem
	{
	public:
		MIMOSystem(const std::vector<std::vector<TransferFunction>>& systemMatrix);
		MIMOSystem(const MIMOSystem& other);
		MIMOSystem(MIMOSystem&& other) noexcept;
		~MIMOSystem();

		size_t getNumInputs() const { return m_numInputs; }
		size_t getNumOutputs() const { return m_numOutputs; }

		const TransferFunction& getTransferFunction(size_t inputIndex, size_t outputIndex) const;
		TransferFunction& getTransferFunction(size_t inputIndex, size_t outputIndex);

		MIMOSystem& operator=(const MIMOSystem& other);
		MIMOSystem& operator=(MIMOSystem&& other) noexcept;

		StateSpaceModel toStateSpaceModel() const;

	private:
		TransferFunction* m_systemMatrix = nullptr;
		size_t m_numInputs = 0;
		size_t m_numOutputs = 0;
	};
}