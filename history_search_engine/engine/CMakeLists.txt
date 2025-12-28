cmake_minimum_required(VERSION 3.15)
project(HistorySearchEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(search_engine
    src/main.cpp
    src/tokenizer.cpp
    src/stemmer.cpp
    src/inverted_index.cpp
    src/boolean_search.cpp
    src/zipf_analyzer.cpp
    src/json_reader.cpp
)

target_link_libraries(search_engine stdc++fs)