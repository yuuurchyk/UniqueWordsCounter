set(TBB_TEST OFF CACHE BOOL "Enable building tbb tests" FORCE)

FetchContent_Declare(
    tbb
    GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
    GIT_TAG        origin/master
)
FetchContent_MakeAvailable(tbb)
