#ifndef LOG_GLOBAL_H
#define LOG_GLOBAL_H

#include <QtCore/qglobal.h>
#ifndef QT_STATIC
#if defined(LOG_LIBRARY)
#  define LOG_EXPORT Q_DECL_EXPORT
#else
#  define LOG_EXPORT Q_DECL_IMPORT
#endif
#else
#define LOG_EXPORT
#endif
#endif // LOG_GLOBAL_H
