## description: Matlab API

function(dep LIBRARY_MACRO_NAME SHARED_LIB STATIC_LIB STATIC_PROFILE_LIB)
    set(LIB_MACRO_NAME MATLAB_AVAILABLE)
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
    list(APPEND DEPS_FOR_SHARED_LIB ${MATLAB_LIBS})
    list(APPEND DEPS_FOR_STATIC_LIB ${MATLAB_LIBS})
    list(APPEND DEPS_FOR_STATIC_PROFILE_LIB ${MATLAB_LIBS}) # only use for static profiling profile

	set(${LIBRARY_MACRO_NAME} "${${LIBRARY_MACRO_NAME}};${LIB_MACRO_NAME}" PARENT_SCOPE)
    set(${SHARED_LIB} "${${SHARED_LIB}};${DEPS_FOR_SHARED_LIB}" PARENT_SCOPE)
    set(${STATIC_LIB} "${${STATIC_LIB}};${DEPS_FOR_STATIC_LIB}" PARENT_SCOPE)
    set(${STATIC_PROFILE_LIB} "${${STATIC_PROFILE_LIB}};${DEPS_FOR_STATIC_PROFILE_LIB}" PARENT_SCOPE)
endfunction()

dep(DEPENDENCY_NAME_MACRO DEPENDENCIES_FOR_SHARED_LIB DEPENDENCIES_FOR_STATIC_LIB DEPENDENCIES_FOR_STATIC_PROFILE_LIB)
