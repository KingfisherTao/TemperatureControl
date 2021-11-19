#include <QMessageBox>
#include "myhelper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_status(new QLabel)
{
    ui->setupUi(this);

    // 初始化 socket 指针
    m_tcpSocket=nullptr;

    initUI();
    initConnect();
}

void MainWindow::initUI()
{
    setWindowTitle(AppName);
    ui->statusbar->addWidget(m_status);
    setWindowFlags(Qt::WindowCloseButtonHint);
    setFixedSize(1046, 616);
    ui->Lab_IP->setText(QString("温控 IP:    %1:%2").arg(CONST_TARGET_IP).arg(CONST_TARGET_PORT));
    ui->Btn_Start->setEnabled(false);

    m_FormatAnalysis.insert("1|0", 10);     m_LEMap.insert("1|0", ui->LE_FeedLiquidT_1);
    m_FormatAnalysis.insert("3|2", 10);    m_LEMap.insert("3|2", ui->LE_FeedLiquidT_2);
    m_FormatAnalysis.insert("5|4", 10);   m_LEMap.insert("5|4", ui->LE_AckflowT_1);
    m_FormatAnalysis.insert("7|6", 10);   m_LEMap.insert("7|6", ui->LE_AckflowT_2);
    m_FormatAnalysis.insert("9|8", 10);   m_LEMap.insert("9|8", ui->LE_EnvTemp);
    m_FormatAnalysis.insert("11|10", 100);  m_LEMap.insert("11|10", ui->LE_FeedLiquidP_1);
    m_FormatAnalysis.insert("13|12", 100);  m_LEMap.insert("13|12", ui->LE_FeedLiquidP_2);
    m_FormatAnalysis.insert("15|14", 100);  m_LEMap.insert("15|14", ui->LE_FilterP_1);
    m_FormatAnalysis.insert("17|16", 100);  m_LEMap.insert("17|16", ui->LE_FilterP_2);
    m_FormatAnalysis.insert("19|18", 10);   m_LEMap.insert("19|18", ui->LE_Ackflow_1);
    m_FormatAnalysis.insert("21|20", 10);   m_LEMap.insert("21|20", ui->LE_Ackflow_2);
    m_FormatAnalysis.insert("23|22", 10);   m_LEMap.insert("23|22", ui->LE_Coolant);
    m_FormatAnalysis.insert("25|24", 100);   m_LEMap.insert("25|24", ui->LE_FreezingP);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::sendQuery);
}

void MainWindow::initConnect()
{
    connect(ui->QA_About, &QAction::triggered, this, &MainWindow::about);
}

MainWindow::~MainWindow()
{
    if(m_tcpSocket!=nullptr)
    {
        m_tcpSocket->abort();
        delete m_tcpSocket;
    }
    delete ui;
}

void MainWindow::on_Btn_Start_clicked()
{
    if (m_tcpSocket){
        if (ui->Btn_Start->text() == "开机")
        {
            sendTcpSocket("7E 7E 18 00 01 10 C1 0C 00 00 20 CC 01 00 00 00 01 00 01 00 00 00 0D 0A");
            ui->Btn_Start->setText("关机");
        }
        else
        {
            sendTcpSocket("7E 7E 18 00 01 10 C1 0C 00 00 06 CC 01 00 00 01 01 00 01 00 00 00 0D 0A");
            ui->Btn_Start->setText("开机");
        }
    }
    else
    {
        QMessageBox::information(this,"提示","请先连接设备");
    }
}

void MainWindow::sendQuery()
{
    sendTcpSocket("7E 7E 18 00 01 10 C1 0C 00 00 04 CC 01 00 00 00 01 00 01 00 00 00 0D 0A");
}

// tcp客户端连接/断开
void MainWindow::on_Btn_Bind_clicked()
{
    if(m_tcpSocket == nullptr)
    {
        QHostAddress ip(CONST_TARGET_IP);
        ip.toIPv4Address();
        quint16 port = CONST_TARGET_PORT;

        m_tcpSocket=new QTcpSocket(this);
        m_tcpSocket->connectToHost(ip,port);

        connect(m_tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(ClientReadError(QAbstractSocket::SocketError)));
        // 等待连接
        if (m_tcpSocket->waitForConnected(1000))
        {
            connect(m_tcpSocket,SIGNAL(readyRead()),this,SLOT(ClinetReadData()));

            //更新UI
            ui->Btn_Bind->setText("断开");
            ui->Btn_Bind->setIcon(QIcon(":/Resource/Img/stop36x36.png"));

            showStatusMessage(QString("<font color=forestgreen>连接至 %1 : %2</font>").arg(CONST_TARGET_IP).arg(CONST_TARGET_PORT));

            ui->Btn_Start->setEnabled(true);
            m_timer->start();
        }
        // 连接失败
        else
        {
            m_tcpSocket->disconnect();
            m_tcpSocket->deleteLater();
            m_tcpSocket=nullptr;

            showStatusMessage("<font color=red>连接设备失败</font>");

            ui->Btn_Start->setEnabled(false);
            m_timer->stop();
        }
    }
    else
    {
        m_tcpSocket->disconnect();  //断开信号槽
        m_tcpSocket->abort();       //终止
        m_tcpSocket->deleteLater(); //释放
        m_tcpSocket=nullptr;
        // 更新UI
        ui->Btn_Bind->setText("连接");
        ui->Btn_Bind->setIcon(QIcon(":/Resource/Img/start36x36.png"));
        showStatusMessage("<font color=red>关闭连接</font>");

        ui->Btn_Start->setEnabled(false);
        m_timer->stop();
    }
}

// tcp客户端发生错误
void MainWindow::ClientReadError(QAbstractSocket::SocketError)
{
    QString err=QString("发生错误:%1").arg(m_tcpSocket->errorString());
    qDebug() << err;
    // 断开所有信号
    m_tcpSocket->disconnect();
    // 终止socket连接
    m_tcpSocket->abort();
    // 释放
    m_tcpSocket->deleteLater();
    m_tcpSocket=nullptr;
    // 更新UI
    ui->Btn_Bind->setText("连接");
    ui->Btn_Bind->setIcon(QIcon(":/Resource/Img/stop36x36.png"));

    showStatusMessage(QString("<font color=red> %1 </font>").arg(err));
}

// tcp客户端读取数据
void MainWindow::ClinetReadData()
{
    QByteArray ba = m_tcpSocket->readAll();
    QString data;
    // hex
    myHelper::ByteToHexString(data,ba);

    // 显示
//    qDebug() << QString("[%1|%2]: %3")
//                .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
//                .arg(QString(CONST_TARGET_IP) + " : " + QString(CONST_TARGET_PORT))
//                .arg(data);

    // 解析查询状态
    if (ba.length() == 59)
    {
        QStringList _list1, _list2;
        QByteArray _temp1,_temp2;

        _temp1 = ba.mid(20, 5);
        _temp2 = ba.mid(27, 26);

        for (int i = 0; i < _temp1.length(); i++)
        {
            _list1.append(QString("%1").
                          arg(QByteArray::number(static_cast<unsigned char>((_temp1.at(i))), 16).
                              toUpper().toULongLong(nullptr, 16), 8, 2, QChar('0')));
        }

        for (int i = 0; i < _temp2.length(); i++)
        {
            unsigned char n =static_cast<unsigned char>((_temp2.at(i)));
            _list2.append(QByteArray::number(n, 16).toUpper());
        }

        updateInfo(_list1, _list2);
    }
}

void MainWindow::updateInfo(QStringList info1, QStringList info2)
{
    // 刷新液冷源工作状态
    QString workState = myHelper::ReversalStr(info1[0]);
    //qDebug() << "液冷源工作状态 : "<< info1[0] << " = " << workState;
    setLEValGreen(ui->LE_SW1, QString(workState[0]));
    setLEValGreen(ui->LE_SW2, QString(workState[1]));
    setLEValGreen(ui->LE_SW3, QString(workState[2]));
    setLEValGreen(ui->LE_SW4, QString(workState[3]));
    setLEValGreen(ui->LE_SW5, QString(workState[4]));
    setLEValGreen(ui->LE_SW6, QString(workState[5]));
    setLEValGreen(ui->LE_SW7, QString(workState[6]));

    // 刷新液冷源故障状态
    QString errorState1 = myHelper::ReversalStr(info1[1]);
    QString errorState2 = myHelper::ReversalStr(info1[2]);
    //qDebug() << "液冷源故障状态1 :" << info1[1] << errorState1;
    //qDebug() << "液冷源故障状态2 :" << info1[2] << errorState2;
    setLEValRed(ui->LE_Error1, QString(errorState1[0]));
    setLEValRed(ui->LE_Error2, QString(errorState1[1]));
    setLEValRed(ui->LE_Error3, QString(errorState1[2]));
    setLEValRed(ui->LE_Error4, QString(errorState1[3]));
    setLEValRed(ui->LE_Error5, QString(errorState1[4]));
    setLEValRed(ui->LE_Error6, QString(errorState1[5]));
    setLEValRed(ui->LE_Error7, QString(errorState1[6]));
    setLEValRed(ui->LE_Error8, QString(errorState1[7]));

    setLEValRed(ui->LE_Error9, QString(errorState2[0]));
    setLEValRed(ui->LE_Error10, QString(errorState2[1]));
    setLEValRed(ui->LE_Error11, QString(errorState2[2]));
    setLEValRed(ui->LE_Error12, QString(errorState2[3]));
    setLEValRed(ui->LE_Error13, QString(errorState2[4]));
    setLEValRed(ui->LE_Error14, QString(errorState2[5]));
    setLEValRed(ui->LE_Error15, QString(errorState2[6]));
    setLEValRed(ui->LE_Error16, QString(errorState2[7]));

    // 刷新报警状态
    QString altertState1 = myHelper::ReversalStr(info1[3]);
    QString altertState2 = myHelper::ReversalStr(info1[4]);

    //qDebug() << "报警状态1 :" << info1[3] << altertState1;
    //qDebug() << "报警状态2 :" << info1[4] << altertState2;

    setLEValRed(ui->LE_Altert1, QString(altertState1[0]));
    setLEValRed(ui->LE_Altert2, QString(altertState1[1]));
    setLEValRed(ui->LE_Altert3, QString(altertState1[2]));
    setLEValRed(ui->LE_Altert4, QString(altertState1[3]));
    setLEValRed(ui->LE_Altert5, QString(altertState1[4]));
    setLEValRed(ui->LE_Altert6, QString(altertState1[5]));
    setLEValRed(ui->LE_Altert7, QString(altertState1[6]));
    setLEValRed(ui->LE_Altert8, QString(altertState1[7]));

    setLEValRed(ui->LE_Altert9, QString(altertState2[0]));
    setLEValRed(ui->LE_Altert10, QString(altertState2[1]));
    setLEValRed(ui->LE_Altert11, QString(altertState2[2]));
    setLEValRed(ui->LE_Altert12, QString(altertState2[3]));
    setLEValRed(ui->LE_Altert13, QString(altertState2[4]));
    setLEValRed(ui->LE_Altert14, QString(altertState2[5]));

    QStringList _temp;
    // 刷新工作状态
    auto _iter = m_FormatAnalysis.begin();
    while (_iter != m_FormatAnalysis.end())
    {
        _temp = _iter.key().split("|");
        QString str = info2[_temp[0].toInt()] + info2[_temp[1].toInt()];
        m_LEMap[_iter.key()]->setText(QString::number(static_cast<float>(str.toInt(nullptr,16)) / _iter.value(), 'f', 2));
        _iter++;
    }
}

void MainWindow::setLEValRed(QLineEdit* le, QString num)
{
    le->setText(num);
    uint val = num.toUInt(nullptr, 10);
    if (val == 0)
        le->setStyleSheet("background-color:rgba(255,255,255,255)");
    else {
        le->setStyleSheet("background-color:rgba(255,0,0,255)");
    }
}

void MainWindow::setLEValGreen(QLineEdit* le, QString num)
{
    le->setText(num);
    uint val = num.toUInt(nullptr, 10);
    if (val == 0)
        le->setStyleSheet("background-color:rgba(255,255,255,255)");
    else {
        le->setStyleSheet("background-color:rgba(0,255,0,255)");
    }
}

bool MainWindow::sendTcpSocket(QString sendData)
{
    QByteArray byteArray;
    // hex字符串转字节
    if(!myHelper::HexStringToByte(sendData,byteArray))
    {
        QMessageBox::information(this,"提示","输入的十六进制字符串有误，请重新输入");
        return false;
    }
    if (m_tcpSocket)
    {
        m_tcpSocket->write(byteArray);
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}


void MainWindow::about()
{
    QMessageBox::about(this, "关于",
                       "本软件是温控程控小程序。\n"
                       "本软件需要网线连接，否则无法访问硬件\n"
                       "                                    本软件的最终解释权归陶晶所有");
}
