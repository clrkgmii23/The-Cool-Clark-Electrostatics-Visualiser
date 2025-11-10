add_executable(CMakeTarget "Src/The Cool Clark Electrostatics Visualiser.cpp"
"Src/glad.c" "Src/Core.cpp" "Src/Shader.cpp" "Src/utils.cpp" "Src/SourceObjects.h" "Src/ComputeManager.cpp" 
"Src/Renderer.cpp" "Src/Camera.cpp" "Src/Interactivity.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(CMakeTarget PRIVATE "Third Party Libraries/include")
target_link_directories   (CMakeTarget PRIVATE "Third Party Libraries/lib")
target_link_libraries(CMakeTarget glfw3 opengl32)

# copy shaders to build dir
file(GLOB SHADERS "Src/Shaders/*")
foreach(shader ${SHADERS})
	file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR}/Src/Shaders ${shader})
	configure_file(${shader} ${CMAKE_BINARY_DIR}/Src/Shaders/${rel} COPYONLY)
endforeach()