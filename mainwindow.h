#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QMessageBox>
#include "myflircamera.h"
#include "mylogging.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonTest_clicked();

    void on_pushButtonClear_clicked();

    void on_textEdit_textChanged();

    void on_pushButtonSaveLog_clicked();

    void on_pushButtonCatchImg_clicked();

private:
    Ui::MainWindow *ui;
    void my_delay(int ms);
    void init_info();
    bool init_camera();
    bool setTimeStamp(const InterfacePtr & interfacePtr);
    bool catchImgFromCamera();
    bool catchSingalImg(const CameraPtr &pCam, const std::string &camerNum);
    bool creatSaveImgFolder(const std::string &camerNum);
    bool MyDeinitizeCamera();
    void myPrintDeviceInfermation(INodeMap & nodeMap, const unsigned int camNum);
    bool myRunMULtipleCameras(const SystemPtr &system, const InterfaceList &interfaceList, const CameraList &camList);
    bool MyConfigureInterface(const InterfaceList & interfaceList);
    bool MyConfigureIEEE1588(const CameraPtr &pCam, const std::string &camerNum);
    bool MyConfigureActionControl(const CameraPtr &pCam, const std::string &camerNum);
    bool MyConfigureOtherNodes(const CameraPtr &pCam, const std::string &camerNum);
    bool MyConfigureTrigger(const CameraPtr &pCam, const std::string &camerNum);
    bool MyConfigureChunkData(const CameraPtr &pCam, const std::string &camerNum);
    bool MyConfigurAcquisition(const CameraPtr &pCam, const std::string &camerNum);
    SystemPtr system;
    CameraList camList;
    CameraList actureCamList;
    InterfaceList interfaceList;
//    CameraPtr pCam = NULL;
//    InterfacePtr interfacePtr = NULL;
    MyLogging *mylogging;
    vector<std::string>camSerialNumber = {"18261062", "18261059"};
};

#endif // MAINWINDOW_H
