cmake_minimum_required(VERSION 3.10)
set(VCPKG_FEATURE_FLAGS "manifests")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project("RTStest")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS thread)

find_package(SFML REQUIRED COMPONENTS Graphics Window System)

add_executable(RTStest
    "main.cpp"
    "include.h"
)

target_link_libraries(RTStest
    PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    Boost::thread
    Boost::headers
)

if(WIN32)
    add_custom_command(
        TARGET RTStest
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SFML::Graphics>
            $<TARGET_FILE:SFML::Window>
            $<TARGET_FILE:SFML::System>
            $<TARGET_FILE_DIR:RTStest>
        COMMENT "Copying SFML DLLs to output directory"
    )
endif()

add_custom_command(
    TARGET RTStest
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/waffle.png
        $<TARGET_FILE_DIR:RTStest>/waffle.png
    COMMENT "Copying waffle.png"
)