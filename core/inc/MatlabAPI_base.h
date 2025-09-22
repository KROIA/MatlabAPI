#pragma once

/// USER_SECTION_START 1

/// USER_SECTION_END

#include "MatlabAPI_global.h"
#include "MatlabAPI_debug.h"
#include "MatlabAPI_info.h"

/// USER_SECTION_START 2
#define MATLAB_API_USE_CPP_API

#ifdef MATLAB_API_USE_CPP_API
#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"
#endif
/// USER_SECTION_END