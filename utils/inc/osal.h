#ifndef OSAL_UTILS
#define OSAL_UTILS

#ifdef WIN
// Windows
#define long long long
#define PATH_MARKER '\\'

#else
// Linux
#define PATH_MARKER '/'

#endif

#endif