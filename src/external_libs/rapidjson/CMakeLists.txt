cmake_minimum_required(VERSION 3.10.2)

project(rapidjson LANGUAGES CXX)

set(_RAPIDJSON_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}) 

add_library(
    ${PROJECT_NAME} STATIC
        ${_RAPIDJSON_INC_DIR}/error/en.h
        ${_RAPIDJSON_INC_DIR}/error/error.h
        
        ${_RAPIDJSON_INC_DIR}/internal/biginteger.h
        ${_RAPIDJSON_INC_DIR}/internal/diyfp.h
        ${_RAPIDJSON_INC_DIR}/internal/dtoa.h
        ${_RAPIDJSON_INC_DIR}/internal/ieee754.h
        ${_RAPIDJSON_INC_DIR}/internal/itoa.h
        ${_RAPIDJSON_INC_DIR}/internal/meta.h
        ${_RAPIDJSON_INC_DIR}/internal/pow10.h
        ${_RAPIDJSON_INC_DIR}/internal/regex.h
        ${_RAPIDJSON_INC_DIR}/internal/stack.h
        ${_RAPIDJSON_INC_DIR}/internal/strfunc.h
        ${_RAPIDJSON_INC_DIR}/internal/strtod.h
        ${_RAPIDJSON_INC_DIR}/internal/swap.h
        
        ${_RAPIDJSON_INC_DIR}/msinttypes/inttypes.h
        ${_RAPIDJSON_INC_DIR}/msinttypes/stdint.h

        ${_RAPIDJSON_INC_DIR}/allocators.h
        ${_RAPIDJSON_INC_DIR}/document.h
        ${_RAPIDJSON_INC_DIR}/encodedstream.h
        ${_RAPIDJSON_INC_DIR}/filereadstream.h
        ${_RAPIDJSON_INC_DIR}/filewritestream.h
        ${_RAPIDJSON_INC_DIR}/fwd.h
        ${_RAPIDJSON_INC_DIR}/istreamwrapper.h
        ${_RAPIDJSON_INC_DIR}/memorybuffer.h
        ${_RAPIDJSON_INC_DIR}/memorystream.h
        ${_RAPIDJSON_INC_DIR}/ostreamwrapper.h
        ${_RAPIDJSON_INC_DIR}/pointer.h
        ${_RAPIDJSON_INC_DIR}/prettywriter.h
        ${_RAPIDJSON_INC_DIR}/rapidjson.h
        ${_RAPIDJSON_INC_DIR}/reader.h
        ${_RAPIDJSON_INC_DIR}/schema.h
        ${_RAPIDJSON_INC_DIR}/stream.h
        ${_RAPIDJSON_INC_DIR}/stringbuffer.h
        ${_RAPIDJSON_INC_DIR}/writer.h
)

target_include_directories(
    ${PROJECT_NAME}
        PUBLIC 
            ${_RAPIDJSON_INC_DIR}
)
    
set_target_properties(rapidjson PROPERTIES LINKER_LANGUAGE CXX)
