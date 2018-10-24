#ifndef MYLOGGING_H
#define MYLOGGING_H

#include <string>
#include <QTextEdit>
#include <type_traits>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDebug>
#include <QDir>

class MyLogging
{
public:
    MyLogging(QTextEdit * qtextedit);
    typedef enum {INFO, WAR, ERRO, ANNO} logs;
    template<typename TYPE>
    void MyPrintLogAll(const logs &kwarg, TYPE data)
    {
        this->MyPrintLog(kwarg, data);
 /*       if(std::is_same<typename std::decay<TYPE>::type, QString>::value)
        {
            QString str = data;
            this->MyPrintLog(kwarg, str.toStdString());
        }
        if( std::is_same<typename std::decay<TYPE>::type, char*>::value)
        {
            std::cout << "string & char" << std::endl;
            this->MyPrintLog(kwarg, data);
        }
        if(std::is_same<typename std::decay<TYPE>::type, unsigned int>::value)
        {
            std::cout << "int" << std::endl;
            int num = int(data);
            this->MyPrintLog(kwarg, std::to_string(num));
        }
        else{
            this->MyPrintLog(kwarg, data);
        }*/
    }
    template<> void MyPrintLogAll<QString>(const logs &kwarg, QString data)
    {
        this->MyPrintLog(kwarg, data.toStdString());
    }
    template<> void MyPrintLogAll<unsigned int>(const logs &kwarg, unsigned int data)
    {
        this->MyPrintLog(kwarg, std::to_string(data));
    }  

private:
    QString strSaveFile = QString("%1_%2.txt").arg(__DATE__).arg(__TIME__).remove(QRegExp("\\s")).remove(QRegExp(":"));
    QTextEdit * OutQTextEdit;
    QString addTime(const QString &str);
    void MyPrintLog(const logs &kwarg, const std::string &str);
    bool creatSaveLogPath(QString &strSaveFile);
};

#endif // MYLOGGING_H
