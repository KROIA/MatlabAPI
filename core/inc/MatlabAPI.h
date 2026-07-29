// @file MatlabAPI.h
// @brief Main public header for the library.
//
// Include this single header to access the entire public API.
// Add your own public headers inside USER_SECTION 2 so that
// consumers only need `#include "MatlabAPI.h"`.
#pragma once

/// USER_SECTION_START 1

/// USER_SECTION_END

#include "MatlabAPI_info.h"

/// USER_SECTION_START 2
#include "MatlabEngine.h"
#include "MatlabArray.h"

#include "math/Matrix.h"
#include "math/StateSpaceModel.h"
#include "math/TransferFunction.h"
#include "math/MIMOSystem.h"


#include "ui/MatlabEmbeddedPlotWidget.h"
/// USER_SECTION_END