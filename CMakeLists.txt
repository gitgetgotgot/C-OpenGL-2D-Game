cmake_minimum_required(VERSION 3.30)
project(OpenGL_2D_Game VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
		glad::glad
        glfw
        SDL2::SDL2
        SDL2_mixer::SDL2_mixer
        glm::glm
        stb::stb
)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
elseif (APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenGL_LIBRARY})
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()


set(GAME_DATA_DIRS
    Resources
    Saves
)

foreach(dir ${GAME_DATA_DIRS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/${dir}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/${dir}
    )
endforeach()