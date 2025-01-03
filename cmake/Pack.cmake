set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME}-${QT_VERSION}-${QT_TOOLCHAIN_NAME})
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION}-${CMAKE_BUILD_TYPE})
set(CPACK_PACKAGE_DESCRIPTION "GOOD LUCK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "liumeng")
set(CPACK_PACKAGE_CONTACT "liumeng1105@outlook.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})

set(CPACK_NSIS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
set(CPACK_NSIS_DISPLAY_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/share/icon/Logo.ico")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/${CPACK_PACKAGE_NAME}.exe")
set(CPACK_NSIS_MENU_LINKS
	"bin/${CPACK_PACKAGE_NAME}.exe" "${CPACK_PACKAGE_NAME}"
	"CHANGELOG.md" "CHANGELOG"
)
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\${CPACK_PACKAGE_NAME}.lnk' '$INSTDIR\\\\bin\\\\${CPACK_PACKAGE_NAME}.exe'")

if(WIN32)
	set(CPACK_GENERATOR "ZIP;NSIS64")
elseif(UNIX)
	set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)
