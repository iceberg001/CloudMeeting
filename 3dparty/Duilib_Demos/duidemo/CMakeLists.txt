# cmake file for duidemo

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RootSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/NodeDemo NodeDemoSrc)
set(source_files ${RootSrc} ${NodeDemoSrc} duidemo.rc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/DuiLib)

add_executable(duidemo WIN32 ${source_files})
add_dependencies(duidemo DuiLib)

target_link_libraries(duidemo
    "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/DuiLib.lib"
    user32.lib gdi32.lib comctl32.lib)

set_target_properties(duidemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
add_custom_command(TARGET duidemo POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${PROJECT_BINARY_DIR}/bin/$<CONFIG>/duidemo.exe ${PROJECT_SOURCE_DIR}/bin/duidemo.exe)
