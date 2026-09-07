cmake_minimum_required(VERSION 3.15)
project(ZombieSurvivor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/lib/raylib/raylib-6.0_win64_msvc16")

add_executable(zombie-survivor
    src/main.cpp
    src/game.cpp
    src/player.cpp
    src/zombie.cpp
    src/weapon.cpp
    src/level.cpp
    src/loot.cpp
    src/story.cpp
    src/ui.cpp
    src/particles.cpp
)

target_include_directories(zombie-survivor PRIVATE ${RAYLIB_DIR}/include)
target_link_directories(zombie-survivor PRIVATE ${RAYLIB_DIR}/lib)
target_link_libraries(zombie-survivor PRIVATE raylib opengl32 gdi32 winmm user32 shell32)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

add_custom_command(TARGET zombie-survivor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${RAYLIB_DIR}/lib/raylib.dll"
    $<TARGET_FILE_DIR:zombie-survivor>
)
