#include "math/StateSpaceModel.h"
#include "MatlabEngine.h"

namespace MatlabAPI
{
	StateSpaceModel::StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D, const Matrix& x0, double timeStep)
		: A(A)
		, B(B)
		, C(C)
		, D(D)
		, x0(x0)
		, x(x0)
		, y(C.getRows(), 1)
		, timeStep(timeStep)
	{
		if (!MatlabEngine::isInstantiated())
		{
			throw std::runtime_error("Matlab engine is not instantiated.");
		}
		
		MatlabEngine::addVariable(A.toMatlabArray("A"));
		MatlabEngine::addVariable(B.toMatlabArray("B"));
		MatlabEngine::addVariable(C.toMatlabArray("C"));
		MatlabEngine::addVariable(D.toMatlabArray("D"));
		MatlabEngine::eval(("sys = ss(A, B, C, D); sysd = c2d(sys, " + std::to_string(timeStep) + ", 'zoh'); [Ad,Bd,Cd,Dd] = ssdata(sysd);").c_str());
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
		const Matrix& x0, double timeStep)
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
		, timeStep(timeStep)
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
		, timeStep(other.timeStep)
	{

	}
	StateSpaceModel::~StateSpaceModel()
	{

	}


	void StateSpaceModel::processTimeStep(const Matrix& u)
	{
		x = Ad * x + Bd * u;
		y = Cd * x + Dd * u;
	}

	void StateSpaceModel::setState(const Matrix& x)
	{
		if (x.getRows() == this->x.getRows() && x.getCols() == this->x.getCols())
			this->x = x;
		else
			throw std::invalid_argument("State vector size mismatch.");
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