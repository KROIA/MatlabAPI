#pragma once
#include "MatlabAPI_base.h"
#include "Matrix.h"
//#include "TransferFunction.h"
#include <vector>
#include <string>

namespace MatlabAPI
{
	class MATLAB_API StateSpaceModel
	{
	public:
		enum C2DMethod
		{
			ZeroOrderHold,
			FirstOrderHold,
			Tustin,
			MatchedPoleZero,
			ImpulseInvariant,
			PrewarpedTustin
		};
		StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D, const Matrix& x0, double timeStep, C2DMethod method);
		StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D, 
			const Matrix& Ad, const Matrix& Bd, const Matrix& Cd, const Matrix& Dd,
			const Matrix& x0, double timeStep, C2DMethod method);
		StateSpaceModel(const StateSpaceModel& other);
		~StateSpaceModel();

		
		void processTimeStep(const Matrix& u);
		void processTimeStepApproxContinuesEuler(const Matrix& u);
		void processTimeStepApproxContinuesBilinear(const Matrix& u);
		void processTimeStepApproxContinuesRk4(const Matrix& u);

		void setState(const Matrix& x);
		const Matrix& getState() const { return x; }
		const Matrix& getOutput() const { return y; }

		const Matrix& getA() const  { return A; }
		const Matrix& getB() const  { return B; }
		const Matrix& getC() const  { return C; }
		const Matrix& getD() const  { return D; }

		const Matrix& getAd() const { return Ad; }
		const Matrix& getBd() const { return Bd; }
		const Matrix& getCd() const { return Cd; }
		const Matrix& getDd() const { return Dd; }

		double getTimeStep() const { return timeStep; }
		C2DMethod getC2DMethod() const { return c2dMethod; }

		void reset() { x = x0; }
		void reset(const std::vector<double> &x0) 
		{ 
			if (x0.size() != this->x0.getRows())
				return;
			for (size_t i = 0; i < this->x0.getRows() && i < x0.size(); ++i)
				this->x0(i, 0) = x0[i];
			reset();
		}

		size_t getInputCount() const { return B.getCols(); }
		size_t getOutputCount() const { return C.getRows(); }

		std::string toString() const;

		// Stream operator
		friend std::ostream& operator<<(std::ostream& os, const StateSpaceModel& model);


		static std::string c2dMethodToString(C2DMethod method)
		{
			using namespace std::string_literals;
			switch (method)
			{
			case ZeroOrderHold:    return "Zero-Order Hold"s;
			case FirstOrderHold:   return "First-Order Hold"s;
			case Tustin:           return "Tustin (Bilinear Transformation)"s;
			case MatchedPoleZero:  return "Matched Pole-Zero"s;
			case ImpulseInvariant: return "Impulse Invariant"s;
			case PrewarpedTustin:  return "Prewarped Tustin"s;      
			}
			return "Unknown Method"s;
		}
		static std::string c2dMethodToMatlabString(C2DMethod method)
		{
			using namespace std::string_literals;
			switch (method)
			{
			case ZeroOrderHold:    return "zoh"s;
			case FirstOrderHold:   return "foh"s;
			case Tustin:           return "tustin"s;
			case MatchedPoleZero:  return "matched"s;
			case ImpulseInvariant: return "impulse"s;
			case PrewarpedTustin:  return "prewarp"s;
			}
			return "Unknown Method"s;
		}
	private:
		Matrix A; // System matrix
		Matrix B; // Input matrix
		Matrix C; // Output matrix
		Matrix D; // Feedthrough (or direct transmission) matrix
		Matrix x0; // Initial state vector
		Matrix x;  // Current state vector
		Matrix y;  // Current output vector

		Matrix Ad; // System matrix
		Matrix Bd; // Input matrix
		Matrix Cd; // Output matrix
		Matrix Dd; // Feedthrough (or direct transmission) matrix
		double timeStep; // Time step for discrete model
		C2DMethod c2dMethod;
	};
}