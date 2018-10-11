PROJECT(websocket-service)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

cmake_policy(SET CMP0015 NEW)

set(LINK_FLAGS)
set(LIBS)

if(UNIX)
	add_compile_options(-std=c++14)

	find_package(Boost
		1.66.0
		REQUIRED
		COMPONENTS system thread program_options log locale timer filesystem chrono
		)
	include_directories(${Boost_INCLUDE_DIR})
	set(LIBS ${LIBS} ${Boost_LIBRARIES})

	# set(LINK_FLAGS -static -static-libgcc -static-libstdc++)
	set(LIBS ${LIBS} pthread dl z m rt)
else(UNIX)
	if(NOT BOOST_INCLUDEDIR)
		message(FATAL_ERROR, "BOOST_INCLUDEDIR not defined")
	endif(NOT BOOST_INCLUDEDIR)
	if(NOT BOOST_LIBRARYDIR)
		message(FATAL_ERROR, "BOOST_LIBRARYDIR not defined")
	endif(NOT BOOST_LIBRARYDIR)
	message(STATUS "Boost include directory: ${BOOST_INCLUDEDIR}")
	message(STATUS "Boost link directory: ${BOOST_LIBRARYDIR}")
	include_directories(${BOOST_INCLUDEDIR})
	link_directories(${BOOST_LIBRARYDIR})
endif(UNIX)

find_package(MYSQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
set(LIBS ${LIBS} ${MYSQL_LIBRARIES})

find_package(DBPOOL REQUIRED)
include_directories(${DBPOOL_INCLUDE_DIR})
# set(LIBS ${LIBS} ${DBPOOL_LIBRARIES})

ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LINK_FLAGS} ${LIBS})
