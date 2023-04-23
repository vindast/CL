#pragma once
#include "CLMacroHelper.h"
#include <Logger/Logger.h>  

namespace CL
{
	void CERR(const std::string& message);
};

#define CL_CRASH()\
{\
CL::Logger::write("CL_CRASH()");\
 int* p = nullptr;\
*p = 0;\
}

#define CL_ASSERT(exp)\
if (!(exp))\
{\
	const char* message =\
		"ASSERTAION FAILED : '"\
		CL_MAKE_MESSAGE(exp)\
		"' in function "\
		CL_FUNCSIG();\
	CL::CERR(message);\
	CL::Logger::write(message);\
	CL_CRASH();\
}; 

#ifdef _DEBUG
#define CL_DEBUG_ASSERT(exp)\
if (!(exp))\
	{\
		const char* message =\
			"DEBUG ASSERTAION FAILED : '"\
			CL_MAKE_MESSAGE(exp)\
			"' in function "\
			CL_FUNCSIG();\
		CL::CERR(message);\
		CL::Logger::write(message);\
		CL_CRASH();\
	}; 
#else
#define CL_DEBUG_ASSERT(exp)
#endif // _DEBUG\

	

#define CL_FATAL()\
{\
	const char* message =\
	"FATAL : "\
	" in function "\
	CL_FUNCSIG();\
	CL::CERR(message);\
	CL::Logger::write(message);\
	CL_CRASH();\
}

#define CL_FATAL_OUT_OF_RANGE(param_name)\
{\
	const char* message =\
	"FATAL : "\
	" in function "\
	CL_FUNCSIG()\
	" parameter '"\
	CL_STRINGLIZE(param_name)\
	"' out of range! ";\
	CL::CERR(message);\
	CL::Logger::write(message);\
	CL_CRASH();\
}

#define CL_FATAL_MSG(msg)\
{\
	const char* message =\
	"FATAL : "\
	"Message = "\
	CL_STRINGLIZE(msg)\
	" in function "\
	CL_FUNCSIG();\
	CL::Logger::write(message);\
	CL::CERR(message);\
	CL_CRASH();\
}