cmake_minimum_required(VERSION 3.29)

option(BUILD_TESTING "build tests" OFF)

if(BUILD_TESTING)
	list(APPEND VCPKG_MANIFEST_FEATURES "test")
endif()

project(QHotReload VERSION 1.1.0)

message(STATUS "${PROJECT_NAME} build tests: ${BUILD_TESTING}")

include(cmake/Common.cmake)
include(cmake/ThirdParty.cmake)
include(cmake/PackUtils.cmake)
include(cmake/Pack.cmake)

install(FILES "CHANGELOG.md" DESTINATION .)
install(DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/
	DESTINATION bin
	FILES_MATCHING
	PATTERN "*"
	PATTERN "*.exp" EXCLUDE
	PATTERN "*.ilk" EXCLUDE
	PATTERN "*.lib" EXCLUDE
	PATTERN "*.pdb" EXCLUDE
	PATTERN "*.a" EXCLUDE
)

add_subdirectory(src)
if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
