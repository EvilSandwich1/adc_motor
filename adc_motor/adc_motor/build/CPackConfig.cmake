# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor;D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor/build")
set(CPACK_CMAKE_GENERATOR "Visual Studio 17 2022")
set(CPACK_COMPONENTS_ALL "")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "C:/Program Files/CMake/share/cmake-3.30/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "adc_motor built using CMake")
set(CPACK_GENERATOR "NSIS")
set(CPACK_INNOSETUP_ARCHITECTURE "x86")
set(CPACK_INSTALL_CMAKE_PROJECTS "D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor/build;adc_motor;ALL;/")
set(CPACK_INSTALL_PREFIX "C:/Program Files (x86)/adc_motor")
set(CPACK_MODULE_PATH "")
set(CPACK_NSIS_DISPLAY_NAME "Stepper App 1.0")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    Delete \"$DESKTOP\\Tydex Golay ADC.lnk\"
")
set(CPACK_NSIS_IGNORE_LICENSE_PAGE "ON")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_MODIFY_PATH "OFF")
set(CPACK_NSIS_MUI_ICON "D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor/res/tydex_logo.ico")
set(CPACK_NSIS_PACKAGE_NAME "StepperApp")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OUTPUT_CONFIG_FILE "D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor/build/CPackConfig.cmake")
set(CPACK_PACKAGE_AUTHOR "TYDEX, LLC")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "C:/Program Files/CMake/share/cmake-3.30/Templates/CPack.GenericDescription.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tydex Software for use with Golay ADC")
set(CPACK_PACKAGE_FILE_NAME "StepperApp")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Stepper App 1.0")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Stepper App 1.0")
set(CPACK_PACKAGE_NAME "Stepper App")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "Tydex, LLC")
set(CPACK_PACKAGE_VERSION "1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_RESOURCE_FILE_LICENSE "C:/Program Files/CMake/share/cmake-3.30/Templates/CPack.GenericLicense.txt")
set(CPACK_RESOURCE_FILE_README "C:/Program Files/CMake/share/cmake-3.30/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "C:/Program Files/CMake/share/cmake-3.30/Templates/CPack.GenericWelcome.txt")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_7Z "ON")
set(CPACK_SOURCE_GENERATOR "7Z;ZIP")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor/build/CPackSourceConfig.cmake")
set(CPACK_SOURCE_ZIP "ON")
set(CPACK_SYSTEM_NAME "win32")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "win32")
set(CPACK_WIX_SIZEOF_VOID_P "4")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "D:/рабочий стол/Курганский ИД/repository/adc_motor/adc_motor/adc_motor/build/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
