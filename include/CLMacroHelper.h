#pragma once

#define CL_MESSAGE(str) __pragma (message(str))

#define CL_MESSAGE_RUNTIME(str) CL::OutputDebugStringCharPtr(str);

#define CL_MAKE_MESSAGE(function_signature) #function_signature 
#define CL_MAKE_CHAR_PTR_SUMM_IMPLEMETATION(A, B) A ## B
#define CL_MAKE_CHAR_PTR_SUMM(A, B) CL_MAKE_CHAR_PTR_SUMM_IMPLEMETATION(A, B)
#define CL_STRINGLIZE_IMPLEMETATION_CHAR_PTR(A) A
#define CL_STRINGLIZE_CHAR_PTR(A) CL_STRINGLIZE_IMPLEMETATION_CHAR_PTR(A)
#define CL_STRINGLIZE_IMPLEMETATION(A) #A
#define CL_STRINGLIZE(A) CL_STRINGLIZE_IMPLEMETATION(A)

#define CL_NO_DISCARD [[nodiscard]]
#define CL_MAX(A, B) ((A) > (B) ? (A) : (B))
#define CL_MIN(A, B) ((A) < (B) ? (A) : (B))
#define CL_ALIGN_DIVIDE(A, align) ((A) / (align) + (((A) % (align)) ? 1 : 0))
#define CL_ALIGN(A, align) (CL_ALIGN_DIVIDE(A, (align)) * (align))
#define CL_ALIGN_BIN_2(A, align2) (((((A) + (1 << align2)) >> align2)) << align2)
#define CL_SWAP(A, B) {auto tmp = B; B = A; A = tmp;}

#define CL_DELETE_ARRAY_PTR(ptr) if(ptr) {delete[] ptr; ptr = nullptr;}
#define CL_DELETE_PTR(ptr)       if(ptr) {delete ptr; ptr = nullptr;}
#define CL_FORCEINLINE __forceinline

#define CL_DELETE_COPY_OPERATORS(class_name)\
private:\
class_name& operator = (const class_name&);\
class_name(const class_name&);

#define CL_FUNCSIG()\
	CL_STRINGLIZE_CHAR_PTR(__FUNCSIG__)\
	" in file "\
	CL_STRINGLIZE(__FILE__)\
	" on line "\
	CL_STRINGLIZE(__LINE__)

#define CL_COMPILATION_TIME_WARNING(message)\
CL_MESSAGE("Warning:")\
CL_MESSAGE(message)\
CL_MESSAGE(CL_MAKE_CHAR_PTR_SUMM("in ", CL_FUNCSIG()))

namespace CL
{
	void OutputDebugStringCharPtr(const char* pStr);
}