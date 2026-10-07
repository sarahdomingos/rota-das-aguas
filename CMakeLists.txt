cmake_minimum_required(VERSION 3.10)

project(RotaDasAguas LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Arquivos fonte
# --------------------------------------------------

file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

# --------------------------------------------------
# Diretórios de include
# --------------------------------------------------

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/include"
)

# --------------------------------------------------
# OpenGL
# --------------------------------------------------

find_package(OpenGL REQUIRED)

# --------------------------------------------------
# GLFW
# --------------------------------------------------

if(WIN32)

    find_library(GLFW_LIBRARY
        NAMES glfw3dll glfw3
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/external/lib"
        NO_DEFAULT_PATH
        REQUIRED
    )

    target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        ${GLFW_LIBRARY}
        glu32
        gdi32
        user32
        shell32
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/external/lib/glfw3.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll"
    )

    if(MINGW)
        # Evita depender das DLLs de runtime do C++ do MinGW
        target_link_libraries(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)

        # A glfw3.dll (32-bit) depende da libgcc_s_dw2-1.dll do MinGW
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        if(EXISTS "${MINGW_BIN_DIR}/libgcc_s_dw2-1.dll")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/libgcc_s_dw2-1.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libgcc_s_dw2-1.dll"
            )
        endif()
    endif()

elseif(APPLE)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            OpenGL::GL
            glfw
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )

else()

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            OpenGL::GL
            glfw
            X11
            pthread
            dl
    )

endif()