
#ifndef SMS_EXPORT_H
#define SMS_EXPORT_H

#ifdef SMS_STATIC_DEFINE
#  define SMS_EXPORT
#  define SMS_NO_EXPORT
#else
#  ifndef SMS_EXPORT
#    ifdef sms_EXPORTS
        /* We are building this library */
#      define SMS_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SMS_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SMS_NO_EXPORT
#    define SMS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SMS_DEPRECATED
#  define SMS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SMS_DEPRECATED_EXPORT
#  define SMS_DEPRECATED_EXPORT SMS_EXPORT SMS_DEPRECATED
#endif

#ifndef SMS_DEPRECATED_NO_EXPORT
#  define SMS_DEPRECATED_NO_EXPORT SMS_NO_EXPORT SMS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SMS_NO_DEPRECATED
#    define SMS_NO_DEPRECATED
#  endif
#endif

#endif /* SMS_EXPORT_H */
