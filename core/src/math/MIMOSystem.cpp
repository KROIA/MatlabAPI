#include "math/MIMOSystem.h"
#include "MatlabEngine.h"


namespace MatlabAPI
{
	MIMOSystem::MIMOSystem(const std::vector<std::vector<TransferFunction>>& systemMatrix)
	{
		size_t rows = systemMatrix.size();
		size_t cols = 0;
		for (const auto& row : systemMatrix)
		{
			cols = std::max(cols, row.size());
		}
		m_numOutputs = rows;
		m_numInputs = cols;
		m_systemMatrix = new TransferFunction[rows * cols];
		for (size_t r = 0; r < rows; r++)
		{
			for (size_t c = 0; c < cols; c++)
			{
				if (c < systemMatrix[r].size())
					m_systemMatrix[r * cols + c] = systemMatrix[r][c];
				else
					m_systemMatrix[r * cols + c] = TransferFunction::ZERO;
			}
		}
	}
	MIMOSystem::MIMOSystem(const MIMOSystem& other)
		: m_numInputs(other.m_numInputs)
		, m_numOutputs(other.m_numOutputs)
		, m_systemMatrix(new TransferFunction[other.m_numInputs * other.m_numOutputs])
	{
		memcpy(m_systemMatrix, other.m_systemMatrix, sizeof(TransferFunction) * other.m_numInputs * other.m_numOutputs);
	}
	MIMOSystem::MIMOSystem(MIMOSystem&& other) noexcept
		: m_numInputs(other.m_numInputs)
		, m_numOutputs(other.m_numOutputs)
		, m_systemMatrix(other.m_systemMatrix)
	{
		other.m_numInputs = 0;
		other.m_numOutputs = 0;
		other.m_systemMatrix = nullptr;
	}
	MIMOSystem::~MIMOSystem()
	{
		delete[] m_systemMatrix;
	}

	const TransferFunction& MIMOSystem::getTransferFunction(size_t inputIndex, size_t outputIndex) const
	{
		if (inputIndex >= m_numInputs || outputIndex >= m_numOutputs)
		{
			throw std::out_of_range("Input or output index out of range");
		}
		return m_systemMatrix[outputIndex * m_numInputs + inputIndex];
	}
	TransferFunction& MIMOSystem::getTransferFunction(size_t inputIndex, size_t outputIndex)
	{ 
		if (inputIndex >= m_numInputs || outputIndex >= m_numOutputs)
		{
			throw std::out_of_range("Input or output index out of range");
		}
		return m_systemMatrix[outputIndex * m_numInputs + inputIndex];
	}

	MIMOSystem& MIMOSystem::operator=(const MIMOSystem& other)
	{
		if (this != &other)
		{
			delete[] m_systemMatrix;
			m_numInputs = other.m_numInputs;
			m_numOutputs = other.m_numOutputs;
			m_systemMatrix = new TransferFunction[other.m_numInputs * other.m_numOutputs];
			memcpy(m_systemMatrix, other.m_systemMatrix, sizeof(TransferFunction) * other.m_numInputs * other.m_numOutputs);
		}
		return *this;
	}
	MIMOSystem& MIMOSystem::operator=(MIMOSystem&& other) noexcept
	{
		if (this != &other)
		{
			delete[] m_systemMatrix;
			m_numInputs = other.m_numInputs;
			m_numOutputs = other.m_numOutputs;
			m_systemMatrix = other.m_systemMatrix;

			other.m_numInputs = 0;
			other.m_numOutputs = 0;
			other.m_systemMatrix = nullptr;
		}
		return *this;
	}

	StateSpaceModel MIMOSystem::toStateSpaceModel(double timeStep) const
	{
		if (!MatlabEngine::isInstantiated())
		{
			throw std::runtime_error("Matlab engine is not instantiated.");
		}

		for (size_t i = 0; i < m_numOutputs; i++)
		{
			for (size_t j = 0; j < m_numInputs; j++)
			{
				std::string tfVarName = "tf_" + std::to_string(i) + "_" + std::to_string(j);
				m_systemMatrix[i * m_numInputs + j].putInMatlabWorkspace(tfVarName);
			}
		}

		// Construct the MIMO transfer function in MATLAB
		std::string mimoVarName = "mimo_sys_temp";
		std::string mimoExpr = mimoVarName + " = [";
		for (size_t i = 0; i < m_numOutputs; i++)
		{
			for (size_t j = 0; j < m_numInputs; j++)
			{
				std::string tfVarName = "tf_" + std::to_string(i) + "_" + std::to_string(j);
				mimoExpr += tfVarName;
				if (j < m_numInputs - 1)
					mimoExpr += ", ";
			}
			if (i < m_numOutputs - 1)
				mimoExpr += "; ";
		}
		mimoExpr += "];";
		MatlabEngine::eval(mimoExpr.c_str());
		MatlabEngine::eval(("mimo_sysd = c2d(" + mimoVarName + ", " + std::to_string(timeStep) + ", 'zoh'); [Ad,Bd,Cd,Dd] = ssdata(ss(mimo_sysd)); [A,B,C,D] = ssdata(ss(mimo_sys_temp));").c_str());

		Matrix Ad = MatlabEngine::getMatrix("Ad");
		Matrix Bd = MatlabEngine::getMatrix("Bd");
		Matrix Cd = MatlabEngine::getMatrix("Cd");
		Matrix Dd = MatlabEngine::getMatrix("Dd");

		Matrix A = MatlabEngine::getMatrix("A");
		Matrix B = MatlabEngine::getMatrix("B");
		Matrix C = MatlabEngine::getMatrix("C");
		Matrix D = MatlabEngine::getMatrix("D");

		return StateSpaceModel(A, B, C, D, Ad, Bd, Cd, Dd, Matrix(B.getRows(), 1), timeStep);
	}
}