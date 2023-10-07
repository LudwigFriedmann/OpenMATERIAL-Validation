#
# Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
#
# Physical path tracer for support of capabilities of the proposed OpenMaterial_material_parameters
# extension for the Khronos Group glTF 2.0 file format as well as an asset check procedures
#

# Project setup --------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
set(CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_BUILD_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
project(pathtracer)
# ----------------------------------------------------------------------------------------------------

# Compilation options --------------------------------------------------------------------------------
option(PT_FAST_MATH "Fast math optimization"                                            OFF)
option(PT_MATERIAL_VERBOSE "Dumps meta information about all the scene materials"       OFF)
option(PT_VALIDATION "Activates the false-color rendering mode for asset validation"    OFF)
# ----------------------------------------------------------------------------------------------------

# Paths and dependencies -----------------------------------------------------------------------------
# Internal
include_directories("src/")
include_directories("src/assets/")
include_directories("src/backgrounds/")
include_directories("src/brdf/")
include_directories("src/lights/")
include_directories("src/materials/")
include_directories("src/renderer/")
include_directories("src/scene/")
include_directories("src/sensors/")
include_directories("src/tonemapping/")
include_directories("src/utilities/")
include_directories("src/validation/")

# External
include_directories("../external/argparse/")
include_directories("../external/CImg/")
include_directories("../external/doctest/")
include_directories("../external/json/")
include_directories("../external/sort_r/")
include_directories("../external/tinygltf/")
# ----------------------------------------------------------------------------------------------------

# Standard optimization ------------------------------------------------------------------------------
if(NOT DEFINED OPT)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(OPT "-O0")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(OPT "-O0")
    endif()
endif()
# ----------------------------------------------------------------------------------------------------


# Finding embree -------------------------------------------------------------------------------------

# Custom call to fetch file
include(FetchContent)
set(FETCHCONTENT_QUIET TRUE)

function(fetch_file file url dir)

    FetchContent_Declare(
        ${file}
        URL                     ${url}
        DOWNLOAD_DIR            ${dir}
        DOWNLOAD_NO_EXTRACT     TRUE
        TLS_VERIFY              TRUE
        GIT_PROGRESS            TRUE
        OVERRIDE_FIND_PACKAGE   TRUE
    )

    FetchContent_Populate(${file})

endfunction(fetch_file)

# embree control block
set(embree_VERSION_MAJOR 4)
set(embree_VERSION_MINOR 1)
set(embree_VERSION_PATCH 0)
set(embree_VERSION "${embree_VERSION_MAJOR}.${embree_VERSION_MINOR}.${embree_VERSION_PATCH}")
set(embree_PATH ${PROJECT_SOURCE_DIR}/../external/embree)
set(embree_DIR  ${embree_PATH}/lib/cmake/embree-${embree_VERSION})

# Check if valid embree installation exists
if (NOT EXISTS ${embree_PATH}/bin OR NOT EXISTS ${embree_PATH}/lib OR NOT EXISTS ${embree_DIR})
    message(WARNING "[Warning]: embree package is missing.")

    if (EXISTS ${embree_PATH})
        message(WARNING "[Warning]: incomplete embree package is found under: ${embree_PATH}.")
        file(REMOVE_RECURSE ${embree_PATH})
    endif()

    set(embree_SRC_REPO "https://github.com/embree/embree/releases/download/v${embree_VERSION}")
    if (WIN32)
        set(ARCHIVE_EXTENSION ".zip")
        set(embree_DLFILE "embree-${embree_VERSION}.x64.windows")
    else()
        set(ARCHIVE_EXTENSION ".tar.gz")
        set(embree_DLFILE "embree-${embree_VERSION}.x86_64.linux")
    endif()

    set(embree_URL      "${embree_SRC_REPO}/${embree_DLFILE}${ARCHIVE_EXTENSION}")
    set(embree_DLPATH   "${PROJECT_SOURCE_DIR}/../external")

    message("Downloading embree-${embree_VERSION}...")

    fetch_file(
        embreeLib
        ${embree_URL}
        ${embree_DLPATH}
    )
    file(ARCHIVE_EXTRACT
        INPUT ${embree_DLPATH}/${embree_DLFILE}${ARCHIVE_EXTENSION}
        DESTINATION ${embree_DLPATH}
    )
    if (NOT EXISTS ${embree_DLPATH}/${embree_DLFILE})
        message("[Error]: Embree archive could not be extracted.")
        message(FATAL_ERROR "Automated download was not succesfull!"
                            " Please download embree ${embree_VERSION} from ${embree_URL} and unpack it into ${embree_PATH}")
    endif()

    # Remove downloaded file
    file(REMOVE ${embree_DLPATH}/${embree_DLFILE}${ARCHIVE_EXTENSION})

    file(RENAME
        ${embree_DLPATH}/${embree_DLFILE}
        ${embree_PATH}
        RESULT RENAME_RESULT
    )
    if (NOT RENAME_RESULT EQUAL 0)
        message(FATAL_ERROR "[Error]: ${RENAME_RESULT}.")
    else()
        message("embree ${embree_VERSION} downloaded!")
    endif()
endif()

set(embree_MAJOR "embree${embree_VERSION_MAJOR}")
find_package(embree 4.0 REQUIRED)

# ----------------------------------------------------------------------------------------------------

# Executable setup -----------------------------------------------------------------------------------
add_executable(pathtracer
    src/assets/AssetGeometry.cpp 
    src/assets/AssetDefs.cpp 
    src/assets/ImageData.cpp 
    src/assets/Mesh.cpp 
    src/assets/materials/AssetMaterial.cpp 
    src/assets/materials/AssetMaterialIor.cpp 
    src/assets/materials/PbrMaterial.cpp 
    src/backgrounds/HdrBackground.cpp 
    src/lights/PointLight.cpp 
    src/lights/RenderLight.cpp 
    src/lights/RenderLightPoint.cpp 
    src/materials/BitmapTexture.cpp 
    src/materials/DiffuseColorMaterial.cpp 
    src/materials/OpenMaterial.cpp 
    src/materials/PhysicallyBasedMaterial.cpp 
    src/materials/RenderMaterial.cpp 
    src/renderer/BDPTRenderer.cpp 
    src/scene/RenderMesh.cpp 
    src/scene/RenderScene.cpp 
    src/scene/SurfacePoint.cpp 
    src/sensors/PinholeCamera.cpp 
    src/sensors/RenderRay.cpp 
    src/sensors/RenderSensor.cpp 
    src/tonemapping/ToneMapping.cpp 
    src/utilities/ImageSaver.cpp 
    src/utilities/Random.cpp 
    src/utilities/ReferenceLink.cpp 
    src/utilities/Utils.cpp 
    src/validation/FCHandler.cpp 
    src/validation/FCSubject.cpp 
    ../external/argparse/argparse.c 
    ../external/doctest.cpp  
    ../external/tiny_gltf.cpp 
    src/main.cpp 
)
target_link_libraries(pathtracer PUBLIC embree)

if (WIN32)
    install(TARGETS pathtracer DESTINATION . )
    install(FILES "${PROJECT_SOURCE_DIR}/../external/embree/bin/${embree_MAJOR}.dll"
                  "${PROJECT_SOURCE_DIR}/../external/embree/bin/tbb12.dll"
            DESTINATION .
    )
else()
    install(TARGETS pathtracer DESTINATION . )
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/../external/embree/lib/
        DESTINATION .
        FILES_MATCHING PATTERN "*tbb*" PATTERN "*${embree_MAJOR}*"
        PATTERN "cmake" EXCLUDE
    )
endif()
# ----------------------------------------------------------------------------------------------------


# Custom options -------------------------------------------------------------------------------------
if(PT_FAST_MATH)
	message(NOTICE "-- Fast math optimization is activated")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(-ffast-math)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		add_compile_options(/fp:fast)
	endif()
else()
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		add_compile_options(/fp:precise)
	endif()
endif()

if(PT_MATERIAL_VERBOSE)
    message(VERBOSE "-- Material verbose dumping is activated")
    add_definitions(-DMATERIAL_VERBOSE_DUMP=1)
endif()

if(PT_VALIDATION)
    message(NOTICE "-- False color rendering mode is activated")
    add_definitions(-DFC_VALIDATION=1)
endif()
# ----------------------------------------------------------------------------------------------------

# Setting C/C++ flags --------------------------------------------------------------------------------
# CFLAGS
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pthread ${OPT} -Wall -Wextra -Dcimg_display=0")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pthread ${OPT} -Wall -Dcimg_display=0")
endif()
	
# CXXFLAGS
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ${OPT} -Wall -Wextra -Dcimg_display=0")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ${OPT} -Wall -Dcimg_display=0")
endif()
# ----------------------------------------------------------------------------------------------------
