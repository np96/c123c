set(CMAKE_CXX_STANDARD 11)
set(LIBFILES
        ServerThread.cpp
        MessageParser.cpp MessageParser.h util.h util.cpp
        Holders/BaseHolder.cpp
        RedisCommands.cpp RedisCommands.h RedisReplyHolder.h
        NonCopyable.h RequestHandler.h
        RequestFunctions.cpp RequestFunctions.h
        Holders/SocketHolder.cpp Holders/SocketHolder.h
        Holders/RedisContextHolder.cpp Holders/RedisContextHolder.h)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
find_package(event REQUIRED)
find_package(hiredis REQUIRED)
find_library(evhtp REQUIRED)
include_directories(${LIBHIREDIS_INCLUDE_DIR})
include_directories(${LIBEVENT_INCLUDE_DIR})
include_directories(${evhtp_INCLUDE_DIR})
set(LIBS ${LIBS} ${evhtp_LIBRARIES})
set(LIBS ${LIBS} ${LIBHIREDIS_LIBRARIES})
set(LIBS ${LIBS} ${LIBEVENT_LIB})
add_library(ServerLibrary ${LIBFILES})
link_libraries(ServerLibrary LIBS)
target_include_directories (ServerLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})