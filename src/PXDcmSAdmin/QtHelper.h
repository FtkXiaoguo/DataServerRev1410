#ifndef QT_HELP_HH_H
#define QT_HELP_HH_H
 
#define Str2QString(str) (QString::fromLocal8Bit((str).c_str()))

#define QString2Str(Qstr) std::string((Qstr).toLocal8Bit())
//
#define WStr2QString(str) (QString::fromUtf16((const ushort*)((str).c_str())))
 
////////
#define Str2char(str) ((str).c_str())

#define QString2char(Qstr) ((Qstr).toLocal8Bit())
#endif


