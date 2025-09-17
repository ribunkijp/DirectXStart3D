cmake_minimum_required(VERSION 3.16)
project(DirectXStart3D CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(directxtex CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(SOURCES
  src/main.cpp
  src/GraphicsApp.cpp
  src/Camera.cpp
  src/GraphicsUtils.cpp
  src/BufferUtils.cpp
  src/Timer.cpp
  src/Player.cpp
  src/InputController.cpp
  src/Ground.cpp
  src/TextureLoader.cpp
)

set(HEADERS
  src/Pch.h
  src/GraphicsApp.h
  src/Camera.h
  src/GraphicsUtils.h
  src/CommonTypes.h
  src/BufferUtils.h
  src/Timer.h
  src/Player.h
  src/InputController.h
  src/Ground.h
  src/TextureLoader.h
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
set_source_files_properties(${HEADERS} PROPERTIES HEADER_FILE_ONLY ON)

source_group(TREE "${CMAKE_SOURCE_DIR}/src" FILES ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:Pch.h>"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  UNICODE _UNICODE NOMINMAX
)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    /W4
    /permissive-
    /utf-8
    /EHsc
    /MP
    /Zc:__cplusplus
    /external:W3
    /external:anglebrackets
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  Microsoft::DirectXTex
  nlohmann_json::nlohmann_json
  d3d11
  dxgi
  d3dcompiler
  Shcore.lib
)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  $<TARGET_PROPERTY:Microsoft::DirectXTex,INTERFACE_INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:nlohmann_json::nlohmann_json,INTERFACE_INCLUDE_DIRECTORIES>
)

if (EXISTS "${CMAKE_SOURCE_DIR}/assets")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets")
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/shaders")
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")
endif()
