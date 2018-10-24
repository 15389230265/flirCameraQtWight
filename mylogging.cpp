#include "mylogging.h"

MyLogging::MyLogging(QTextEdit *qtextedit)
{
    this->OutQTextEdit = qtextedit;
    this->creatSaveLogPath(this->strSaveFile);
}

QString MyLogging::addTime(const QString &str)
{
    return QString("%1 %2 :%3").arg(__DATE__).arg(__TIME__).arg(str);
}

//template <typename TYPE>
//void MyLogging::MyPrintLogAll(const logs &kwarg, TYPE data)
//{
//    if(std::is_same<typename std::decay<TYPE>::type,std::string>::value || std::is_same<typename std::decay<TYPE>::type,char>::value)
//    {
//        this->MyPrintLog(kwarg, data);
//    }
//    if(std::is_same<typename std::decay<TYPE>::type, QString>::value)
//    {
//        this->MyPrintLog(kwarg, data.toStdString());
//    }
//    if(std::is_same<typename std::decay<TYPE>::type, int>::value)
//    {
//        this->MyPrintLog(kwarg, std::to_string(data));
//    }
//}

void MyLogging::MyPrintLog(const logs &kwarg, const std::string &str)
{
    switch (kwarg) {
    case INFO:{
        this->OutQTextEdit->setTextColor(QColor(0, 0, 0));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    case WAR:{
        this->OutQTextEdit->setTextColor(QColor(200, 200, 50));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    case ERRO:{
        this->OutQTextEdit->setTextColor(QColor(200, 0, 50));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    case ANNO:{
        this->OutQTextEdit->setTextColor(QColor(0, 150, 200));
        this->OutQTextEdit->insertPlainText("\n");
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str) + "\n"));
        break;
    }
    }
}

bool MyLogging::creatSaveLogPath(QString &strSaveFile){
    QString strWorkPath = QDir::currentPath();
    QString strSaveLogPath = strWorkPath + "/Mylog";
    QDir saveLogDir(strSaveLogPath);
    if(!saveLogDir.exists()){
        if(!saveLogDir.mkpath(strSaveLogPath)){
            qDebug() << "ERRO: Canot creat dir" << strSaveLogPath;
            return false;
        }
    }
    QFile file(strSaveLogPath + "/" + strSaveFile);
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "ERRO: Canot creat file" << strSaveLogPath + "/" + strSaveFile;
        return false;
    }
    file.close();
    return true;
}
