#include "MatlabEngine.h"
#include "MatlabAPI_debug.h"
#include "engine.h" // Matlab Engine API
#include <QThread>

namespace MatlabAPI
{
	static Engine* s_engine = nullptr; // Matlab Engine instance
	static MatlabEngine* s_instance = nullptr; // singleton instance


	MatlabEngine::MatlabEngine(const char* startcmd)
	{
		s_engine = engOpen(startcmd); // open a MATLAB engine
		if (!s_engine)
		{
			Logger::logError("Can't start MATLAB engine");
		}
		else
		{
			Logger::log("MATLAB engine started successfully", Log::info, Log::Colors::green);
		}
	}
	MatlabEngine::~MatlabEngine()
	{
		if (s_engine)
		{
			engClose(s_engine);
			s_engine = nullptr;
			Logger::log("MATLAB engine closed", Log::info, Log::Colors::yellow);
		}

		// clean up variables
		for (auto& pair : m_variables)
		{
			if (pair.second)
				delete pair.second;
		}
	}


	bool MatlabEngine::instantiate()
	{
		return instantiate("");
	}
	bool MatlabEngine::instantiate(const char* startcmd, int retryCount)
	{
		do {
			if (s_instance != nullptr)
				return true; // already instantiated
			s_instance = new MatlabEngine(startcmd);
			if (s_engine == nullptr)
			{
				delete s_instance;
				s_instance = nullptr;
				Logger::logWarning("Retrying to start MATLAB engine...");
				QThread::msleep(1000); // wait a bit before retrying
			}
		} while (s_engine == nullptr && --retryCount > 0);
		return true;
	}
	bool MatlabEngine::terminate()
	{
		if (s_instance == nullptr)
			return true; // already destroyed
		delete s_instance;
		s_instance = nullptr;
		return s_engine == nullptr;
	}
	bool MatlabEngine::isInstantiated()
	{
		return s_instance != nullptr;
	}

	MatlabEngine* MatlabEngine::getInstance()
	{
		return s_instance;
	}



	int MatlabEngine::eval(const char* command)
	{
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return -1;
		}
		int ret = engEvalString(s_engine, command);
		Logger::logDebug("Evaluated command: \n\"" + std::string(command) + "\"\n -> return code: " + std::to_string(ret));
		return ret;
	}


	bool MatlabEngine::addVariable(MatlabArray* var)
	{
		if (!var)
			return false;
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return false;
		}
		const std::string& name = var->getName();
		if (name.empty())
		{
			Logger::logError("Variable name is empty");
			return false;
		}
		auto it = s_instance->m_variables.find(name);
		if (it != s_instance->m_variables.end())
		{
			Logger::logWarning("Variable with name '" + name + "' already exists, overwriting.");
			mxArray* oldArray = it->second->release();
			if (oldArray)
				mxDestroyArray(oldArray);
			delete it->second;
			it->second = var;
		}
		else
		{
			var->m_owner = s_instance;
			s_instance->m_variables[name] = var;
		}
		if (engPutVariable(s_engine, name.c_str(), var->get()) != 0)
		{
			Logger::logError("Failed to put variable '" + name + "' into MATLAB engine.");
			return false;
		}
		return true;
	}
	bool MatlabEngine::removeVariable(const std::string& name)
	{
		if (name.empty())
			return false;
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return false;
		}
		auto it = s_instance->m_variables.find(name);
		if (it == s_instance->m_variables.end())
		{
			Logger::logWarning("Variable with name '" + name + "' does not exist.");
			return false;
		}
		if (engEvalString(s_engine, ("clear " + name).c_str()) != 0)
		{
			Logger::logError("Failed to clear variable '" + name + "' from MATLAB engine.");
			return false;
		}
		//mxArray* oldArray = it->second->release();
		//if (oldArray)
		//	mxDestroyArray(oldArray);
		delete it->second;
		s_instance->m_variables.erase(it);
		return true;
	}
	MatlabArray* MatlabEngine::getVariable(const std::string& name)
	{
		if (name.empty())
			return nullptr;
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return nullptr;
		}
		auto it = s_instance->m_variables.find(name);
		//if (it == s_instance->m_variables.end())
		//{
		//	Logger::logWarning("Variable with name '" + name + "' does not exist.");
		//	return nullptr;
		//}
		mxArray* arr = engGetVariable(s_engine, name.c_str());
		if (!arr)
		{
			Logger::logError("Failed to get variable '" + name + "' from MATLAB engine.");
			return nullptr;
		}
		if (it == s_instance->m_variables.end())
		{
			// create new MatlabArray to manage this variable
			MatlabArray* var = new MatlabArray(name, arr, false); // do not take ownership, as engGetVariable returns a copy
			var->m_owner = s_instance;
			s_instance->m_variables[name] = var;
			Logger::logDebug("Variable with name '" + name + "' did not exist, created new MatlabArray: "+ var->toString());
			return var;
		}
		it->second->overwrite(arr, false, true); // do not take ownership, as engGetVariable returns a copy
		Logger::logDebug("Variable with name '" + name + "' exists, updated MatlabArray: " + it->second->toString());
		return it->second;
	}
	Matrix MatlabEngine::getMatrix(const std::string& name)
	{
		return Matrix(getVariable(name));
	}

	std::vector<std::string> MatlabEngine::listVariables()
	{
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return {};
		}
		std::vector<std::string> names;
		for (const auto& pair : s_instance->m_variables)
		{
			names.push_back(pair.first);
		}
		return names;
	}

	double MatlabEngine::getDiscreteTimeStep()
	{
		if (s_instance == nullptr)
		{
			err_matlabNotStarted();
			return 0.0;
		}
		return s_instance->m_discreteTimeStep;
	}
	void MatlabEngine::setDiscreteTimeStep(double dt)
	{
		if (s_instance == nullptr)
		{
			err_matlabNotStarted();
			return;
		}
		s_instance->m_discreteTimeStep = dt;
	}


	Engine* MatlabEngine::getEngine()
	{
		return s_engine;
	}

	bool MatlabEngine::updateVariableFromEngine(MatlabArray* var)
	{
		if (!var)
			return false;
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return false;
		}
		const std::string& name = var->getName();
		if (name.empty())
		{
			Logger::logError("Variable name is empty");
			return false;
		}
		auto it = s_instance->m_variables.find(name);
		if (it == s_instance->m_variables.end())
		{
			Logger::logWarning("Variable with name '" + name + "' does not exist.");
			return false;
		}
		mxArray* arr = engGetVariable(s_engine, name.c_str());
		if (!arr)
		{
			Logger::logError("Failed to get variable '" + name + "' from MATLAB engine.");
			return false;
		}
		it->second->overwrite(arr, false, true); // do not take ownership, as engGetVariable returns a copy
		return true;
	}
	bool MatlabEngine::sendVariableToEngine(MatlabArray* var)
	{
		if (!var)
			return false;
		if (s_engine == nullptr)
		{
			err_matlabNotStarted();
			return false;
		}
		const std::string& name = var->getName();
		if (name.empty())
		{
			Logger::logError("Variable name is empty");
			return false;
		}
		auto it = s_instance->m_variables.find(name);
		if (it == s_instance->m_variables.end())
		{
			Logger::logWarning("Variable with name '" + name + "' does not exist.");
			return false;
		}
		if (engPutVariable(s_engine, name.c_str(), var->get()) != 0)
		{
			Logger::logError("Failed to put variable '" + name + "' into MATLAB engine.");
			return false;
		}
		return true;
	}


	void MatlabEngine::err_matlabNotStarted()
	{
		Logger::logError("MATLAB engine is not started, call MatlabEngine::instantiate() first.");
	}

}