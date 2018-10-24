#ifndef MYLOGGING_H
#define MYLOGGING_H

#include <string>
#include <QTextEdit>

class MyLogging
{
public:
    MyLogging(QTextEdit * qtextedit);
    typedef enum {INFO, WAR, ERRO, ANNO} logs;
    void MyPrintLog(const logs &kwarg, const std::string &str);
private:
    QTextEdit * OutQTextEdit;
    QString addTime(const QString &str);
};

#endif // MYLOGGING_H
