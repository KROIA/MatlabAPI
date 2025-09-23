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
		typedef void (StateSpaceModel::* ProcessTimeStepFunc)(const Matrix& u);
	public:
		/**
		 * @brief 
		 * Continuous to Discrete conversion methods used in the matlab c2d function
		 */
		enum C2DMethod
		{
			ZeroOrderHold,
			FirstOrderHold,
			Tustin,
			MatchedPoleZero,
			ImpulseInvariant,
			PrewarpedTustin
		};

		/**
		 * @brief 
		 * Integration solvers for continuous time state space models
		 * Euler: Forward Euler method
		 * Bilinear: Bilinear (Tustin) method
		 * Rk4: 4th-order Runge-Kutta method
		 * Discretized: Use the discretized version of the model (Ad, Bd, Cd, Dd), created using the specified C2DMethod
		 */
		enum IntegrationSolver
		{
			Euler,
			Bilinear,
			Rk4,
			Discretized
		};

		/**
		 * @brief Creates a StateSpaceModel from the continuous-time state space matrices A, B, C, D and the initial state x0.
		 * @param A System matrix
		 * @param B Input matrix
		 * @param C Output matrix
		 * @param D Feedthrough (or direct transmission) matrix
		 * @param x0 Initial state vector
		 * @param timeStep timestep in seconds used in the c2d conversion by matlab
		 * @param method C2DMethod used in the c2d conversion by matlab
		 */
		StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D, const Matrix& x0, double timeStep, C2DMethod method);
		
		/**
		 * @brief Creates a StateSpaceModel from both the continuous-time state space matrices A, B, C, D 
		 *        and the discrete-time state space matrices Ad, Bd, Cd, Dd and the initial state x0.
		 * @param A System matrix
		 * @param B Input matrix
		 * @param C Output matrix
		 * @param D Feedthrough (or direct transmission) matrix
		 * @param Ad Discrete system matrix
		 * @param Bd Discrete input matrix
		 * @param Cd Discrete output matrix
		 * @param Dd Discrete feedthrough (or direct transmission) matrix
		 * @param x0 Initial state vector
		 * @param timeStep timestep in seconds used in the c2d conversion by matlab
		 * @param method C2DMethod used in the c2d conversion by matlab
		 */
		StateSpaceModel(const Matrix& A, const Matrix& B, const Matrix& C, const Matrix& D, 
			const Matrix& Ad, const Matrix& Bd, const Matrix& Cd, const Matrix& Dd,
			const Matrix& x0, double timeStep, C2DMethod method);
		StateSpaceModel(const StateSpaceModel& other);
		~StateSpaceModel();

		void setIntegrationSolver(IntegrationSolver solver);
		IntegrationSolver getIntegrationSolver() const { return solver; }

		static void setDefaultIntegrationSolver(IntegrationSolver solver) { defaultSolver = solver; }
		static IntegrationSolver getDefaultIntegrationSolver() { return defaultSolver; }
		
		/**
		 * @brief Automatically process one time step using the selected integration solver
		 * @param u input of the system. Must be a column vector with size equal to the number of inputs of the system (B.cols)
		 */
		void processTimeStep(const Matrix& u)
		{
			(this->*processTimeStepFunc)(u);
		}

		/**
		 * @brief Explicitly process one time step using the discretized model
		 * @param u input of the system. Must be a column vector with size equal to the number of inputs of the system (B.cols)
		 */
		void processTimeStepDiscretized(const Matrix& u);

		/**
		 * @brief Explicitly process one time step using the Forward Euler method
		 * @param u input of the system. Must be a column vector with size equal to the number of inputs of the system (B.cols)
		 */
		void processTimeStepEuler(const Matrix& u);

		/**
		 * @brief Explicitly process one time step using the Bilinear (Tustin) method
		 * @param u input of the system. Must be a column vector with size equal to the number of inputs of the system (B.cols)
		 */
		void processTimeStepBilinear(const Matrix& u);

		/**
		 * @brief Explicitly process one time step using the 4th-order Runge-Kutta method
		 * @param u input of the system. Must be a column vector with size equal to the number of inputs of the system (B.cols)
		 */
		void processTimeStepRk4(const Matrix& u);

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

		static std::string integrationSolverToString(IntegrationSolver solver)
		{
			using namespace std::string_literals;
			switch (solver)
			{
			case Euler:        return "Forward Euler"s;
			case Bilinear:     return "Bilinear (Tustin)"s;
			case Rk4:          return "4th-order Runge-Kutta"s;
			case Discretized:  return "Discretized Model"s;
			}
			return "Unknown Solver"s;
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

		IntegrationSolver solver;
		ProcessTimeStepFunc processTimeStepFunc;
		static IntegrationSolver defaultSolver;
	};
}