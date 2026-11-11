cmake_minimum_required(VERSION 3.20)
project(CloudMeeting CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/W3)
endif()

# ------------------------------------------------------------------
# DuiLib 预编译策略：
#   1. 第一次构建时 add_subdirectory 编译 DuiLib，生成 duilib_static.lib
#   2. 之后只要预编译库存在，就不再编译 DuiLib 源码，直接链接 .lib
#      （除非 DuiLib 代码有改动，需手动删除 build 目录重新生成）
# ------------------------------------------------------------------
set(DUILIB_DIR ${CMAKE_SOURCE_DIR}/3dparty/DuiLib)
set(DUILIB_PREBUILT_LIB ${CMAKE_BINARY_DIR}/3dparty/DuiLib/$<CONFIG>/duilib_static.lib)
set(DUILIB_PREBUILT_RELEASE ${CMAKE_BINARY_DIR}/3dparty/DuiLib/Release/duilib_static.lib)
set(DUILIB_PREBUILT_DEBUG ${CMAKE_BINARY_DIR}/3dparty/DuiLib/Debug/duilib_static.lib)

if(EXISTS ${DUILIB_PREBUILT_RELEASE} OR EXISTS ${DUILIB_PREBUILT_DEBUG})
    message(STATUS "DuiLib: using prebuilt static library (no recompile)")
    add_library(duilib_static STATIC IMPORTED GLOBAL)
    if(EXISTS ${DUILIB_PREBUILT_RELEASE})
        set_target_properties(duilib_static PROPERTIES
            IMPORTED_LOCATION ${DUILIB_PREBUILT_RELEASE}
            IMPORTED_LOCATION_RELEASE ${DUILIB_PREBUILT_RELEASE}
        )
    endif()
    if(EXISTS ${DUILIB_PREBUILT_DEBUG})
        set_target_properties(duilib_static PROPERTIES
            IMPORTED_LOCATION_DEBUG ${DUILIB_PREBUILT_DEBUG}
        )
        if(NOT EXISTS ${DUILIB_PREBUILT_RELEASE})
            set_target_properties(duilib_static PROPERTIES
                IMPORTED_LOCATION ${DUILIB_PREBUILT_DEBUG}
            )
        endif()
    endif()
else()
    message(STATUS "DuiLib: building from source (first-time only)")
    add_subdirectory(3dparty/DuiLib)
endif()

set(CLOUDMEETING_SOURCES
    src/main.cpp
    src/core/meeting_manager.cpp
    src/core/server_logic.cpp
    src/core/client_logic.cpp
    src/core/remote_frame_store.cpp
    src/net/tcp_socket.cpp
    src/net/net_utils.cpp
    src/media/video_capture.cpp
    src/media/audio_capture.cpp
    src/media/audio_player.cpp
    src/media/desktop_capture.cpp
    src/ui/main_wnd.cpp
    src/ui/ui_create_meeting.cpp
    src/ui/ui_join_meeting.cpp
    src/ui/ui_meeting_room.cpp
    src/ui/ui_video_display.cpp
    src/ui/ui_chat_message.cpp
    src/ui/ui_partner_list.cpp
)

add_executable(CloudMeeting WIN32 ${CLOUDMEETING_SOURCES})

target_compile_definitions(CloudMeeting PRIVATE
    UNICODE _UNICODE _WINDOWS UILIB_STATIC WIN32_LEAN_AND_MEAN
)

target_include_directories(CloudMeeting PRIVATE
    src
    src/core
    src/net
    src/media
    src/ui
    ${DUILIB_DIR}
    ${DUILIB_DIR}/Core
    ${DUILIB_DIR}/Control
    ${DUILIB_DIR}/Layout
    ${DUILIB_DIR}/Utils
)

target_link_libraries(CloudMeeting
    duilib_static
    ws2_32
    winmm
    mfplat
    mf
    mfreadwrite
    mfuuid
    ole32
    strmiids
    gdi32
    user32
    comctl32
    oleaut32
)

set_property(TARGET CloudMeeting PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
add_custom_command(TARGET CloudMeeting POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/resource
    $<TARGET_FILE_DIR:CloudMeeting>/resource
)