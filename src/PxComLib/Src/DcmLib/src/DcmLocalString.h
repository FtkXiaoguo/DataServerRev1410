
#ifndef DCM_LOCAL_STRING_H
#define DCM_LOCAL_STRING_H

#include <string>
//#define dllString std::string

 
#define dcm_string std::string
#define stringTochar(dcm_string) dcm_string.c_str() 
#define charTostring(c) std::string(c) 
#define dcm_string_size(dcm_string) dcm_string.size()

#endif //DCM_LOCAL_STRING_H