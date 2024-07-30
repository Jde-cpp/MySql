cmake_minimum_required(VERSION 3.16)
project( Jde.MySql )
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Public/build )
include(common)
#######################################################
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

include_directories( "${cmakeLibraryPath}/mysql-connector-cpp/include/mysqlx" )
link_directories( debug ${cmakeLibraryPath}/mysql-connector-cpp/lib64/debug optimized ${cmakeLibraryPath}/mysql-connector-cpp/lib64/release )

add_library( ${CMAKE_PROJECT_NAME} SHARED MySqlDataSource.cpp MySqlRow.cpp MySqlSchemaProc.cpp  MySqlStatements.cpp )
#######################################################

include_directories( "../../Public" )
#target_link_libraries( ${CMAKE_PROJECT_NAME} )
target_link_libraries( ${CMAKE_PROJECT_NAME} mysqlcppconn8 )
target_link_libraries( ${CMAKE_PROJECT_NAME} debug libfmtd.a optimized fmt::libfmt.a )
#######################################################
target_precompile_headers( ${CMAKE_PROJECT_NAME}
  PRIVATE
	<xdevapi.h>
	<vector>
	<boost/crc.hpp>
	<boost/system/error_code.hpp>
	<nlohmann/json.hpp>
	<jde/TypeDefs.h>
	"../../Framework/source/DateTime.h"
	<jde/Str.h>
)
