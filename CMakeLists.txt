cmake_minimum_required(VERSION 3.20)
project(AmbaTrain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ----------------------------------------------------------------------------
# 1. Dependensi JSON: nlohmann_json
# ----------------------------------------------------------------------------
find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "[AmbaTrain] Mengunduh nlohmann/json via FetchContent...")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# ----------------------------------------------------------------------------
# 2. Dependensi Web Framework: Crow (Standalone Asio)
# ----------------------------------------------------------------------------
find_package(Crow CONFIG QUIET)
if(NOT Crow_FOUND)
    message(STATUS "[AmbaTrain] Mengunduh Crow via FetchContent...")
    set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CROW_USE_BOOST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.2.0
    )
    FetchContent_MakeAvailable(Crow)
endif()

# ----------------------------------------------------------------------------
# 3. Dependensi Database & Keamanan: libpqxx & libsodium
# ----------------------------------------------------------------------------
find_package(libpqxx CONFIG QUIET)
find_package(jwt-cpp CONFIG QUIET)
find_package(libsodium CONFIG QUIET)

include_directories(src)

# Kumpulkan semua file sumber C++
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

if(SRC_FILES)
    add_executable(ambatrain ${SRC_FILES})

    # Link nlohmann_json
    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(ambatrain PRIVATE nlohmann_json::nlohmann_json)
    endif()

    # Link Crow
    if(TARGET Crow::Crow)
        target_link_libraries(ambatrain PRIVATE Crow::Crow)
    endif()

    # Link libpqxx (dukung CMake Target dan Direct Library untuk Ubuntu/Linux)
    if(TARGET libpqxx::pqxx)
        target_link_libraries(ambatrain PRIVATE libpqxx::pqxx)
    else()
        find_library(LIBPQXX_LIB pqxx)
        find_library(LIBPQ_LIB pq)
        if(LIBPQXX_LIB)
            target_link_libraries(ambatrain PRIVATE ${LIBPQXX_LIB})
            if(LIBPQ_LIB)
                target_link_libraries(ambatrain PRIVATE ${LIBPQ_LIB})
            endif()
        endif()
    endif()

    # Link jwt-cpp jika ditemukan
    if(TARGET jwt-cpp::jwt-cpp)
        target_link_libraries(ambatrain PRIVATE jwt-cpp::jwt-cpp)
    endif()

    # Link libsodium (dukung CMake Target dan Direct Library)
    if(TARGET libsodium::libsodium)
        target_link_libraries(ambatrain PRIVATE libsodium::libsodium)
    else()
        find_library(LIBSODIUM_LIB sodium)
        if(LIBSODIUM_LIB)
            target_link_libraries(ambatrain PRIVATE ${LIBSODIUM_LIB})
        endif()
    endif()

    # Windows socket & security libraries jika menggunakan MinGW
    if(WIN32)
        target_link_libraries(ambatrain PRIVATE ws2_32 wsock32 crypt32)
    endif()

    # Linux pthread
    if(UNIX AND NOT APPLE)
        target_link_libraries(ambatrain PRIVATE pthread)
    endif()
endif()

# ----------------------------------------------------------------------------
# 4. Unit Testing (GoogleTest)
# ----------------------------------------------------------------------------
enable_testing()
find_package(GTest CONFIG QUIET)
file(GLOB_RECURSE TEST_FILES "tests/*.cpp")
if(GTest_FOUND AND TEST_FILES)
    add_executable(ambatrain_tests ${TEST_FILES})
    target_link_libraries(ambatrain_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
    )
    if(TARGET Crow::Crow)
        target_link_libraries(ambatrain_tests PRIVATE Crow::Crow)
    endif()
    if(TARGET nlohmann_json::nlohmann_json)
        target_link_libraries(ambatrain_tests PRIVATE nlohmann_json::nlohmann_json)
    endif()
    add_test(NAME AllTests COMMAND ambatrain_tests)
endif()
