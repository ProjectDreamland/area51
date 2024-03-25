set(ENTROPY_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB ENTROPY_SRC
	"${ENTROPY_PATH}/*.cpp"
	"${ENTROPY_PATH}/*.hpp"
)

# Common
file(GLOB ENTROPY_COMMON_SRC
	"${ENTROPY_PATH}/Common/*.cpp"
	"${ENTROPY_PATH}/Common/*.hpp"
)

# Audio
file(GLOB ENTROPY_AUDIO_SRC
	"${ENTROPY_PATH}/Audio/*.cpp"
	"${ENTROPY_PATH}/Audio/*.hpp"
)

# Audio Hardware (PC)
set(ENTROPY_AUDIO_HARDWARE_SRC
	"${ENTROPY_PATH}/Audio/Hardware/audio_hardware_pc.cpp"
	"${ENTROPY_PATH}/Audio/Hardware/audio_stream_pc.cpp"
)

# Audio IAL
file(GLOB ENTROPY_AUDIO_IAL_SRC
	"${ENTROPY_PATH}/Audio/IAL/*.cpp"
	"${ENTROPY_PATH}/Audio/IAL/*.hpp"
)

# Direct3D Engine
file(GLOB ENTROPY_D3DENGINE_SRC
	"${ENTROPY_PATH}/D3DEngine/*.cpp"
	"${ENTROPY_PATH}/D3DEngine/*.hpp"
)

# IOManager
set(ENTROPY_IOMANAGER_SRC
	"${ENTROPY_PATH}/IOManager/io_cache.cpp"
	"${ENTROPY_PATH}/IOManager/io_cache.hpp"
	"${ENTROPY_PATH}/IOManager/io_device.cpp"
	"${ENTROPY_PATH}/IOManager/io_device.hpp"
	#"${ENTROPY_PATH}/IOManager/io_device_pc.cpp"
	"${ENTROPY_PATH}/IOManager/io_dfs.cpp"
	"${ENTROPY_PATH}/IOManager/io_dfs.hpp"
	"${ENTROPY_PATH}/IOManager/io_filesystem.cpp"
	"${ENTROPY_PATH}/IOManager/io_filesystem.hpp"
	"${ENTROPY_PATH}/IOManager/io_header.hpp"
	"${ENTROPY_PATH}/IOManager/io_mgr.cpp"
	"${ENTROPY_PATH}/IOManager/io_mgr.hpp"
	"${ENTROPY_PATH}/IOManager/io_request.cpp"
	"${ENTROPY_PATH}/IOManager/io_request.hpp"
	"${ENTROPY_PATH}/IOManager/io_stdio.cpp"
	"${ENTROPY_PATH}/IOManager/io_stdio.hpp"
)

# IOManager - Device DVD (PC)
set(ENTROPY_IOMANAGER_DEVICE_DVD_SRC
	"${ENTROPY_PATH}/IOManager/Device_DVD/io_device_dvd.cpp"
	"${ENTROPY_PATH}/IOManager/Device_DVD/io_device_dvd.hpp"
	"${ENTROPY_PATH}/IOManager/Device_DVD/io_device_dvd_pc.cpp"
)

# MemCard
set(ENTROPY_MEMCARD_SRC
	"${ENTROPY_PATH}/MemCard/memcard.cpp"
	#"${ENTROPY_PATH}/MemCard/memcard_pc.cpp"
	"${ENTROPY_PATH}/MemCard/memcard_stub.cpp"
)

# Network
file(GLOB ENTROPY_NETWORK_SRC
	"${ENTROPY_PATH}/Network/*.cpp"
	"${ENTROPY_PATH}/Network/*.hpp"
)

# Network PC
file(GLOB ENTROPY_NETWORK_PC_SRC
	"${ENTROPY_PATH}/Network/PC/*.cpp"
	"${ENTROPY_PATH}/Network/PC/*.hpp"
)

add_library(Entropy STATIC 	${ENTROPY_SRC} 
							${ENTROPY_COMMON_SRC}
							${ENTROPY_AUDIO_SRC}
							${ENTROPY_AUDIO_HARDWARE_SRC}
							${ENTROPY_AUDIO_IAL_SRC}
							${ENTROPY_D3DENGINE_SRC}
							${ENTROPY_IOMANAGER_SRC}
							${ENTROPY_IOMANAGER_DEVICE_DVD_SRC}
							${ENTROPY_MEMCARD_SRC}
							${ENTROPY_NETWORK_SRC}
							${ENTROPY_NETWORK_PC_SRC}
							)


target_include_directories(Entropy PUBLIC "${ENTROPY_PATH}" PUBLIC "${ENTROPY_PATH}/Audio" )
set_xcore_defaults(Entropy)

target_link_libraries(Entropy x_files)