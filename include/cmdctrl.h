#ifndef CMDCTRL_H
#define CMDCTRL_H

#include <QObject>
#include <QString>
#include <QQueue>
#include <QByteArray>
#include <tfpreadthread.h>
#include <socketworkerthread.h>
#include <threadsafequeue.h>
#include <QKeyEvent>

class CmdCtrl : public QObject
{
    Q_OBJECT
public:
    explicit CmdCtrl(int argc, char* argv[], QObject *parent = nullptr);
    ~CmdCtrl();

private:
    void cmd_handler(int argc, char* argv[]);

signals:
    void signal_set_new_udp_connect(QString ip_addr, int port);
    void signal_set_new_tcp_connect(QString ip_addr, int port);
    void signal_set_tfp_windows(int win);

private:
    enum TRANS_TYPE {
        UDP = 0,
        TCP,
        USB
    };

    ThreadSafeQueue<QByteArray> *udp_send_queue;
    SocketWorkerThread * udp_send_thread;
    TFPReadThread *tfp_read_thread;

    QString ip_addr;
    int port = -1;
    int tfp_win = 20;

    TRANS_TYPE trans_type = UDP;

};

#endif // CMDCTRL_H
