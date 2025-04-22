cmake_minimum_required(VERSION 3.5.1)
project(pulsarlib VERSION 1.0.1)
include(GNUInstallDirs)

# file(GLOB pulsarlib_sources "src/*.cpp")
# file(GLOB pulsarlib_headers "src/*.h")
# add_library(pulsarlib SHARED ${pulsarlib_sources})

add_definitions(-D_UNIX -D_ENABLE_PROFILER_ -D_ENABLE_TRACING_ -D_ENABLE_VISUALIZATION_ -D_MONTE_CARLO_ -D_NO_ROOT_ -I${CMAKE_SOURCE_DIR}/src)

#set_target_properties(pulsarlib PROPERTIES
#    VERSION ${PROJECT_VERSION}
#    SOVERSION 1
#    PUBLIC_HEADER "${pulsarlib_headers}")
 
#configure_file(pulsarlib.pc.in pulsarlib.pc @ONLY)


#install(TARGETS pulsarlib
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)
#install(FILES ${CMAKE_BINARY_DIR}/pulsarlib.pc
#    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


find_library(CFITSIO_LIB cfitsio HINTS ENV LD_LIBRARY_PATH)
find_library(FFTW3_LIB fftw3 HINTS ENV FFTW_LIB REQUIRED)
find_library(LIBNOVA_LIB nova PATHS ENV LD_LIBRARY_PATH)


# required to properly link the dynamic library :
# target_link_libraries(pulsarlib ${CFITSIO_LIB} ${LIBNOVA_LIB} ${ROOT_LIBRARIES} ${FFTW3_LIB} -ldl -lpthread)

add_executable(main_structure_function_1D apps/main_structure_function_1D.cpp)
target_link_libraries(main_structure_function_1D msfitslib ${CFITSIO_LIB} ${LIBNOVA_LIB} ${ROOT_LIBRARIES} ${FFTW3_LIB} -ldl -lpthread)

add_executable(main_autocorr_1D apps/main_autocorr_1D.cpp)
target_link_libraries(main_autocorr_1D msfitslib ${CFITSIO_LIB} ${LIBNOVA_LIB} ${ROOT_LIBRARIES} ${FFTW3_LIB} -ldl -lpthread)

