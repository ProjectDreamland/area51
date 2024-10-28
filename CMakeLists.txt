cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Area51 C CXX)
if(APPLE)
    enable_language(OBJC)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin") 

if (MSVC)
    # Always generate PDBs
    add_compile_options(/Zi)
    add_link_options(/DEBUG)

    add_compile_options(/W3) # Warning level
endif()

# XCore utilities

set(XCORE_PATH "${CMAKE_SOURCE_DIR}/xCore")
set(XCORE_AUX_PATH "${CMAKE_SOURCE_DIR}/xCore/Auxiliary")
set(XCORE_X_FILES_PATH "${CMAKE_SOURCE_DIR}/xCore/x_files")
set(XCORE_3RDPARTY_PATH "${CMAKE_SOURCE_DIR}/xCore/3rdParty")

function(set_xcore_defaults TARGET)
	target_include_directories(${TARGET} 	PRIVATE ${XCORE_PATH} 
											PRIVATE ${XCORE_AUX_PATH}
											PRIVATE ${XCORE_X_FILES_PATH})
											
	# Compile definitions
	target_compile_definitions(${TARGET}	PRIVATE TARGET_PC
											PRIVATE $<$<CONFIG:Debug>:CONFIG_DEBUG>
											PRIVATE $<$<CONFIG:Release>:CONFIG_RETAIL>)
endfunction()

# Support utilities

set(SUPPORT_PATH "${CMAKE_SOURCE_DIR}/Support")

function(set_support_defaults TARGET)
	set_xcore_defaults(${TARGET})
	target_include_directories(${TARGET} PRIVATE "${SUPPORT_PATH}")
endfunction()

add_subdirectory(xCore)
add_subdirectory(Support)
add_subdirectory(Apps)
