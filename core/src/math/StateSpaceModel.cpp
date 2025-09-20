#include "math/StateSpaceModel.h"
#include "MatlabEngine.h"

namespace MatlabAPI
{
	StateSpaceModel::StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D, const Matrix& x0)
		: A(A)
		, B(B)
		, C(C)
		, D(D)
		, x0(x0)
		, x(x0)
		, y(C.getRows(), 1)
	{
		if (!MatlabEngine::isInstantiated())
		{
			throw std::runtime_error("Matlab engine is not instantiated.");
		}
		double dt = MatlabEngine::getDiscreteTimeStep();
		MatlabEngine::addVariable(A.toMatlabArray("A"));
		MatlabEngine::addVariable(B.toMatlabArray("B"));
		MatlabEngine::addVariable(C.toMatlabArray("C"));
		MatlabEngine::addVariable(D.toMatlabArray("D"));
		MatlabEngine::eval(("sys = ss(A, B, C, D); sysd = c2d(sys, " + std::to_string(dt) + ", 'zoh'); [Ad,Bd,Cd,Dd] = ssdata(sysd);").c_str());
		Ad = MatlabEngine::getMatrix("Ad");
		Bd = MatlabEngine::getMatrix("Bd");
		Cd = MatlabEngine::getMatrix("Cd");
		Dd = MatlabEngine::getMatrix("Dd");

		if (x0.getRows() != A.getRows() || x0.getCols() != 1)
		{
			this->x0 = Matrix(A.getRows(), 1);
		}
		x = this->x0;
	}
	StateSpaceModel::StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D,
		const Matrix& Ad, const Matrix& Bd, const Matrix& Cd, const Matrix& Dd,
		const Matrix& x0)
		: A(A)
		, B(B)
		, C(C)
		, D(D)
		, x0(x0)
		, x(x0)
		, y(C.getRows(), 1)
		, Ad(Ad)
		, Bd(Bd)
		, Cd(Cd)
		, Dd(Dd)
	{
		
	}
	StateSpaceModel::StateSpaceModel(const StateSpaceModel& other)
		: A(other.A)
		, B(other.B)
		, C(other.C)
		, D(other.D)
		, x0(other.x0)
		, x(other.x)
		, Ad(other.Ad)
		, Bd(other.Bd)
		, Cd(other.Cd)
		, Dd(other.Dd)
	{

	}
	StateSpaceModel::~StateSpaceModel()
	{

	}

	/*StateSpaceModel StateSpaceModel::createFromTransferFunction(const TransferFunction& tf)
	{
		Matrix x0(0,0);
		return createFromTransferFunction(tf, x0);
	}
	StateSpaceModel StateSpaceModel::createFromTransferFunction(const TransferFunction &tf, const Matrix& x0)
	{
		if (!MatlabEngine::isInstantiated())
		{
			throw std::runtime_error("Matlab engine is not instantiated.");
		}

		const std::vector<double>& num = tf.getNumerator();
		Matrix numMat(1, num.size());
		for (size_t i = 0; i < num.size(); i++)
		{
			numMat(0, i) = num[i];
		}

		const std::vector<double>& den = tf.getDenominator();
		Matrix denMat(1, den.size());
		for (size_t i = 0; i < den.size(); i++)
		{
			denMat(0, i) = den[i];
		}

		double dt = MatlabEngine::getDiscreteTimeStep();
		MatlabEngine::addVariable(numMat.toMatlabArray("num"));
		MatlabEngine::addVariable(denMat.toMatlabArray("den"));
		MatlabEngine::eval(("sys = tf(num, den); sysd = c2d(sys, "+std::to_string(dt)+", 'zoh'); [Ad,Bd,Cd,Dd] = ssdata(ss(sysd)); [A,B,C,D] = ssdata(ss(sys));").c_str());
		Matrix Ad = MatlabEngine::getMatrix("Ad");
		Matrix Bd = MatlabEngine::getMatrix("Bd");
		Matrix Cd = MatlabEngine::getMatrix("Cd");
		Matrix Dd = MatlabEngine::getMatrix("Dd");

		Matrix A = MatlabEngine::getMatrix("A");
		Matrix B = MatlabEngine::getMatrix("B");
		Matrix C = MatlabEngine::getMatrix("C");
		Matrix D = MatlabEngine::getMatrix("D");

		return StateSpaceModel(A, B, C, D, x0);

	}*/


	void StateSpaceModel::processTimeStep(const Matrix& u)
	{
		y = Cd * x + Dd * u;
		x = Ad * x + Bd * u;
	}


	std::string StateSpaceModel::toString() const
	{
		std::string str = "StateSpaceModel:\n";
		str += "A = \n" + A.toString() + "\n";
		str += "B = \n" + B.toString() + "\n";
		str += "C = \n" + C.toString() + "\n";
		str += "D = \n" + D.toString() + "\n";
		str += "Ad = \n" + Ad.toString() + "\n";
		str += "Bd = \n" + Bd.toString() + "\n";
		str += "Cd = \n" + Cd.toString() + "\n";
		str += "Dd = \n" + Dd.toString() + "\n";
		str += "x0 = \n" + x0.toString() + "\n";
		str += "x = \n" + x.toString() + "\n";
		str += "y = \n" + y.toString() + "\n";
		return str;
	}

	// Stream operator
	std::ostream& operator<<(std::ostream& os, const StateSpaceModel& model)
	{
		os << model.toString();
		return os;
	}
}