#include "math/TransferFunction.h"
#include "MatlabEngine.h"

namespace MatlabAPI
{
	const TransferFunction TransferFunction::ZERO({ 0 }, { 1 });
	const TransferFunction TransferFunction::ONE({ 1 }, { 1 });
	const TransferFunction TransferFunction::S({ 1, 0 }, { 1 });
	const TransferFunction TransferFunction::INTEGRATOR({ 1 }, { 1, 0 });

	TransferFunction::TransferFunction()
	{
		this->operator=(ZERO);
	}
	TransferFunction::TransferFunction(const std::vector<double>& num, const std::vector<double>& den)
		: numerator(num)
		, denominator(den)
	{
		if (denominator.empty() || (denominator.size() == 1 && denominator[0] == 0.0))
		{
			throw std::invalid_argument("Denominator cannot be zero.");
		}
	}

	TransferFunction::TransferFunction(const TransferFunction& other)
		: numerator(other.numerator)
		, denominator(other.denominator)
	{

	}

	void TransferFunction::operator=(const TransferFunction& other)
	{
		if (this != &other)
		{
			numerator = other.numerator;
			denominator = other.denominator;
		}
	}

	StateSpaceModel TransferFunction::toStateSpaceModel(double timeStep, StateSpaceModel::C2DMethod methode) const
	{
		if (!MatlabEngine::isInstantiated())
		{
			throw std::runtime_error("Matlab engine is not instantiated.");
		}


		putInMatlabWorkspace("sys");
		MatlabEngine::eval(("sysd = c2d(sys, " + std::to_string(timeStep) + ", '"+ StateSpaceModel::c2dMethodToMatlabString(methode) + "'); [Ad,Bd,Cd,Dd] = ssdata(ss(sysd)); [A,B,C,D] = ssdata(ss(sys));").c_str());
		Matrix Ad = MatlabEngine::getMatrix("Ad");
		Matrix Bd = MatlabEngine::getMatrix("Bd");
		Matrix Cd = MatlabEngine::getMatrix("Cd");
		Matrix Dd = MatlabEngine::getMatrix("Dd");

		Matrix A = MatlabEngine::getMatrix("A");
		Matrix B = MatlabEngine::getMatrix("B");
		Matrix C = MatlabEngine::getMatrix("C");
		Matrix D = MatlabEngine::getMatrix("D");

		return StateSpaceModel(A, B, C, D, Ad, Bd, Cd, Dd, Matrix(B.getRows(), 1), timeStep, methode);
	}

	void TransferFunction::putInMatlabWorkspace(const std::string& varName) const
	{
		Matrix numMat(1, numerator.size());
		for (size_t i = 0; i < numerator.size(); i++)
		{
			numMat(0, i) = numerator[i];
		}

		Matrix denMat(1, denominator.size());
		for (size_t i = 0; i < denominator.size(); i++)
		{
			denMat(0, i) = denominator[i];
		}

		MatlabEngine::addVariable(numMat.toMatlabArray("num"));
		MatlabEngine::addVariable(denMat.toMatlabArray("den"));
		MatlabEngine::eval((varName + " = tf(num, den);").c_str());
	}

}