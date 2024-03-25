set(NETWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB NETWORK_SRC 
	"${NETWORK_PATH}/*.cpp"
	"${NETWORK_PATH}/*.hpp"
)

set(NETWORK_VOICE_SRC
"${NETWORK_PATH}/Voice/Headset.hpp"
"${NETWORK_PATH}/Voice/headset_common.cpp"
"${NETWORK_PATH}/Voice/Headset_STUB.cpp"
"${NETWORK_PATH}/Voice/lpc10.cpp"
"${NETWORK_PATH}/Voice/lpc10.hpp"
"${NETWORK_PATH}/Voice/speex.cpp"
"${NETWORK_PATH}/Voice/speex.hpp"
"${NETWORK_PATH}/Voice/VoiceMgr.cpp"
"${NETWORK_PATH}/Voice/VoiceMgr.hpp"
"${NETWORK_PATH}/Voice/VoiceProxy.cpp"
"${NETWORK_PATH}/Voice/VoiceProxy.hpp"
)

file(GLOB_RECURSE NETWORK_DOWNLOADER_SRC
"${NETWORK_PATH}/downloader/*.c"
"${NETWORK_PATH}/downloader/*.h"
"${NETWORK_PATH}/downloader/*.cpp"
"${NETWORK_PATH}/downloader/*.hpp"
)

add_library(NetworkMgr STATIC ${NETWORK_SRC} ${NETWORK_VOICE_SRC} ${NETWORK_DOWNLOADER_SRC})
set_support_defaults(NetworkMgr)

target_link_libraries(NetworkMgr Entropy)