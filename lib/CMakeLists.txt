add_library(c_taint SHARED
        Analyses.cc
        CTaintASTConsumer.cc
        CTaintPluginASTAction.cc
        GrammarChecks.cc
        UnparserASTConsumer.cc
)

target_include_directories(c_taint PUBLIC
        ${CLANG_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(c_taint PUBLIC ${CLANG_DEFINITIONS})
target_compile_features(c_taint PUBLIC cxx_std_20)
target_compile_options(c_taint PUBLIC
        -Wall
        -Wextra
        -Werror 
        -Wpedantic
        -Wno-unused-parameter
        -fno-exceptions
        -fno-rtti
)
