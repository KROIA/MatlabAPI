#pragma once
#include "MatlabAPI_base.h"
#include "MatlabArray.h"
#include "math/Matrix.h"
#include <unordered_map>

struct engine; // Forward declaration of mxArray
typedef struct engine Engine;

namespace MatlabAPI
{
	class MATLAB_API MatlabEngine
	{
		friend class MatlabArray;
		MatlabEngine(const char* startcmd);
		~MatlabEngine();
	public:
		static bool instantiate();
		static bool instantiate(const char* startcmd, int retryCount = 10);
		static bool destroy();
		static bool isInstantiated();

		static MatlabEngine* getInstance();
		

		static int eval(const char* command);

		static bool addVariable(MatlabArray* var);
		static bool removeVariable(const std::string& name);
		static MatlabArray* getVariable(const std::string& name);
		static Matrix getMatrix(const std::string& name);
		static std::vector<std::string> listVariables();


		static double getDiscreteTimeStep();
		static void setDiscreteTimeStep(double dt);
	private:

		static Engine* getEngine();
		static bool updateVariableFromEngine(MatlabArray* var);
		static bool sendVariableToEngine(MatlabArray* var);

		static void err_matlabNotStarted();

		double m_discreteTimeStep = 0.01;
		std::unordered_map<std::string, MatlabArray*> m_variables; // map of variable name to MatlabArray
	};
}