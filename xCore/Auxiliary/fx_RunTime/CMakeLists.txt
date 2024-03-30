file(GLOB FX_RUNTIME_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(fx_RunTime STATIC ${FX_RUNTIME_SRC})
set_xcore_defaults(fx_RunTime)
target_include_directories(fx_RunTime PRIVATE "${CMAKE_SOURCE_DIR}/Support")
target_link_libraries(fx_RunTime Entropy)