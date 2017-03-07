
set(LibEvent_EXTRA_PREFIXES /usr/local /opt/local "$ENV{HOME}")

foreach (prefix ${LibEvent_EXTRA_PREFIXES})
    list(APPEND LibEvent_INCLUDE_PATHS "${prefix}/include")
    list(APPEND LibEvent_LIB_PATHS "${prefix}/lib")
endforeach ()

find_path(LIBEVENT_INCLUDE_DIR event.h PATHS)
find_library(LIBEVENT_LIB NAMES event PATHS ${LibEvent_LIB_PATHS})

if (LIBEVENT_LIB AND LIBEVENT_INCLUDE_DIR)
    set(LibEvent_FOUND TRUE)
    set(LIBEVENT_LIB ${LIBEVENT_LIB})
else ()
    set(LibEvent_FOUND FALSE)
endif ()

if (LibEvent_FOUND)
    if (NOT LibEvent_FIND_QUIETLY)
        message(STATUS "Found libevent: ${LIBEVENT_LIB}")
    endif ()
else ()
    if (LibEvent_FIND_REQUIRED)
        message(FATAL_E RROR "Could not find libevent.")
    endif ()
    message(WARNING "libevent not found.")
endif ()

mark_as_advanced(
        LIBEVENT_LIB
        LIBEVENT_INCLUDE_DIR
)