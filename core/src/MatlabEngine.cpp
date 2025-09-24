#include "MatlabEngine.h"



#include "QApplication.h"
#include "MatlabAPI_debug.h"
#ifdef MATLAB_API_USE_CPP_API
#include "MatlabEngine.hpp"
#include "MatlabDataArray.hpp"
#include <windows.h>
#include <stdexcept>

#else
#include "engine.h" // Matlab Engine API
#endif
#include <QThread>



namespace MatlabAPI
{

	// Convert UTF-8 (char*) -> UTF-16 (std::u16string)
	std::u16string to_u16string(const char* input) {
		if (!input) return u"";

		int size_needed = MultiByteToWideChar(
			CP_UTF8,            // source encoding
			0,                  // flags
			input,              // input UTF-8 string
			-1,                 // null-terminated
			nullptr, 0          // get required size
		);

		if (size_needed <= 0) throw std::runtime_error("Failed MultiByteToWideChar");

		std::u16string output(size_needed - 1, 0); // exclude null terminator

		MultiByteToWideChar(
			CP_UTF8,
			0,
			input,
			-1,
			reinterpret_cast<wchar_t*>(&output[0]),
			size_needed
		);

		return output;
	}

	// Convert UTF-16 (std::u16string) -> UTF-8 (std::string)
	std::string to_utf8(const std::u16string& input) {
		if (input.empty()) return "";

		int size_needed = WideCharToMultiByte(
			CP_UTF8,
			0,
			reinterpret_cast<const wchar_t*>(input.data()),
			static_cast<int>(input.size()),
			nullptr, 0,
			nullptr, nullptr
		);

		if (size_needed <= 0) throw std::runtime_error("Failed WideCharToMultiByte");

		std::string output(size_needed, 0);

		WideCharToMultiByte(
			CP_UTF8,
			0,
			reinterpret_cast<const wchar_t*>(input.data()),
			static_cast<int>(input.size()),
			output.data(),
			size_needed,
			nullptr, nullptr
		);

		return output;
	}

#ifdef MATLAB_API_USE_CPP_API
	static std::unique_ptr<matlab::engine::MATLABEngine> s_engine = nullptr; // Matlab Engine instance (C++ API)
	
	// Read from a StreamBuffer into std::string
	std::string streamBufferToString(matlab::engine::StreamBuffer* buffer) {
		std::basic_istream<char16_t> in(buffer);
		std::u16string u16;
		std::getline(in, u16);   // or read all with std::istreambuf_iterator

		return to_utf8(u16);
	}

#else
	static Engine* s_engine = nullptr; // Matlab Engine instance
#endif	
	static MatlabEngine* s_instance = nullptr; // singleton instance



#ifdef MATLAB_API_USE_CPP_API
	MatlabEngine::MatlabEngine(const std::u16string& startcmd)
#else
	MatlabEngine::MatlabEngine(const char* startcmd)
#endif
	{
		// Start MATLAB engine
#ifdef MATLAB_API_USE_CPP_API
		//s_engine = matlab::engine::connectMATLAB(to_u16string(startcmd));
		if (startcmd.empty())
			s_engine = matlab::engine::startMATLAB(); // start a new MATLAB engine
		else
			s_engine = matlab::engine::connectMATLAB(startcmd);
#else
		s_engine = engOpen(startcmd); // open a MATLAB engine
#endif	
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
#ifdef MATLAB_API_USE_CPP_API
			matlab::engine::terminateEngineClient();
#else
			engClose(s_engine);
#endif	

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
#ifdef MATLAB_API_USE_CPP_API
		return instantiate(u"");
#else
		return instantiate("");
#endif
	}
#ifdef MATLAB_API_USE_CPP_API
	bool MatlabEngine::instantiate(const std::u16string& startcmd, int retryCount)
#else
	bool MatlabEngine::instantiate(const char* startcmd, int retryCount)
#endif
	{
		int initialRetryCount = retryCount;
		do {
			if (s_instance != nullptr)
				return true; // already instantiated
			try{
				s_instance = new MatlabEngine(startcmd);
			}
			catch (const std::exception& e) {
				Logger::logError(std::string("Exception while starting MATLAB engine: ") + e.what());
				s_instance = nullptr;
			}
			
			if (s_engine == nullptr)
			{
				delete s_instance;
				s_instance = nullptr;
				Logger::logWarning("Retrying to start MATLAB engine ["+std::to_string(initialRetryCount-retryCount)+"/"+std::to_string(initialRetryCount)+"]");
				size_t sleepMillis = 1000;
				auto startTime = std::chrono::high_resolution_clock::now();
				while (std::chrono::high_resolution_clock::now() - startTime < std::chrono::milliseconds(sleepMillis))
					QApplication::processEvents();
			}
		} while (s_engine == nullptr && --retryCount > 0);
		if(!s_engine)
		{
			Logger::logError("Failed to start MATLAB engine after multiple attempts.");
		}
		return s_engine != nullptr;
	}
#ifndef MATLAB_API_USE_CPP_API
	bool MatlabEngine::instantiate(const std::u16string& startcmd, int retryCount)
	{
		std::string cmd = to_utf8(startcmd);
		return instantiate(cmd.c_str(), retryCount);
	}
#endif
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
		int ret = 0;
#ifdef MATLAB_API_USE_CPP_API
		const std::shared_ptr<matlab::engine::StreamBuffer> output;
		const std::shared_ptr<matlab::engine::StreamBuffer> error;
		s_engine->eval(to_u16string(command), output, error);
		std::string resultStr = streamBufferToString(output.get());
		std::string errorStr = streamBufferToString(error.get());
		Logger::logDebug("Evaluated command: \n\"" + std::string(command) + "\" result: " + resultStr + " error: " + errorStr);
		if (errorStr.size() > 0)
			ret = -1;
#else
		ret = engEvalString(s_engine, command);
		Logger::logDebug("Evaluated command: \n\"" + std::string(command) + "\"\n -> return code: " + std::to_string(ret));
#endif
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
#ifdef MATLAB_API_USE_CPP_API

#else
			mxArray* oldArray = it->second->release();
			if (oldArray)
				mxDestroyArray(oldArray);
#endif	

			delete it->second;
			it->second = var;
		}
		else
		{
			var->m_owner = s_instance;
			s_instance->m_variables[name] = var;
		}
#ifdef MATLAB_API_USE_CPP_API
		try {
			s_engine->setVariable(to_u16string(name.c_str()), *(var->get()));
		}
		catch (const matlab::engine::EngineException& e) {
			Logger::logError("Failed to put variable '" + name + "' into MATLAB engine. Exception: " + std::string(e.what()));
			return false;
		}
#else
		if (engPutVariable(s_engine, name.c_str(), var->get()) != 0)
		{
			Logger::logError("Failed to put variable '" + name + "' into MATLAB engine.");
			return false;
		}
#endif	

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
#ifdef MATLAB_API_USE_CPP_API
		try {
			s_engine->eval(to_u16string(("clear " + name).c_str()));
		}
		catch (const matlab::engine::EngineException& e) {
			Logger::logError("Failed to clear variable '" + name + "' from MATLAB engine. Exception: " + std::string(e.what()));
			return false;
		}
#else
		if (engEvalString(s_engine, ("clear " + name).c_str()) != 0)
		{
			Logger::logError("Failed to clear variable '" + name + "' from MATLAB engine.");
			return false;
		}
#endif
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
#ifdef MATLAB_API_USE_CPP_API
		matlab::data::Array arr;
		try {
			arr = s_engine->getVariable(to_u16string(name.c_str()));
		}
		catch (const matlab::engine::EngineException& e) {
			Logger::logError("Failed to get variable '" + name + "' from MATLAB engine. Exception: " + std::string(e.what()));
			return nullptr;
		}
#else
		mxArray* arr = engGetVariable(s_engine, name.c_str());
		if (!arr)
		{
			Logger::logError("Failed to get variable '" + name + "' from MATLAB engine.");
			return nullptr;
		}
#endif
		if (it == s_instance->m_variables.end())
		{
			// create new MatlabArray to manage this variable
#ifdef MATLAB_API_USE_CPP_API
			MatlabArray* var = new MatlabArray(name, arr);
#else
			MatlabArray* var = new MatlabArray(name, arr, false); // do not take ownership, as engGetVariable returns a copy
#endif
			var->m_owner = s_instance;
			s_instance->m_variables[name] = var;
			Logger::logDebug("Variable with name '" + name + "' did not exist, created new MatlabArray: " + var->toString());
			return var;
		}
#ifdef MATLAB_API_USE_CPP_API
		it->second->overwrite(arr);
#else
		it->second->overwrite(arr, false, true); // do not take ownership, as engGetVariable returns a copy
#endif
		Logger::logDebug("Variable with name '" + name + "' exists, updated MatlabArray: " + it->second->toString());
		return it->second;
	}
	Matrix MatlabEngine::getMatrix(const std::string& name)
	{
		return Matrix(getVariable(name));
	}
#ifdef MATLAB_API_USE_CPP_API
	MatlabArray MatlabEngine::getProperty(MatlabArray* array, const std::u16string& property)
	{
		return MatlabArray(array->getName()+"."+to_utf8(property), s_engine->getProperty(array->getAPIArray(), property));
	}
#endif
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

#ifdef MATLAB_API_USE_CPP_API
	const std::unique_ptr<matlab::engine::MATLABEngine> &MatlabEngine::getEngine()
	{
		return s_engine;
	}
#else
	Engine* MatlabEngine::getEngine()
	{
		return s_engine;
	}
#endif

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
#ifdef MATLAB_API_USE_CPP_API
		matlab::data::Array arr;
		try {
			arr = s_engine->getVariable(to_u16string(name.c_str()));
		}
		catch (const matlab::engine::EngineException& e) {
			Logger::logError("Failed to get variable '" + name + "' from MATLAB engine. Exception: " + std::string(e.what()));
			return false;
		}
		it->second->overwrite(arr);
#else
		mxArray* arr = engGetVariable(s_engine, name.c_str());
		if (!arr)
		{
			Logger::logError("Failed to get variable '" + name + "' from MATLAB engine.");
			return false;
		}
		it->second->overwrite(arr, false, true); // do not take ownership, as engGetVariable returns a copy
#endif
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
#ifdef MATLAB_API_USE_CPP_API
		try {
			s_engine->setVariable(to_u16string(name.c_str()), *(var->get()));
		}
		catch (const matlab::engine::EngineException& e) {
			Logger::logError("Failed to put variable '" + name + "' into MATLAB engine. Exception: " + std::string(e.what()));
			return false;
		}
#else
		if (engPutVariable(s_engine, name.c_str(), var->get()) != 0)
		{
			Logger::logError("Failed to put variable '" + name + "' into MATLAB engine.");
			return false;
		}
#endif
		return true;
	}


	void MatlabEngine::err_matlabNotStarted()
	{
		Logger::logError("MATLAB engine is not started, call MatlabEngine::instantiate() first.");
	}

}