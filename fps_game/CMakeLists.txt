cmake_minimum_required(VERSION 3.16)
project(BattleRoyale LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(battle_royale
    src/main.cpp
    src/Game.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Player.cpp
    src/World.cpp
    src/UI.cpp
    src/Font.cpp
    src/Effects.cpp
    src/BotManager.cpp
    src/DropManager.cpp
    src/Pickup.cpp
    src/Zone.cpp
    src/Build.cpp
    src/SaveData.cpp
)

target_include_directories(battle_royale PRIVATE src)
target_link_libraries(battle_royale PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

# Make the shaders directory discoverable from the binary
target_compile_definitions(battle_royale PRIVATE
    SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders"
)
