include(FetchContent)
fetchcontent_declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 8.1.0
)
fetchcontent_makeavailable(fmt)

message(STATUS "Fetching FMT")
