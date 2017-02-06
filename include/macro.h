#ifndef _MACRO_H_
#define _MACRO_H_

#define __CAT(x) #x

#define SAFE_DELETE(x) {if(NULL != x) {delete (x);(x) = NULL;}}
#define SAFE_DELETE_VEC(x) {if(NULL != x) {delete [] (x);(x) = NULL;}}

/* comment time 2016/12/21 19:07 author:yld
	双字节符号整数
*/
typedef signed short SWORD;

/* comment time 2016/12/21 19:09 author:yld
四字节符号整数
*/
typedef signed int SDWORD;

#ifdef _MSC_VER
	
	typedef unsigned __int64 QWORD;
	typedef signed __int64 SQWORD;
#else
/**
* \brief 八字节无符号整数
*
*/	
typedef unsigned long long QWORD;

/**
* \brief 八字节符号整数
*
*/
typedef signed long long SQWORD;
#endif // _MSC_VER

/* comment time 2016/12/21 19:12 author:yld
	内存清零
*/
#define bzero_(p,s)      memset(p,0,s)
#define bcopy_(d,s,ss) memcpy(d,s,ss)



typedef unsigned long       DWORD;
typedef unsigned short		WORD;
typedef unsigned char		BYTE;



typedef int					SOCKET;


#define MAX_IP_LENGTH  16

#endif
