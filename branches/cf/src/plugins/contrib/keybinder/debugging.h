#ifndef __WX_DEBUGGING_H__
#define __WX_DEBUGGING_H__
//#include <manager.h>
//#include <messagemanager.h>


#define NOT_EQUAL !=

#if defined(kbLOGGING)
    #define LOGGING 1
#endif
#define wxLogMessage wxLogDebug
#define LOGIT wxLogDebug

#if LOGGING
    #undef wxLogMessage
    #undef LOGIT
   // wxMSW wont write msg to our log window via wxLogDebug
   #ifdef __WXMSW__
    #define TRAP asm("int3")
    #define LOGIT wxLogMessage
   #endif
   // wxGTK wxLogMessage turns into a wxMessage in GTK
   #ifdef __WXGTK__
    #define LOGIT wxLogMessage
   #endif
   #ifdef __WXMAC__
    #define TRAP asm("trap")
   #endif
#endif

#endif  //__WX_DEBUGGING_H__