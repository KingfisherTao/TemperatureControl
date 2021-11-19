#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#include "NetAssistant/mserver.h"
#include "NetAssistant/msocket.h"

#define CONST_TARGET_IP "192.168.1.200"
#define CONST_TARGET_PORT 2000
#define AppName "温控控制程序"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void ClinetReadData();
    void ClientReadError(QAbstractSocket::SocketError);

private slots:
    void on_Btn_Bind_clicked();
    void on_Btn_Start_clicked();
    void about();
    void sendQuery();

private:
    void initUI();
    void initConnect();
    void setLEValRed(QLineEdit* le, QString num);
    void setLEValGreen(QLineEdit* le, QString num);
    void showStatusMessage(const QString &message);
    void updateInfo(QStringList info1, QStringList info2);
    bool sendTcpSocket(QString sendData);

    QTimer *m_timer;

    QMap<QString, uint> m_FormatAnalysis;
    QMap<QString, QLineEdit*> m_LEMap;

    Ui::MainWindow *ui;
    QLabel *m_status = nullptr;         // 窗口状态栏
    QTcpSocket* m_tcpSocket;
};
#endif // MAINWINDOW_H
