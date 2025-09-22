#pragma once
#include "MatlabAPI_base.h"
#include "MatlabArray.h"
#include "math/Matrix.h"
#include <unordered_map>

#ifdef MATLAB_API_USE_CPP_API
namespace matlab {
	namespace engine
	{
		class MATLABEngine;
	}
}
#else
struct engine; // Forward declaration of mxArray
typedef struct engine Engine;
#endif

namespace MatlabAPI
{
	class MATLAB_API MatlabEngine
	{
		friend class MatlabArray;
#ifdef MATLAB_API_USE_CPP_API
		MatlabEngine(const std::u16string& startcmd);
#else
		MatlabEngine(const char* startcmd);
#endif
		~MatlabEngine();
	public:
		static bool instantiate();
#ifdef MATLAB_API_USE_CPP_API
		static bool instantiate(const std::u16string& startcmd, int retryCount = 10);
#else
		static bool instantiate(const std::u16string& startcmd, int retryCount = 10);
		static bool instantiate(const char* startcmd, int retryCount = 10);
#endif
		static bool terminate();
		static bool isInstantiated();

		static MatlabEngine* getInstance();
		

		static int eval(const char* command);

		static bool addVariable(MatlabArray* var);
		static bool removeVariable(const std::string& name);
		static MatlabArray* getVariable(const std::string& name);
		static Matrix getMatrix(const std::string& name);
		static std::vector<std::string> listVariables();
#ifdef MATLAB_API_USE_CPP_API
		static MatlabArray getProperty(MatlabArray* array, const std::u16string& property);
#endif


		static double getDiscreteTimeStep();
		static void setDiscreteTimeStep(double dt);
	private:
#ifdef MATLAB_API_USE_CPP_API
		static const std::unique_ptr<matlab::engine::MATLABEngine> &getEngine();
#else
		static Engine* getEngine();
#endif

		static bool updateVariableFromEngine(MatlabArray* var);
		static bool sendVariableToEngine(MatlabArray* var);

		static void err_matlabNotStarted();

		double m_discreteTimeStep = 0.01;
		std::unordered_map<std::string, MatlabArray*> m_variables; // map of variable name to MatlabArray
	};
}