if(WIN32)
	cmake_minimum_required(VERSION 3.16)
else(WIN32)
	cmake_minimum_required(VERSION 3.13)
endif(WIN32)

project(newPlatform)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32)
	set(SDL2_ROOT "${PROJECT_SOURCE_DIR}/dependencies")
	set(GLFW_ROOT "${PROJECT_SOURCE_DIR}/dependencies")
	set(GLEW_ROOT "${PROJECT_SOURCE_DIR}/dependencies")
	set(GLEW_USE_STATIC_LIBS ON)
endif(WIN32)

option(USE_GLFW "Use GLFW instead of SDL2" OFF)
option(USE_GLES "Use OpenGL ES" ON)

if(USE_GLFW)
        add_executable(${PROJECT_NAME} "main.cxx" "GLHelloWorld.cxx" "PlatformFactory.cxx" "BzfPlatform.cxx" "GLFWPlatform.cxx")
	target_compile_definitions(${PROJECT_NAME} PUBLIC USE_GLFW)
else(USE_GLFW)
        add_executable(${PROJECT_NAME} "main.cxx" "GLHelloWorld.cxx" "PlatformFactory.cxx" "BzfPlatform.cxx" "SDL2Platform.cxx")
endif(USE_GLFW)

if(USE_GLES)
        target_compile_definitions(${PROJECT_NAME} PUBLIC USE_GLES2)
endif(USE_GLES)

if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC GLEW_STATIC)
endif(WIN32)

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif(MSVC)

if(USE_GLFW)
	find_package(glfw3 3.2 REQUIRED)
	target_link_libraries(${PROJECT_NAME} glfw)
else(USE_GLFW)
	find_package(SDL2 REQUIRED)
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
endif(USE_GLFW)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

find_package(GLEW 2.1.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")
