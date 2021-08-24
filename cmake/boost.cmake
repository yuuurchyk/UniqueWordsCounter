if(NOT UNIX AND NOT WIN32)
    message(FATAL_ERROR "Cannot deduce operating system type: neither UNIX, nor Windows")
endif()

FetchContent_Declare(
    Boost
    URL  https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.7z
    URL_HASH SHA256=3a8b314871646e7404886736273c053278ba71b11347f72d4751411d65d6d1a5
)
FetchContent_GetProperties(boost)

set(BoostBuildNeeded ON CACHE BOOL "Whether in-place boost needs building")

if(NOT boost_POPULATED)
    FetchContent_Populate(boost)

    set(boostInstallPath ${boost_BINARY_DIR}/install)

    if($CACHE{BoostBuildNeeded})
        message(STATUS "Building boost in-place")

        # use relative paths in boost build tools
        # in order not to encounter issues with spaces
        # in full paths
        file(RELATIVE_PATH binaryDirRelativePath ${boost_SOURCE_DIR} ${boost_BINARY_DIR})
        set(boostInstallRelativePath ${binaryDirRelativePath}/install)

        # Step 1: bootstrap
        set(bootstrapExecutable "")
        set(bootstrapArgs "")
        if(UNIX)
            list(APPEND bootstrapExecutable "bash")
            list(APPEND bootstrapExecutable "./bootstrap.sh")
            list(APPEND bootstrapArgs "--with-libraries=program_options,iostreams")
            list(APPEND bootstrapArgs "--prefix=${boostInstallRelativePath}")
        elseif(WIN32)
            list(APPEND bootstrapExecutable ".\\bootstrap.bat")
        endif()
        list(JOIN bootstrapExecutable " " bootstrapExecutableName)
        set(bootstrapOutputFile "${boost_BINARY_DIR}/bootstrap_output.log")

        message(STATUS "Running ${bootstrapExecutableName}")
        execute_process(
            COMMAND             ${bootstrapExecutable} ${bootstrapArgs}
            WORKING_DIRECTORY   ${boost_SOURCE_DIR}
            OUTPUT_FILE         ${bootstrapOutputFile}
            ERROR_FILE          ${bootstrapOutputFile}
            RESULT_VARIABLE     bootstrapResult
            COMMAND_ECHO        STDOUT
        )
        if(bootstrapResult)
            message(FATAL_ERROR "${bootstrapExecutableName} failed, see logs at ${bootstrapOutputFile}")
        else()
            message(STATUS "Done")
        endif()

        # Step 2: b2
        set(b2Executable "")
        if(UNIX)
            set(b2Executable "b2")
        elseif(WIN32)
            set(b2Executable ".\\b2.exe")
        endif()
        set(b2OutputFile "${boost_BINARY_DIR}/bootstrap_output.log")

        message(STATUS "Running ${b2Executable}")
        execute_process(
            COMMAND             ${b2Executable}
                                --prefix=${boostInstallRelativePath}
                                --build-dir=${binaryDirRelativePath}/build
                                --with-program_options
                                --with-iostreams
                                link=static
                                threading=multi
                                install
            WORKING_DIRECTORY   ${boost_SOURCE_DIR}
            OUTPUT_FILE         ${b2OutputFile}
            ERROR_FILE          ${b2OutputFile}
            RESULT_VARIABLE     b2Result
            COMMAND_ECHO        STDOUT
        )
        if(b2Result)
            message(FATAL_ERROR "${b2Executable} failed, see logs at ${b2OutputFile}")
        else()
            message(STATUS "Done")
        endif()

        set(BoostBuildNeeded OFF CACHE BOOL "Whether in-place boost needs building" FORCE)
    else()
        message(STATUS "Boost has already been built")
    endif()

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_NO_SYSTEM_PATHS ON)
    set(BOOST_ROOT ${boostInstallPath})

    find_package(Boost 1.75 EXACT REQUIRED COMPONENTS program_options iostreams)
endif()
