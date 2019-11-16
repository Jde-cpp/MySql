#pragma once

#ifdef JDE_MYSQL_EXPORTS
	#ifdef _MSC_VER
		#define JDE_MYSQL_VISIBILITY __declspec( dllexport )
	#else
		#define JDE_MYSQL_VISIBILITY __attribute__((visibility("default")))
	#endif
#else 
	#ifdef _MSC_VER 
		#define JDE_MYSQL_VISIBILITY __declspec( dllimport )
		#if NDEBUG
			#pragma comment(lib, "Jde.DB.MySql.lib")
		#else
			#pragma comment(lib, "Jde.DB.MySql.lib")
		#endif
	#else
		#define JDE_MYSQL_VISIBILITY
	#endif
#endif