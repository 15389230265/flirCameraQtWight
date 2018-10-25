#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
    ui->textEdit->document()->setMaximumBlockCount(1000);
    this->mylogging = new MyLogging(ui->textEdit);
    this->init_info();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonTest_clicked()
{
    if(!this->init_camera()){
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("init_camera erro!\n"));
    }
//    my_delay(10000);
//    if(!this->catchImgFromCamera()){
//        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("catch image erro!\n"));
//    }
//    my_delay(10000);
//    if(!this->MyDeinitizeCamera()){
//        this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Camera deinitize erro!\n");
//    }

}

void MainWindow::on_pushButtonClear_clicked()
{
    ui->textEdit->clear();
    this->init_info();
}

void MainWindow::on_textEdit_textChanged()
{
    ui->textEdit->moveCursor(QTextCursor::End);
}

void MainWindow::my_delay(int ms)
{
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<ms)
        QCoreApplication::processEvents();
}

void MainWindow::init_info()
{
    QString qsDate = QObject::tr("Application build date: ");
    qsDate += QString("%1 %2 \r\n\r\n").arg(__DATE__).arg(__TIME__);
    this->mylogging->MyPrintLogAll<QString>(MyLogging::ANNO, qsDate);
    #ifdef _flir_spin_113
        ui->textEdit->insertPlainText("using flir spinview 1.13\n");
    #endif
    #ifdef _flir_spin_new
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "using flir spinview new\r\n\r\n");
    #endif
}

bool MainWindow::init_camera()
{
    this->system = System::GetInstance();
    this->camList = system->GetCameras();    
    CameraPtr pCam = NULL;
    InterfacePtr interfacePtr = NULL;    
    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "------Camlist Info-------\n");
    unsigned int num_camList = camList.GetSize();
    num_camList = num_camList / 2;
    this->mylogging->MyPrintLogAll<QString>(MyLogging::INFO, QString("Camrea Num : %1\r\n").arg(num_camList));
    this->myPrintDeviceInfermation();
    // Finish if there are no cameras
    if(num_camList == 0){
        camList.Clear();
        system->ReleaseInstance();
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::WAR, "Not enough cameras!\n");
        return false;
    }

    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "------Interfaces Info-------");
    this->interfaceList = system->GetInterfaces();
    const unsigned int num_interfaceList = interfaceList.GetSize();
    this->mylogging->MyPrintLogAll<QString>(MyLogging::INFO, QString("Interfaces Num : %1\r\n").arg(num_interfaceList));
    InterfacePtr interfacePtrLast = NULL;
    gcstring interfaceDisplayNameLast;
    unsigned int cameraListIndex = 0;
    for (unsigned int i = 0; i < num_interfaceList; i++){
        interfacePtr = interfaceList.GetByIndex(i);
        interfacePtr->UpdateCameras();
        INodeMap &nodeMapInterface = interfacePtr->GetTLNodeMap();
        CStringPtr ptrInterfaceDisplayName = nodeMapInterface.GetNode("InterfaceDisplayName");
        const gcstring interfaceDisplayName = ptrInterfaceDisplayName->GetValue();
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::INFO, interfaceDisplayName + "\n");
        CameraList camList1 = interfacePtr->GetCameras();
        if (camList1.GetSize() < num_camList){
            this->mylogging->MyPrintLogAll<const char*>(MyLogging::INFO, interfaceDisplayName + " have no camera.\n");
            continue;
        }else{
            this->mylogging->MyPrintLogAll<const char*>(MyLogging::INFO, interfaceDisplayName + " have camera.\n");
            if(!this->MyConfigureInterface(interfacePtr, string(interfaceDisplayName))){
                this->mylogging->MyPrintLogAll<const char*>(MyLogging::ERRO, interfaceDisplayName + " Configure Setting erro!\r\n");
                return false;
            }
            for(unsigned int j = 0; j < 2; j++){
                if(j == cameraListIndex)
                {
                    continue;
                }else{
                    camList1.RemoveBySerial(camSerialNumber[j]);
                }
            }
            actureCamList.Append(camList1);
            cameraListIndex += 1;
            if(cameraListIndex > num_camList-1){
                break;
            }
            pCam = actureCamList.GetByIndex(cameraListIndex);
            CStringPtr ptrStringSerial = pCam->GetTLDeviceNodeMap().GetNode("DeviceSerialNumber");
            std::string camAdress= string(interfaceDisplayName) + " " + camSerialNumber[cameraListIndex] + "/" + string(ptrStringSerial->GetValue());
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camAdress + " \n");                    
            interfacePtrLast = interfacePtr;
            interfaceDisplayNameLast = interfaceDisplayName;
        }
        my_delay(1000);
    }
    for(unsigned int i = 0; i < actureCamList.GetSize(); i++){
        pCam = actureCamList.GetByIndex(i);
        CStringPtr ptrStringSerial = pCam->GetTLDeviceNodeMap().GetNode("DeviceSerialNumber");
        std::string camAdress= camSerialNumber[cameraListIndex] + "/" + string(ptrStringSerial->GetValue());
        //Initizlize camera
        pCam->Init();
        // Congigure Interface Settingd
        // Configure IEEE 1588 settings
        if(!this->MyConfigureIEEE1588(pCam, camAdress)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, camAdress + "Configure IEEE 1588 settings erro!\r\n");
            return false;
        }
        //
        if(!this->MyConfigureActionControl(pCam, camAdress)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, camAdress + "Configure Action control settings erro!\r\n");
            return false;
        }
        // Configure Action control settings
        if(!this->MyConfigureOtherNodes(pCam, camAdress)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, camAdress + "Configure other node settings for frame synchronization erro!\r\n");
            return false;
        }
        // Configure trigger mode
        if(!this->MyConfigureTrigger(pCam, camAdress)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, camAdress + "Configure trigger mode erro!\r\n");
            return false;
        }
        // Configure chunk data
        if(!this->MyConfigureChunkData(pCam, camAdress)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, camAdress + "Configure chunk data erro!\r\n");
            return false;
        }
        // Configure Acquisition
        if(!this->MyConfigurAcquisition(pCam, camAdress)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, camAdress + "Configure Acquisition erro!\r\n");
            return false;
        }
    }
    if(!this->setTimeStamp(interfacePtrLast)){
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Interface " + string(interfaceDisplayNameLast) + " cannot set timestamps...\n");
        return false;
    }
/*    for (auto camNum : camSerialNumber){
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::ANNO, "Camera Num : " + camNum );
        pCam = camList.GetBySerial(camNum);
        //Initizlize camera
        pCam->Init();
        // Congigure Interface Settingd
        // Configure IEEE 1588 settings
        if(!this->MyConfigureIEEE1588(pCam, camNum)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Camrea" + camNum + "Configure IEEE 1588 settings erro!\r\n");
            return false;
        }
        //
        if(!this->MyConfigureActionControl(pCam, camNum)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Camrea" + camNum + "Configure Action control settings erro!\r\n");
            return false;
        }
        // Configure Action control settings
        if(!this->MyConfigureOtherNodes(pCam, camNum)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Camrea" + camNum + "Configure other node settings for frame synchronization erro!\r\n");
            return false;
        }
        // Configure trigger mode
        if(!this->MyConfigureTrigger(pCam, camNum)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Camrea" + camNum + "Configure trigger mode erro!\r\n");
            return false;
        }
        // Configure chunk data
        if(!this->MyConfigureChunkData(pCam, camNum)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Camrea" + camNum + "Configure chunk data erro!\r\n");
            return false;
        }
        // Configure Acquisition
        if(!this->MyConfigurAcquisition(pCam, camNum)){
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::ERRO, "Camrea" + camNum + "Configure Acquisition erro!\r\n");
            return false;
        }
        this->my_delay(1000);
    }*/

    return true;
}

bool MainWindow::catchImgFromCamera(){
    CameraPtr pCam = NULL;
    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CATCH IMAGE FROM CAMERA ***");
    try {
        for (auto camNum : camSerialNumber){
            pCam = camList.GetBySerial(camNum);
            this->catchSingalImg(pCam, camNum);
            pCam->EndAcquisition();
        }
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::catchSingalImg(const CameraPtr &pCam, const std::string &camerNum){
    try {
        // Retrieve next received image and ensure image completion
        ImagePtr pResultImage = pCam->GetNextImage();
        if (pResultImage->IsIncomplete()){
            ostringstream imagestatus;
            imagestatus << pResultImage->GetImageStatus();
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " Image incomplete with image status " + imagestatus.str() + "\n");
            cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl ;
        }
        else {
            // Print image information
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " grabbed image 1 " +
                                                        ", width = " + std::to_string(pResultImage->GetWidth()) +
                                                        ", height = " + std::to_string(pResultImage->GetHeight()) + "\n");
            // Get timestamp
            ChunkData chunkData = pResultImage->GetChunkData();
            // Retrieve timestamp
            const int64_t timestamp = chunkData.GetTimestamp();
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "\tTimestamp: " + std::to_string(timestamp) + "\n");
            cout << "\tTimestamp: " << timestamp << endl;
            // Check if camera(s) is(are) synchronized to master camera
            // NOTE : This offset is not calcurated from image timestamp.
            // This value is latched when GevIEEE1588DataSetLatch is executed. Therefore, not so accurate and this is just for an information.
            CCommandPtr ptrGevIEEE1588DataSetLatch = pCam->GetNodeMap().GetNode("GevIEEE1588DataSetLatch");
            ptrGevIEEE1588DataSetLatch->Execute();
            CIntegerPtr ptrGevIEEE1588OffsetFromMasterLatched = pCam->GetNodeMap().GetNode("GevIEEE1588OffsetFromMasterLatched");
            const int64_t timeoffset = ptrGevIEEE1588OffsetFromMasterLatched->GetValue();
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "\tTime Offset: " + std::to_string(timeoffset) + "\n");
            cout << "\tTime Offset: " << timeoffset << endl;
            // Convert image to mono 8
            ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
//                ImagePtr convertedImage = pResultImage->Convert(PixelFormat_RGB16, HQ_LINEAR);
            // Create a unique filename and save image
            if(!this->creatSaveImgFolder(camerNum)){
                this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Image " + camerNum + "cannot creat floder\n");
            }
            ostringstream filename;
            filename << "./myImage/" << camerNum << "/";
            filename << "Camera-";
            filename <<camerNum;
            filename << "-" << timestamp << ".jpg";
            convertedImage->Save(filename.str().c_str());
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Image saved at " + filename.str() + "\n");
            cout << "Image saved at " << filename.str() << endl << endl;
        }
        pResultImage->Release();
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::creatSaveImgFolder(const std::string &camerNum){
    QString strWorkPath = QDir::currentPath();
    QString strSaveLogPath = strWorkPath + "/myImage/" + QString::fromStdString(camerNum);
    QDir saveLogDir(strSaveLogPath);
    if(!saveLogDir.exists()){
        if(!saveLogDir.mkpath(strSaveLogPath)){
            qDebug() << "ERRO: Canot creat dir" << strSaveLogPath;
            return false;
        }
    }
    return true;
}

bool MainWindow::setTimeStamp(const InterfacePtr & interfacePtr){
    CameraPtr pCam = NULL;
    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** SET TIMESTAMP ***");
    try {
        INodeMap &nodeMapInterface = interfacePtr->GetTLNodeMap();
        interfacePtr->UpdateCameras();
        // Display interface name
        CStringPtr ptrInterfaceDisplayName = nodeMapInterface.GetNode("InterfaceDisplayName");
        const gcstring interfaceDisplayName = ptrInterfaceDisplayName->GetValue();
        // Latch Timestamp for Camera 0
        pCam = actureCamList.GetBySerial(camSerialNumber[0]);
        CCommandPtr ptrTimestampLatch = pCam->GetNodeMap().GetNode("TimestampLatch");
        if (!IsAvailable(ptrTimestampLatch))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Camera " + camSerialNumber[0] + " Unable to retreive Time Stamp Latch (node retrieval). Aborting...\n");
            return false;
        }
        ptrTimestampLatch->Execute();
        CIntegerPtr ptrTimestampLatchValue = pCam->GetNodeMap().GetNode("TimestampLatchValue");
        if (!IsAvailable(ptrTimestampLatchValue) || !IsReadable(ptrTimestampLatchValue))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Camera " + camSerialNumber[0] + " Unable to read Time Stamp Latch Value (node retrieval). Aborting...\n");
            return false;
        }
        // Print out timestamp value
        int64_t timestampLatchValue = ptrTimestampLatchValue->GetValue();
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Camera " + camSerialNumber[0] + " Timestamp = " + std::to_string(timestampLatchValue) + "\n");
        // Issue Scheduled action command
        // 2 seconds used for this example
        const int64_t scheduleTimestampLatchValue = 2000000000;
        timestampLatchValue += scheduleTimestampLatchValue;
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Interface " + string(interfaceDisplayName) + " Update ActionTime = " + std::to_string(timestampLatchValue) + "\n");
        // Start PTP frame sync
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** Start PTP Frame sync ***");
        // Set action time
        CIntegerPtr ptrGevActionTime = nodeMapInterface.GetNode("GevActionTime");
        if (!IsAvailable(ptrGevActionTime) || !IsWritable(ptrGevActionTime))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Interface " + string(interfaceDisplayName) + " Unable to set Action Time (node retrieval). Aborting...\n");
            return false;
        }
        ptrGevActionTime->SetValue(timestampLatchValue);
        // Send action command
        unsigned int action_command_results_size = interfacePtr->GetCameras().GetSize();
        ActionCommandResult* action_command_results = new ActionCommandResult[action_command_results_size];
        // Send action command
        CCommandPtr ptrActionCommand = nodeMapInterface.GetNode("ActionCommand");
        if (!IsAvailable(ptrActionCommand))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Interface " + string(interfaceDisplayName) + " Unable to issue ActionCommand. Aborting...\n");
            return false;
        }
        ptrActionCommand->Execute();
/*
        interfacePtr->SendActionCommand(0, 1, 1, timestampLatchValue, &action_command_results_size, action_command_results);
        if (action_command_results_size != camList.GetSize())
        {
            cout << "Not every camera responded to the action command" << endl;
        }
        for (unsigned int j = 0; j < action_command_results_size; j++)
        {
            if (action_command_results[j].Status != ACTION_COMMAND_STATUS_OK)
            {
                cout << "A device did not successfully acknowledge the action command, with status " + to_string(static_cast<long long>(action_command_results[i].Status)) << endl;
            }
        }
*/
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

void MainWindow::myPrintDeviceInfermation()
{
    CameraPtr pCam = NULL;
    try {
        for(unsigned int i = 0; i < camList.GetSize(); i++)
        {
            pCam = camList.GetByIndex(i);
            INodeMap &nodeMapInterface = pCam->GetTLDeviceNodeMap();
            FeatureList_t features;
            const CCategoryPtr category = nodeMapInterface.GetNode("DeviceInformation");
            std::string str;
            if (IsAvailable(category) && IsReadable(category))
            {
                category->GetFeatures(features);
                for (FeatureList_t::const_iterator it = features.begin(); it != features.end(); ++it)
                {
                    const CNodePtr pfeatureNode = *it;
                    str = pfeatureNode->GetName() + " : ";
                    CValuePtr pValue = (CValuePtr)pfeatureNode;
                    if(IsReadable(pValue))
                    {
                        str += pValue->ToString() + "\n";
                        this->mylogging->MyPrintLogAll(MyLogging::INFO, str);
                        if(pfeatureNode->GetName() == "DeviceSerialNumber"){
                            if(std::find(camSerialNumber.begin(), camSerialNumber.end(), std::string(pValue->ToString())) == camSerialNumber.end()){
                                this->camSerialNumber.insert(camSerialNumber.end(), std::string(pValue->ToString()));
                            }
                        }
                    }
                    else{
                        str += "Node not readable \n";
                        this->mylogging->MyPrintLogAll(MyLogging::WAR, str);
                    }
                }
            }
            else
            {
                str = "Device control information not available.\n";
                this->mylogging->MyPrintLogAll(MyLogging::ERRO, str);
            }
            ui->textEdit->insertPlainText("\n");
        }

    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
}

bool MainWindow::MyConfigureInterface(const InterfacePtr & interfacePtr, const std::string &interfaceNum){
    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING ACTION CONTROL ***");
    try {
        INodeMap &nodeMapInterface = interfacePtr->GetTLNodeMap();
        CIntegerPtr ptrGevActionDeviceKey = nodeMapInterface.GetNode("GevActionDeviceKey");
        if (!IsAvailable(ptrGevActionDeviceKey) || !IsWritable(ptrGevActionDeviceKey))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, interfaceNum + " Unable to set Interface Action Device Key (node retrieval). Aborting...\n");
            return false;
        }
        // Set Action Device Key to 0
        ptrGevActionDeviceKey->SetValue(0);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, interfaceNum + " action device key is set 0\n");
        this->my_delay(1000);
        CIntegerPtr ptrGevActionGroupKey = nodeMapInterface.GetNode("GevActionGroupKey");
        if (!IsAvailable(ptrGevActionGroupKey) || !IsWritable(ptrGevActionGroupKey))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, " Unable to set Interface Action Group Key (node retrieval). Aborting...\n");
            return false;
        }
        // Set Action Group Key to 1
        ptrGevActionGroupKey->SetValue(1);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, interfaceNum + " action group key is set 1\n");
        this->my_delay(1000);
        CIntegerPtr ptrGevActionGroupMask = nodeMapInterface.GetNode("GevActionGroupMask");
        if (!IsAvailable(ptrGevActionGroupMask) || !IsWritable(ptrGevActionGroupMask))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, " Unable to set Interface Action Group Mask (node retrieval). Aborting...\n");
            return false;
        }
        // Set Action Group Mask to 1
        ptrGevActionGroupMask->SetValue(1);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, interfaceNum + " action group mask is set 1\n");
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyConfigureIEEE1588(const CameraPtr &pCam, const std::string &camerNum){
    try {
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING IEEE 1588 ***");
        CBooleanPtr ptrIEEE1588 = pCam->GetNodeMap().GetNode("GevIEEE1588");
        if (!IsAvailable(ptrIEEE1588) || !IsWritable(ptrIEEE1588))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to enable IEEE 1588 (node retrieval). Aborting...\n");
            return false;
        }
        // Enable IEEE 1588
        ptrIEEE1588->SetValue(true);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " IEEE 1588 is enabled.\n");
        // Requires delay for at least 6 seconds to enable 1588 settings
        // Add 10 second delay here
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Waiting for 10 seconds before sending action command. \n");
        this->my_delay(10000);
        // Check if IEEE 1588 settings is enabled for each camera
        CCommandPtr ptrGevIEEE1588DataSetLatch = pCam->GetNodeMap().GetNode("GevIEEE1588DataSetLatch");
        if (!IsAvailable(ptrGevIEEE1588DataSetLatch))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to execute IEEE1588 data set latch (node retrieval). Aborting...\n");
            return false;
        }
        ptrGevIEEE1588DataSetLatch->Execute();
        // Check if 1588 status is not in intialization
        CEnumerationPtr ptrGevIEEE1588StatusLatched = pCam->GetNodeMap().GetNode("GevIEEE1588StatusLatched");
        if (!IsAvailable(ptrGevIEEE1588StatusLatched) || !IsReadable(ptrGevIEEE1588StatusLatched))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to read IEEE1588 status (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrGevIEEE1588StatusLatchedInitializing = ptrGevIEEE1588StatusLatched->GetEntryByName("Initializing");
        if (!IsAvailable(ptrGevIEEE1588StatusLatchedInitializing) || !IsReadable(ptrGevIEEE1588StatusLatchedInitializing))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to get IEEE1588 status (enum entry retrieval). Aborting...\n");
            return false;
        }
        if(ptrGevIEEE1588StatusLatched->GetIntValue() == ptrGevIEEE1588StatusLatchedInitializing->GetValue())
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " is in Initializing mode. It can't send action command.\n");
            return false;
        }
        // Check if camera(s) is(are) synchronized to master camera
        // Verify if camera offset from master is larger than 1000ns which means camera(s) is(are) not synchronized
        CIntegerPtr ptrGevIEEE1588OffsetFromMasterLatched = pCam->GetNodeMap().GetNode("GevIEEE1588OffsetFromMasterLatched");
        if (!IsAvailable(ptrGevIEEE1588OffsetFromMasterLatched) || !IsReadable(ptrGevIEEE1588OffsetFromMasterLatched))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to read IEEE1588 offset (node retrieval). Aborting...\n");
            return false;
        }
        if (ptrGevIEEE1588OffsetFromMasterLatched->GetValue() > 1000)
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " has offset higher than 1000ns. Camera(s) is(are) not synchronized\n");
            return false;
        }
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyConfigureActionControl(const CameraPtr &pCam, const std::string &camerNum){
    try {
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING ACTION CONTROL ***");
        // Apply action group setting
        CIntegerPtr ptrActionDeviceKey = pCam->GetNodeMap().GetNode("ActionDeviceKey");
        if (!IsAvailable(ptrActionDeviceKey) || !IsWritable(ptrActionDeviceKey))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Action Device Key (node retrieval). Aborting...\n");
            return false;
        }
        // Set action device key to 0
        ptrActionDeviceKey->SetValue(0);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " action device key is set 0\n");
        CIntegerPtr ptrActionGroupKey = pCam->GetNodeMap().GetNode("ActionGroupKey");
        if (!IsAvailable(ptrActionGroupKey) || !IsWritable(ptrActionGroupKey))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Action Group Key (node retrieval). Aborting...\n");
            return false;
        }
        // Set action group key to 1
        ptrActionGroupKey->SetValue(1);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " action group key is set 1\n");
        CIntegerPtr ptrActionGroupMask = pCam->GetNodeMap().GetNode("ActionGroupMask");
        if (!IsAvailable(ptrActionGroupMask) || !IsWritable(ptrActionGroupMask))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to retreive Action Group Mask (node retrieval). Aborting...\n");
            return false;
        }
        // Set action group mask to 1
        ptrActionGroupMask->SetValue(1);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " action group mask is set 1\n");
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyConfigureOtherNodes(const CameraPtr &pCam, const std::string &camerNum){
    try {
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING OTHER NODES ***");
        // Frame rate setting
        // Turn on frame rate control
        CBooleanPtr ptrFrameRateEnable = pCam->GetNodeMap().GetNode("AcquisitionFrameRateEnable");
        if (!IsAvailable(ptrFrameRateEnable) || !IsWritable(ptrFrameRateEnable))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to enable Acquisition Frame Rate (node retrieval). Aborting...\n");
            return false;
        }
        // Enable  Acquisition Frame Rate Enable
        ptrFrameRateEnable->SetValue(true);
        CFloatPtr ptrFrameRate = pCam->GetNodeMap().GetNode("AcquisitionFrameRate");
        if (!IsAvailable(ptrFrameRate) || !IsWritable(ptrFrameRate))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Acquisition Frame Rate (node retrieval). Aborting...\n");
            return false;
        }
        // Set 10fps for this example
        // const float frameRate = 10.0f;
        const float frameRate = 10.0f;
        ptrFrameRate->SetValue(frameRate);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " Frame rate is set to " + std::to_string(frameRate) + "\n");
        // Turn off exposure auto.
        // Otherwise, it can miss action command because exposure is too long
        CEnumerationPtr ptrExposureAuto = pCam->GetNodeMap().GetNode("ExposureAuto");
        if (!IsAvailable(ptrExposureAuto) || !IsWritable(ptrExposureAuto))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to disable Exposure Auto (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrExposureAutoOff = ptrExposureAuto->GetEntryByName("Off");
        if (!IsAvailable(ptrExposureAutoOff) || !IsReadable(ptrExposureAutoOff))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Exposure Auto (node retrieval). Aborting...\n");
            return false;
        }
        // Turn off Exposure Auto
        ptrExposureAuto->SetIntValue(ptrExposureAutoOff->GetValue());
        CFloatPtr ptrExposureTime = pCam->GetNodeMap().GetNode("ExposureTime");
        if (IsAvailable(ptrExposureTime) && IsWritable(ptrExposureTime))
        {
            // Set exposure time to 1000 for this example
            //const float exposureTime = 1000.0f;
            const float exposureTime = 30000.0f;
            ptrExposureTime->SetValue(exposureTime);
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " Exposure time is set to " + std::to_string(exposureTime) + "\n");
        }
        else
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Exposure Time (node retrieval). Aborting...\n");
        }
        // Turn off gain auto. 一种电路能自动地调整视讯信号的电子放大,来补偿因照明亮度位阶的改变
        CEnumerationPtr ptrGainAuto = pCam->GetNodeMap().GetNode("GainAuto");
        if (!IsAvailable(ptrGainAuto) || !IsWritable(ptrGainAuto))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to disable Gain Auto (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrGainAutoOff = ptrGainAuto->GetEntryByName("Off");
        if (!IsAvailable(ptrGainAutoOff) || !IsReadable(ptrGainAutoOff))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Gain Auto (node retrieval). Aborting...\n");
            return false;
        }
        // Turn off Gain Auto
        ptrGainAuto->SetIntValue(ptrGainAutoOff->GetValue());
        /* Turn off balance white auto.
        // Turn off balance white auto.
        CEnumerationPtr ptrBalanceWhiteAuto = pCam->GetNodeMap().GetNode("BalanceWhiteAuto");
        if (!IsAvailable(ptrBalanceWhiteAuto) || !IsWritable(ptrBalanceWhiteAuto))
        {
            cout << "Camera " << i << " Unable to disable Balance White Auto (node retrieval). Aborting..." << endl;
            return -1;
        }
        CEnumEntryPtr ptrBalanceWhiteAutoOff = ptrBalanceWhiteAuto->GetEntryByName("Off");
        if (!IsAvailable(ptrBalanceWhiteAutoOff) || !IsReadable(ptrBalanceWhiteAutoOff))
        {
            cout << "Camera " << i << " Unable to set Balance White Auto (enum entry retrieval). Aborting..." << endl;
            return -1;
        }
        // Turn off Gain Auto
        ptrBalanceWhiteAuto->SetIntValue(ptrBalanceWhiteAutoOff->GetValue());
        */
        // Apply acquisition timing, timestamp setting
        CEnumerationPtr ptrAcquisitionTiming = pCam->GetNodeMap().GetNode("AcquisitionFrameAcquisitionTiming");
        if (!IsAvailable(ptrAcquisitionTiming) || !IsWritable(ptrAcquisitionTiming))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Acquisition Frame Acquisition Timing (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrAcquisitionTimingRealtimeClock = ptrAcquisitionTiming->GetEntryByName("RealtimeClock");
        if (!IsAvailable(ptrAcquisitionTimingRealtimeClock) || !IsReadable(ptrAcquisitionTimingRealtimeClock))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set acquisition timing (enum entry retrieval). Aborting...\n");
            return false;
        }
        // Set Acquisition Frame Acquisition Timing to realtime clock
        ptrAcquisitionTiming->SetIntValue(ptrAcquisitionTimingRealtimeClock->GetValue());
        CEnumerationPtr ptrAcquisitionTimestamp = pCam->GetNodeMap().GetNode("AcquisitionImageTimestamp");
        if (!IsAvailable(ptrAcquisitionTimestamp) || !IsWritable(ptrAcquisitionTimestamp))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to retreive Acquisition Image Timestamp (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrAcquisitionTimestampStartOfExposure = ptrAcquisitionTimestamp->GetEntryByName("StartOfExposure");
        if (!IsAvailable(ptrAcquisitionTimestampStartOfExposure) || !IsReadable(ptrAcquisitionTimestampStartOfExposure))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Acquisition Image Timestamp (enum entry retrieval). Aborting...\n");
            return false;
        }
        // Set Acquisition Image Timestamp to start of exposure
        ptrAcquisitionTimestamp->SetIntValue(ptrAcquisitionTimestampStartOfExposure->GetValue());
        /* Additional configuration for ISS Yokohama demo Set 2x2 binning
        CIntegerPtr ptrBinningHorizontal = pCam->GetNodeMap().GetNode("BinningHorizontal");
        if (IsAvailable(ptrBinningHorizontal) && IsWritable(ptrBinningHorizontal))
        {
            // Set Binning Horizontal to 2 for this example
            const int binningHorizontal = 2;
            ptrBinningHorizontal->SetValue(binningHorizontal);
            cout << "Camera " << i << " Binning Horizontal is set to " << binningHorizontal << endl;
        }
        else
        {
            cout << "Camera " << i << " Unable to set Binning Horizontal (node retrieval). Aborting..." << endl;
        }
        CIntegerPtr ptrBinningVertical = pCam->GetNodeMap().GetNode("BinningVertical");
        if (IsAvailable(ptrBinningVertical) && IsWritable(ptrBinningVertical))
        {
            // Set Binning Vertical to 2 for this example
            const int binningVertical = 2;
            ptrBinningVertical->SetValue(binningVertical);
            cout << "Camera " << i << " Binning Vertical is set to " << binningVertical << endl;
        }
        else
        {
            cout << "Camera " << i << " Unable to set Binning Vertical (node retrieval). Aborting..." << endl;
        }
        */
        /* Set Stream Default Buffer Count
        CIntegerPtr ptrBuffer = pCam->GetTLStreamNodeMap().GetNode("StreamDefaultBufferCount");
        ptrBuffer->SetValue(200);
        cout << "Camera " << i << " Stream Default Buffer Count is set to " << ptrBuffer->GetValue() << endl;
        CIntegerPtr ptrStreamDefaultBufferCount = pCam->GetNodeMap().GetNode("StreamDefaultBufferCount");
        if (IsAvailable(ptrStreamDefaultBufferCount) && IsWritable(ptrStreamDefaultBufferCount))
        {
            // Set Stream Default Buffer Count to 200 for this example
            const int streamDevaultBufferCount = 200;
            ptrStreamDefaultBufferCount->SetValue(streamDevaultBufferCount);
            cout << "Camera " << i << " Stream Default Buffer Count is set to " << streamDevaultBufferCount << endl;
        }
        else
        {
            cout << "Camera " << i << " Unable to set Stream Default Buffer Count(node retrieval). Aborting..." << endl;
        }
*/
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyConfigureTrigger(const CameraPtr &pCam, const std::string &camerNum){
    try {
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING TRIGGER ***");
        // Trigger setting
        //
        // Ensure trigger mode off
        //
        // *** NOTES ***
        // The trigger must be disabled in order to configure whether the source is software or hardware.
        //
        CEnumerationPtr ptrTriggerMode = pCam->GetNodeMap().GetNode("TriggerMode");
        if (!IsAvailable(ptrTriggerMode) || !IsReadable(ptrTriggerMode))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to disable trigger mode (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
        if (!IsAvailable(ptrTriggerModeOff) || !IsReadable(ptrTriggerModeOff))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set trigger mode (enum entry retrieval). Aborting...\n");
            return false;
        }
        ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " Trigger mode disabled...\n");
        // Select trigger selector
        CEnumerationPtr ptrTriggerSelector = pCam->GetNodeMap().GetNode("TriggerSelector");
        if (!IsAvailable(ptrTriggerSelector) || !IsWritable(ptrTriggerSelector))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set trigger mode (enum entry retrieval). Aborting...\n");
            return false;
        }
        // Need to select AcquisitionStart for real time clock
        CEnumEntryPtr ptrTriggerSelectorAcquisitionStart = ptrTriggerSelector->GetEntryByName("AcquisitionStart");
        if (!IsAvailable(ptrTriggerSelectorAcquisitionStart) || !IsReadable(ptrTriggerSelectorAcquisitionStart))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set trigger selector (enum entry retrieval). Aborting...\n");
            return false;
        }
        ptrTriggerSelector->SetIntValue(ptrTriggerSelectorAcquisitionStart->GetValue());
        //
        // Select trigger source
        //
        // *** NOTES ***
        // The trigger source must be set to hardware or software while trigger mode is off.
        //
        CEnumerationPtr ptrTriggerSource = pCam->GetNodeMap().GetNode("TriggerSource");
        if (!IsAvailable(ptrTriggerSelector) || !IsWritable(ptrTriggerSelector))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to get trigger source (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrTriggerSourceAction0 = ptrTriggerSource->GetEntryByName("Action0");
        if (!IsAvailable(ptrTriggerSourceAction0) || !IsReadable(ptrTriggerSourceAction0))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set trigger selector (enum entry retrieval). Aborting...\n");
            return false;
        }
        ptrTriggerSource->SetIntValue(ptrTriggerSourceAction0->GetValue());
        //
        // Turn trigger mode on
        //
        // *** LATER ***
        // Once the appropriate trigger source has been set, turn trigger mode
        // on in order to retrieve images using the trigger.
        //
        CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
        if (!IsAvailable(ptrTriggerModeOn) || !IsReadable(ptrTriggerModeOn))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to disable trigger mode (enum entry retrieval). Aborting...\n");
            return false;
        }
        ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());
        // TODO: Blackfly and Flea3 GEV cameras need 1 second delay after trigger mode is turned on
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " Trigger mode turned back on...\n");
        // Set Strobe output
        CEnumerationPtr ptrLineSelector = pCam->GetNodeMap().GetNode("LineSelector");
        if (!IsAvailable(ptrLineSelector) || !IsWritable(ptrLineSelector))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Lineselector (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrLine2 = ptrLineSelector->GetEntryByName("Line2");
        if (!IsAvailable(ptrLine2) || !IsReadable(ptrLine2))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to get Line2 (node retrieval). Aborting...\n");
            return false;
        }
        ptrLineSelector->SetIntValue(ptrLine2->GetValue());
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Selected LineSelector is Line: " + string(ptrLineSelector->GetCurrentEntry()->GetSymbolic()) + "\n");
        // Line Mode
        CEnumerationPtr ptrLineMode = pCam->GetNodeMap().GetNode("LineMode");
        if (!IsAvailable(ptrLineMode) || !IsWritable(ptrLineMode))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to get Line Mode (node retrieval). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrOutput = ptrLineMode->GetEntryByName("Output");
        if (!IsAvailable(ptrOutput) || !IsReadable(ptrOutput))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to get Output (node retrieval). Aborting...\n");
            return false;
        }
        ptrLineMode->SetIntValue(ptrOutput->GetValue());
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Selected LineMode is Line: " + string(ptrLineMode->GetCurrentEntry()->GetSymbolic()) + "\n");
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyConfigureChunkData(const CameraPtr &pCam, const std::string &camerNum){
    try {
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING CHUNK DATA ***");
        //
        // Activate chunk mode
        //
        // *** NOTES ***
        // Once enabled, chunk data will be available at the end of the payload
        // of every image captured until it is disabled. Chunk data can also be
        // retrieved from the nodemap.
        //
        CBooleanPtr ptrChunkModeActive = pCam->GetNodeMap().GetNode("ChunkModeActive");
        if (!IsAvailable(ptrChunkModeActive) || !IsWritable(ptrChunkModeActive))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to chunk mode. Aborting...\n");
            return false;
        }
        ptrChunkModeActive->SetValue(true);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " Chunk mode activated...\n");
        CEnumerationPtr ptrChunkSelector = pCam->GetNodeMap().GetNode("ChunkSelector");
        if (!IsAvailable(ptrChunkSelector) || !IsWritable(ptrChunkSelector))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Chunk Selector is not writable\n");
            return false;
        }
        // Select Timestamp for Chunk data
        CEnumEntryPtr ptrChunkSelectorTimestamp = ptrChunkSelector->GetEntryByName("Timestamp");
        if (!IsAvailable(ptrChunkSelectorTimestamp) || !IsReadable(ptrChunkSelectorTimestamp))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " Unable to set Chunk Selector (node retrieval). Aborting...\n");
            return false;
        }
        ptrChunkSelector->SetIntValue(ptrChunkSelectorTimestamp->GetValue());
        // Retrieve corresponding boolean
        CBooleanPtr ptrChunkEnable = pCam->GetNodeMap().GetNode("ChunkEnable");
        // Enable the boolean, thus enabling the corresponding chunk data
        if (!IsAvailable(ptrChunkEnable))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " not available\n");
            return false;
        }
        else if (ptrChunkEnable->GetValue())
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " can getValue\n");
        }
        else if (IsWritable(ptrChunkEnable))
        {
            ptrChunkEnable->SetValue(true);
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " enabled\n");
        }
        else
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, camerNum + " not writable\n");
            return false;
        }
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyConfigurAcquisition(const CameraPtr &pCam, const std::string &camerNum){
    try {
        this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CONFIGURING Acquisition ***");
        // Set acquisition mode to continuous
        CEnumerationPtr ptrAcquisitionMode = pCam->GetNodeMap().GetNode("AcquisitionMode");
        if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Unable to set acquisition mode to continuous (node retrieval; camera " + camerNum + "). Aborting...\n");
            return false;
        }
        CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Unable to set acquisition mode to continuous (entry 'continuous' retrieval " + camerNum + "). Aborting...\n");
            return false;
        }
        const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " acquisition mode set to continuous...\n");
        // Packet size, Packet delay setting
        CIntegerPtr ptrPacketSize = pCam->GetNodeMap().GetNode("GevSCPSPacketSize");
        if (!IsAvailable(ptrPacketSize) || !IsWritable(ptrPacketSize))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Unable to set packet size (node retrieval; camera " + camerNum + "). Aborting...\n");
            return false;
        }
        // Set packet size to maximum value
        const int64_t packetSize = 9000;
//			const int64_t packetSize = 1400;	// Do not use Jumbo packet due to switch issue
        ptrPacketSize->SetValue(packetSize);
/*
        CIntegerPtr ptrPacketDelay = pCam->GetNodeMap().GetNode("GevSCPD");
        if (!IsAvailable(ptrPacketDelay) || !IsWritable(ptrPacketDelay))
        {
            cout << "Unable to set packet delay (node retrieval; camera " << i << "). Aborting..." << endl << endl;
            return -1;
        }
        // Set packet delay 9000 by using formula packet delay x 8
        // for BFS only, there is 8 bit internal tick
        // for Gen 2 cameras , just use packet delay 9000
        const int64_t packetDelay = 72000;
        ptrPacketDelay->SetValue(packetDelay);
*/
        // Device Link Throughput Limit setting
        CIntegerPtr ptrDeviceLinkThroughputLimit = pCam->GetNodeMap().GetNode("DeviceLinkThroughputLimit");
        if (!IsAvailable(ptrDeviceLinkThroughputLimit) || !IsWritable(ptrDeviceLinkThroughputLimit))
        {
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::WAR, "Unable to set device link throughput limit (node retrieval; camera " + camerNum + "). Aborting...\n");
            return false;
        }
        /* Set throughput close to the maximum limit 125000000
//            const int64_t deviceLinkThroughputLimit = 100000000;
        const int64_t deviceLinkThroughputLimit = 125000000;      // for 6 cameras sharing 120M
        ptrDeviceLinkThroughputLimit->SetValue(deviceLinkThroughputLimit);
        cout << "Camera " << i << " Device Link Thoughput Limit set to " << ptrDeviceLinkThroughputLimit->GetValue() << endl;
*/
        // Begin acquiring images
        pCam->BeginAcquisition();
        this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, camerNum + " begin acquisition...\n");
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

bool MainWindow::MyDeinitizeCamera(){
    CameraPtr pCam = NULL;
    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** DEINITIZE CAMERA ***");
    try {
        for (auto camNum : camSerialNumber){
            pCam = actureCamList.GetBySerial(camNum);
            // Deinitialize camera
            pCam->DeInit();
            this->mylogging->MyPrintLogAll<std::string>(MyLogging::INFO, "Camera " + camNum + " Deinitize...\n");
        }
        // Clear camera list before releasing system
        actureCamList.Clear();
        interfaceList.Clear();
        // Release system
        system->ReleaseInstance();
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
    return true;
}

void MainWindow::on_pushButtonSaveLog_clicked()
{
    //获取保存的文件名
    QString strFileSave = QFileDialog::getSaveFileName(
                this,
                tr("保存文件"),
                tr("."),
                tr("Text Files(*.txt)")
                );
    if (strFileSave.isEmpty()){
        return;
    }
    QFile fileOut (strFileSave);
    if ( ! fileOut.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this, tr("保存文件"), tr("打开保存文件失败:") + fileOut.errorString());
    }
    QTextStream tsOut (& fileOut);
    tsOut << ui->textEdit->toPlainText();
}

void MainWindow::on_pushButtonCatchImg_clicked()
{
    CameraPtr pCam = NULL;
    InterfacePtr interfacePtr = NULL;
    this->mylogging->MyPrintLogAll<const char*>(MyLogging::ANNO, "*** CATCH SINGLE IMAGE ***");
    try {
        for (unsigned int i = 0; i < actureCamList.GetSize(); i++){
             pCam = actureCamList.GetByIndex(i);
             CStringPtr ptrStringSerial = pCam->GetTLDeviceNodeMap().GetNode("DeviceSerialNumber");
             this->catchSingalImg(pCam, string(ptrStringSerial->GetValue()));
            }
//        for (auto camNum : camSerialNumber){
//            pCam = camList.GetBySerial(camNum);
//            this->catchSingalImg(pCam, camNum);
//        }
    } catch (Spinnaker::Exception &e) {
        this->mylogging->MyPrintLogAll<QString>(MyLogging::ERRO, QString("%1 \r\n\r\n").arg(QString(e.what())));
    }
}
