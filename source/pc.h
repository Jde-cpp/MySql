
#include <vector>
#pragma warning( disable : 4245) 
#include <boost/crc.hpp> 
#pragma warning( default : 4245) 
#include <boost/system/error_code.hpp>
#ifndef __INTELLISENSE__
	#include <spdlog/spdlog.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/fmt/ostr.h>
#endif

#include "../../Framework/source/Exception.h"
#include "../../Framework/source/DateTime.h"
#include "../../Framework/source/StringUtilities.h"
#include <xdevapi.h>	