#ifndef _MY_LOG_H_
#define  _MY_LOG_H_

//#define __DEBUG__
//#ifdef __DEBUG__
//		//#define  DEBUG(format,...)  printf("File: "__FILE__",Line: %05d: "format" /n",  __LINE__, ##__VA_ARGS__) 
//	//	#define  DEBUG(format,args...)  printf("File: "__FILE__",Line: %05d: "format" \n",  __LINE__, ##args)   
//
// // 由于在网上copy的代码，有编码不一致问题，让我以为 g++ 不支持可变参数宏（恰好互联网上有这方面的论述）
//// 折腾了一晚上
//		#define  DEBUG(format,...)   printf("File: "__FILE__", Line: %u: "format" \n", __LINE__,##__VA_ARGS__)
//#else
//	#define  DEBUG(format,...)  
//#endif

#include<android/log.h>

# define	MY_LOG_LEVEL_VERBOSE 	1
# define	MY_LOG_LEVEL_DEBUG			2
# define	MY_LOG_LEVEL_INFO				3
# define	MY_LOG_LEVEL_WARNING	4
# define	MY_LOG_LEVEL_ERROR			5
# define	MY_LOG_LEVEL_FATAL			6
# define	MY_LOG_LEVEL_SLIENT			7

#	ifndef MY_LOG_TAG
		#define MY_LOG_TAG  __FILE__
#endif

#ifndef MY_LOG_LEVEL
		#define MY_LOG_LEVEL   MY_LOG_LEVEL_VERBOSE
#endif

#define MY_LOG_NOOP 		(void) 0

#define  MY_LOG_PRINT( level, fmt, ...) \
				__android_log_print(level, MY_LOG_TAG, "(%s:%u)  %s: "  fmt,   \
						__FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)

#if   MY_LOG_LEVEL_VERBOSE >= MY_LOG_LEVEL
		#define MY_LOG_VERBOSE(fmt, ...) \
			MY_LOG_PRINT(MY_LOG_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
#else
		#define MY_LOG_VERBOSE(fmt, ...)   MY_LOG_NOOP
#endif

#if   MY_LOG_LEVEL_DEBUG>= MY_LOG_LEVEL
		#define MY_LOG_DEBUG(fmt, ...) \
			MY_LOG_PRINT(MY_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
		#define MY_LOG_DEBUG(fmt, ...)   MY_LOG_NOOP
#endif

#if   MY_LOG_LEVEL_INFO >= MY_LOG_LEVEL
		#define MY_LOG_INFO(fmt, ...) \
			MY_LOG_PRINT(MY_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#else
		#define MY_LOG_INFO(fmt, ...)   MY_LOG_NOOP
#endif

#if   MY_LOG_LEVEL_WARNING >= MY_LOG_LEVEL
		#define MY_LOG_WARNING(fmt, ...) \
			MY_LOG_PRINT(MY_LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#else
		#define MY_LOG_WARNING(fmt, ...)   MY_LOG_NOOP
#endif

#if   MY_LOG_LEVEL_ERROR >= MY_LOG_LEVEL
		#define MY_LOG_ERROR(fmt, ...) \
			MY_LOG_PRINT(MY_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#else
		#define MY_LOG_ERROR(fmt, ...)   MY_LOG_NOOP
#endif


#if   MY_LOG_LEVEL_FATAL >= MY_LOG_LEVEL
		#define MY_LOG_FATAL(fmt, ...) \
			MY_LOG_PRINT(MY_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#else
		#define MY_LOG_FATAL(fmt, ...)   MY_LOG_NOOP
#endif



#endif   //_MY_LOG_H_
