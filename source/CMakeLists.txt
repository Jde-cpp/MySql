set( CMAKE_CXX_COMPILER clang-12 )
cmake_minimum_required(VERSION 3.16)
project(MySql)
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../Framework/cmake )
include(SetUpConfigurations)
#######################################################
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
add_library( Jde.MySql SHARED MySqlDataSource.cpp MySqlRow.cpp MySqlSchemaProc.cpp  MySqlStatements.cpp )
#######################################################
include_directories( "$ENV{REPO_DIR}/mysql-connector-cpp/include/mysqlx" )
target_link_libraries(Jde.MySql Threads::Threads)
target_link_libraries(Jde.MySql mysqlcppconn8)
#######################################################
target_precompile_headers(Jde.MySql
  PRIVATE
	<vector>
	<boost/crc.hpp>
	<boost/system/error_code.hpp>
	<spdlog/spdlog.h>
	<spdlog/sinks/basic_file_sink.h>
	<spdlog/fmt/ostr.h>
	<nlohmann/json.hpp>
	"../../Framework/source/Exception.h"
	"../../Framework/source/DateTime.h"
	"../../Framework/source/StringUtilities.h"
	<xdevapi.h>
)
