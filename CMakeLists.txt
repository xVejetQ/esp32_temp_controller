cmake_minimum_required(VERSION 3.16)

set(IDF_PATH $ENV{IDF_PATH})
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(COMPONENTS main)
project(esp32_temp_control)
