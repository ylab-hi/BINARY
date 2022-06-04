include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 8.1.0
)
FetchContent_MakeAvailable(fmt)

message(STATUS "Fetching FMT")
