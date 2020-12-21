#include "defines.h"

inline QString ftpToString(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return codec->toUnicode(input.toLatin1());
#else
    return input;
#endif
}

inline QString stringToFtp(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return QString::fromLatin1(codec->fromUnicode(input));
#else
    return input;
#endif
}
