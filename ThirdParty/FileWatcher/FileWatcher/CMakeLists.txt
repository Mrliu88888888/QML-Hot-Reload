project(FileWatcher)

if(WIN32)
	set(FILEWATCHER_SOURCE_FILE "SimpleFileWatcher/FileWatcherWin32.cpp")
elseif(UNIX)
	set(FILEWATCHER_SOURCE_FILE "SimpleFileWatcher/FileWatcherLinux.cpp")
elseif(APPLE)
	set(FILEWATCHER_SOURCE_FILE "SimpleFileWatcher/FileWatcherOSX.cpp")
endif()

add_library(${PROJECT_NAME}
	"FileChangeNotifier.cpp"
	"SimpleFileWatcher/FileWatcher.cpp"
	${FILEWATCHER_SOURCE_FILE}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE FILEWATCHER_EXPORTS)
target_include_directories(${PROJECT_NAME} PUBLIC .)
