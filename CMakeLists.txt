cmake_minimum_required(VERSION 3.10)
project(CEventHub VERSION 1.0)

# Add debug symbols
set(CMAKE_BUILD_TYPE Debug)


# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set CMake module path if needed
list(APPEND CMAKE_MODULE_PATH "/usr/local/cmake")

# Find required packages - Azure IoT C SDK
find_package(azure_c_shared_utility REQUIRED CONFIG)
find_package(uamqp REQUIRED CONFIG)
find_package(azure-identity-cpp CONFIG REQUIRED)

# Add executable
add_executable(ceh main.cpp eh_sender.c eh_receiver.c)


target_link_libraries(ceh PRIVATE uamqp)
target_link_libraries(ceh PRIVATE Azure::azure-identity)
target_link_libraries(ceh PRIVATE aziotsharedutil)


# Include directories
#target_include_directories(ceh PUBLIC
#    ${azure_c_shared_utility_INCLUDE_DIRS}
#    ${azure_uamqp_c_INCLUDE_DIRS}
#    /usr/local/include/azureiot
#)

# Link libraries
#target_link_libraries(ceh
#    ${azure_c_shared_utility_LIBRARIES}
#    ${azure_uamqp_c_LIBRARIES}
#    aziotsharedutil
#    uamqp
#)