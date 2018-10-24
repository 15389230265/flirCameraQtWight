#include "mylogging.h"

MyLogging::MyLogging(QTextEdit *qtextedit)
{
    this->OutQTextEdit = qtextedit;
}

QString MyLogging::addTime(const QString &str)
{
    return QString("%1 %2 :%3").arg(__DATE__).arg(__TIME__).arg(str);
}

void MyLogging::MyPrintLog(const logs &kwarg, const std::string &str)
{
    switch (kwarg) {
    case INFO:{
        this->OutQTextEdit->setTextColor(QColor(0, 0, 0));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    case WAR:{
        this->OutQTextEdit->setTextColor(QColor(255, 0, 0));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    case ERRO:{
        this->OutQTextEdit->setTextColor(QColor(0, 255, 0));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    case ANNO:{
        this->OutQTextEdit->setTextColor(QColor(0, 0, 255));
        this->OutQTextEdit->insertPlainText(this->addTime(QString::fromStdString(str)));
        break;
    }
    }
}
