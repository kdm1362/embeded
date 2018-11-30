cmake_minimum_required (VERSION 2.8) 
 
set(CMAKE_MODULE_PATH "/usr/local/lib/cmake/${CMAKE_MODULE_PATH}") 
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

find_package(raspicam REQUIRED)
find_package(OpenCV)
find_library(wiringPi_LIB wiringPi)

add_compile_options(-g)
add_executable (main main.cpp opencv.cpp serial.cpp)  
target_link_libraries (main ${raspicam_CV_LIBS})
target_link_libraries (main ${OpenCV_LIBS} wiringPi)

