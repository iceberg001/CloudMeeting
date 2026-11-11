# cmake file for DuiLib —— 静态库版本
# 输出：${CMAKE_BINARY_DIR}/3dparty/DuiLib/<Config>/duilib_static.lib
# 主工程 CMakeLists 检测到该库存在后改用 imported target 直接链接（不再重编）

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RootSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Control ControlSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core CoreSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugin PluginSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Layout LayoutSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Utils UtilsSrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Control)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Layout)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utils)

# 静态库：UILIB_API 展开为空（UIlib.h 依据 UILIB_STATIC 宏判断）
# 与主工程 CloudMeeting 的宏保持一致（UNICODE/_UNICODE 影响 TCHAR ABI，必须相同）
add_definitions(-DUILIB_STATIC -DUNICODE -D_UNICODE -D_WINDOWS)

# 覆盖全局设置（仅本子目录生效）：
# 1. DuiLib 源码是 GBK 编码且使用 ::byte，与全局 /utf-8 和 C++17 的 std::byte 冲突，
#    因此本目录强制 C++14 并移除 /utf-8（/utf-8 经 add_compile_options 加入目录属性，
#    不在 CMAKE_CXX_FLAGS 里，需从目录属性中过滤）
string(REPLACE "/utf-8" "" DUILIB_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${DUILIB_CXX_FLAGS}")
get_directory_property(DUILIB_DIR_OPTS COMPILE_OPTIONS)
list(FILTER DUILIB_DIR_OPTS EXCLUDE REGEX "/utf-8")
set_directory_properties(PROPERTIES COMPILE_OPTIONS "${DUILIB_DIR_OPTS}")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(duilib_static STATIC ${ControlSrc} ${CoreSrc} ${PluginSrc} ${LayoutSrc} ${UtilsSrc} ${RootSrc})

# 与主工程一致使用静态 CRT（/MT、/MTd），否则链接期 RuntimeLibrary 不匹配
set_target_properties(duilib_static PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

set_target_properties(duilib_static PROPERTIES
    DEBUG_POSTFIX ""
    OUTPUT_NAME "duilib_static"
)

target_link_libraries(duilib_static comctl32 gdi32 user32)
