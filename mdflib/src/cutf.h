#pragma once

#include <cstdint>
#include <cstddef>
#include <string>         //strlen
#include <cwchar>          //wcslen

// Can be compiled as .C if necessary.
#ifdef __cplusplus
#include <string>

extern "C"
{
#endif

//----------------------------------------------------------------------------------------
// C based functions
//----------------------------------------------------------------------------------------

//
//  Estimates how much buffer length is required to convert utf-8 to wchar_t (string length + 1)
//
size_t utf8zestimate(const char* s);

//
//  Converts utf-8 string to wide version.
//
//  if out == nullptr - then estimate how much buffer space is needed.
//
//  returns target string length. Length includes extra nul terminating character.
//
size_t utf8towchar(const char* s, size_t inSize, wchar_t* out, size_t outsize);

//
// utf8 zero terminated function
//
size_t utf8ztowchar(const char* s, wchar_t* out, size_t outsize);


//
//  Estimates how much many elements in characters is required to convert wide string to utf-8 (string length + 1)
//
size_t wcharzestimate(const wchar_t* ws);

//
//  Converts wide string to utf-8 string.
//
//  if out == nullptr - then estimate how much buffer space is needed.
//
//  returns filled buffer length (not string length)
//
size_t wchartoutf8(const wchar_t* s, size_t inSize, char* out, size_t outsize);

//
// utf8 zero terminated function
//
size_t wcharztoutf8(const wchar_t* s, char* out, size_t outsize);

int      cutf_is_valid(uint8_t* start, uint8_t* end);
int      cutf_starts_with_bom(uint8_t* start, uint8_t* end);
uint8_t* cutf_append(uint32_t cp, uint8_t* result, size_t* remain);
uint32_t cutf_next(uint8_t** it, size_t* remain);
uint32_t cutf_peek_next(uint8_t* it);
uint32_t cutf_prior(uint8_t** it);
void     cutf_advance(uint8_t** it, size_t n);
size_t   cutf_distance(uint8_t* first, uint8_t* last);
size_t   cutf_16to8(uint16_t* start, uint16_t* end, uint8_t* out, size_t outsize);
size_t   cutf_8to16(uint8_t* start, uint8_t* end, uint16_t* out, size_t outsize);
size_t   cutf_32to8(uint32_t* start, uint32_t* end, uint8_t* out, size_t outsize);
size_t   cutf_8to32(uint8_t* start, uint8_t* end, uint32_t* out, size_t outsize);
size_t   cutf_replace_invalid(uint8_t* start, uint8_t* end, uint8_t* out, size_t limit, uint32_t replacement);
size_t   cutf_default_replace_invalid(uint8_t* start, uint8_t* end, uint8_t* out, size_t limit);

#ifdef __cplusplus
};

//----------------------------------------------------------------------------------------
// C++ based functions
//----------------------------------------------------------------------------------------
std::wstring utf8towide(const char* s);
std::wstring utf8towide(const std::string& s);
std::string  widetoutf8(const wchar_t* ws);
std::string  widetoutf8(const std::wstring& ws);

#endif

