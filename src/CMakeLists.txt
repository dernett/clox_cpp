add_executable(clox main.cpp compiler.cpp vm.cpp)

target_include_directories(clox PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(clox PUBLIC cxx_std_23)
