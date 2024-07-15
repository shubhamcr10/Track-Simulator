
cmake_minimum_required(VERSION 2.8.12)
if (${CMAKE_VERSION} VERSION_GREATER 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(client_examples)

find_package(microcdr REQUIRED)
find_package(microxrcedds_client REQUIRED)

set(SRCS
    Track.c
    )

# # Track Subscriber
# add_executable(TrackSubscriber TrackSubscriber.c ${SRCS})
# target_link_libraries(TrackSubscriber microxrcedds_client)
# set_target_properties(TrackSubscriber PROPERTIES
#     C_STANDARD
#         99
#     C_STANDARD_REQUIRED
#         YES
#     )
# target_compile_options(TrackSubscriber
#     PRIVATE
#         $<$<C_COMPILER_ID:MSVC>:/W4 /wd4700 /wd4996 /wd4820 /wd4255 /wd4668>
#         $<$<C_COMPILER_ID:GNU>:-pedantic -fstrict-aliasing -Wall -Wcast-align -Wshadow>
#     )

# Track Publisher
add_executable(TrackPublisher TrackPublisher.cpp ${SRCS})
target_link_libraries(TrackPublisher microxrcedds_client)
set_target_properties(TrackPublisher PROPERTIES
    C_STANDARD
        99
    C_STANDARD_REQUIRED
        YES
    )
target_compile_options(TrackPublisher
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/W4 /wd4700 /wd4996 /wd4820 /wd4255 /wd4668>
        $<$<C_COMPILER_ID:GNU>:-pedantic -fstrict-aliasing -Wall -Wcast-align -Wshadow>
    )

