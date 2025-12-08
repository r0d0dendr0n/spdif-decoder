cmake_minimum_required (VERSION 3.5)
project (spdif-decoder)

add_executable (spdif-decoder
    codechandler.c
    helper.c
    myspdif.c
    myspdifdec.c
    resample.c
    spdif-loop.c
)

#SET(FFMPEG ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg-4.3.1)
#SET(FFMPEG /usr/local/ffmpeg)
SET(FFMPEG /usr/src/ffmpeg/ffmpeg-git)
#SET(FFMPEG /usr/src/ffmpeg/ffmpeg-4.3.9)

target_include_directories (spdif-decoder PRIVATE ${FFMPEG})
#target_include_directories (spdif-decoder PUBLIC ${FFMPEG}/include)

target_link_directories(spdif-decoder PUBLIC ${FFMPEG}/lib)

# NO_SYSTEM_ENVIRONMENT_PATH NO_DEFAULT_PATH
FIND_LIBRARY(libavcodec avcodec ${FFMPEG}/libavcodec)
FIND_LIBRARY(libavformat avformat ${FFMPEG}/libavformat)
FIND_LIBRARY(libavdevice avdevice ${FFMPEG}/libavdevice)
FIND_LIBRARY(libavutil avutil ${FFMPEG}/libavutil)
FIND_LIBRARY(libswresample swresample ${FFMPEG}/libswresample)
FIND_LIBRARY(libavfilter avfilter ${FFMPEG}/libavfilter)

TARGET_LINK_LIBRARIES(spdif-decoder 
#    ${libavcodec}
#    ${libavformat}
#    ${libavdevice}
#    ${libavutil}
#    ${libswresample}
#    ${libavfilter}

    ${FFMPEG}/libavcodec/libavcodec.so
    ${FFMPEG}/libavformat/libavformat.so
    ${FFMPEG}/libavdevice/libavdevice.so
    ${FFMPEG}/libavutil/libavutil.so
    ${FFMPEG}/libswresample/libswresample.so
    ${FFMPEG}/libavfilter/libavfilter.so

#    ${FFMPEG}/lib/libavcodec.so
#    ${FFMPEG}/lib/libavformat.so
#    ${FFMPEG}/lib/libavdevice.so
#    ${FFMPEG}/lib/libavutil.so
#    ${FFMPEG}/lib/libswresample.so
#    ${FFMPEG}/lib/libavfilter.so

    ao
    m
)
