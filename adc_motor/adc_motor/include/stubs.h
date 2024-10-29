#ifndef __STUBS__
#define __STUBS__ 1

//#define LCOMP_LINUX 1

//#ifdef _WINDOWS
//   #include <windows.h>
//#else
//   #include <fcntl.h>
//   #include <unistd.h>
//   #include <sys/ioctl.h>
//   #include <sys/mman.h>


#ifndef DEFINE_GUID
   #include "../include/guiddef.h"

   typedef void * LPVOID;
   typedef void * PVOID;
   typedef void * HANDLE;
   typedef void * HMODULE;
   typedef void * HINSTANCE;
   typedef char CHAR, * PCHAR;
   typedef unsigned char UCHAR, * PUCHAR;
   typedef unsigned int ULONG, ULONG32, * PULONG;             //int !!!!!!!!!!!!!!!!!!!
   typedef int LONG, * PLONG;
   typedef short SHORT, * PSHORT;
   typedef unsigned short USHORT, * PUSHORT;
   typedef LONG HRESULT;


   #ifndef INVALID_HANDLE_VALUE
      #define INVALID_HANDLE_VALUE ((void *)(-1))
   #endif


   #endif   

   #ifndef _WINDOWS
         #define __stdcall
   #endif

  
   // это в salayer linux утащить 
  // #define ERROR_INVALID_FUNCTION           1L    // dderror
//   #define ERROR_FILE_NOT_FOUND             2L
//   #define ERROR_PATH_NOT_FOUND             3L
//   #define ERROR_TOO_MANY_OPEN_FILES        4L
//   #define ERROR_ACCESS_DENIED              5L
//   #define ERROR_INVALID_HANDLE             6L
   //////////


//#endif
#endif
