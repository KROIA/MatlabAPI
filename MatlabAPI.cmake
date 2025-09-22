## description: Matlab API Wrapper for QT c++ applications 
include(FetchContent)

function(dep LIBRARY_MACRO_NAME SHARED_LIB STATIC_LIB STATIC_PROFILE_LIB)
    # Define the git repository and tag to download from
    set(LIB_NAME MatlabAPI)		
    set(LIB_MACRO_NAME MATLAB_API_LIBRARY_AVAILABLE)
    set(GIT_REPO https://github.com/KROIA/MatlabAPI.git)	
    set(GIT_TAG main)		

    FetchContent_Declare(
        ${LIB_NAME}
        GIT_REPOSITORY ${GIT_REPO}
        GIT_TAG        ${GIT_TAG}
    )

    set(${LIB_NAME}_NO_EXAMPLES True)						# Disables the examlpes of the library
    set(${LIB_NAME}_NO_UNITTESTS True)						# Disables the unittests of the library

    message("Downloading dependency: ${LIB_NAME} from: ${GIT_REPO} tag: ${GIT_TAG}")
    FetchContent_MakeAvailable(${LIB_NAME})

    find_package(Matlab REQUIRED COMPONENTS ENG_LIBRARY MX_LIBRARY MAT_LIBRARY)
    
    # Include directories
    include_directories(${Matlab_INCLUDE_DIRS})
    
    set(MATLAB_LIBS
        ${Matlab_ENGINE_LIBRARY}
        ${Matlab_DATAARRAY_LIBRARY}
        ${Matlab_ENG_LIBRARY}
        ${Matlab_MX_LIBRARY}
        ${Matlab_MEX_LIBRARY}
        ${Matlab_MAT_LIBRARY}
    )

    # Add this library to the specific profiles of this project
    list(APPEND DEPS_FOR_SHARED_LIB ${LIB_NAME}_shared ${MATLAB_LIBS})
    list(APPEND DEPS_FOR_STATIC_LIB ${LIB_NAME}_static ${MATLAB_LIBS})
    list(APPEND DEPS_FOR_STATIC_PROFILE_LIB ${LIB_NAME}_static_profile ${MATLAB_LIBS}) # only use for static profiling profile

    set(${LIBRARY_MACRO_NAME} "${${LIBRARY_MACRO_NAME}};${LIB_MACRO_NAME}" PARENT_SCOPE)
    set(${SHARED_LIB} "${${SHARED_LIB}};${DEPS_FOR_SHARED_LIB}" PARENT_SCOPE)
    set(${STATIC_LIB} "${${STATIC_LIB}};${DEPS_FOR_STATIC_LIB}" PARENT_SCOPE)
    set(${STATIC_PROFILE_LIB} "${${STATIC_PROFILE_LIB}};${DEPS_FOR_STATIC_PROFILE_LIB}" PARENT_SCOPE)
endfunction()

dep(DEPENDENCY_NAME_MACRO DEPENDENCIES_FOR_SHARED_LIB DEPENDENCIES_FOR_STATIC_LIB DEPENDENCIES_FOR_STATIC_PROFILE_LIB)
