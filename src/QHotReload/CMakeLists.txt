project(QHotReload-${QT_VERSION}-${QT_TOOLCHAIN_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(${PROJECT_NAME} "main.cpp" "qml.qrc" "res.qrc")
if(WIN32)
	target_sources(${PROJECT_NAME} PRIVATE "res/Top.rc")
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_BINARY_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE
	${QT}::Core ${QT}::Gui ${QT}::Widgets ${QT}::Quick
	${THIRDPARTY_FILEWATCHER_LIBS}
)

DEPLOYQT(${PROJECT_NAME} bin)
install(DIRECTORY ${QT_BIN_PATH}/../qml DESTINATION bin
	FILES_MATCHING
	PATTERN "*"
	PATTERN "*.pdb" EXCLUDE
	PATTERN "*.dll" EXCLUDE
)
file(GLOB_RECURSE QML_DLL_FILES ${QT_BIN_PATH}/../qml/*.dll)
foreach(dll_file ${QML_DLL_FILES})
	string(REPLACE ".dll" "d.dll" tmp_file ${dll_file})
	if(NOT EXISTS ${tmp_file})
		continue()
	endif()
	get_filename_component(dll_dir ${dll_file} PATH)
	file(RELATIVE_PATH dll_relative_dir ${QT_BIN_PATH}/.. ${dll_dir})
	install(FILES ${dll_file} DESTINATION bin/${dll_relative_dir})
endforeach()
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
