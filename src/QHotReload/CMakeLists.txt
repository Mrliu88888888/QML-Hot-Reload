project(QHotReload)

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
install(DIRECTORY ${QT_BIN_PATH}/../qml DESTINATION bin)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
