#ifndef TFPREADTHREAD_H
#define TFPREADTHREAD_H

#include <QThread>
#include <QObject>
#include <common.h>
#include <QQueue>
#include <QByteArray>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <threadsafequeue.h>

class TFPReadThread : public QThread
{
    Q_OBJECT
public:
//    explicit TFPReadThread(QQueue<QByteArray>& send_quque, QObject *parent = nullptr);
    explicit TFPReadThread(ThreadSafeQueue<QByteArray>& send_quque, QObject *parent = nullptr);
    ~TFPReadThread();


    int getTfp_win() const;
    void setTfp_win(int newTfp_win);

public slots:
    void slot_set_tfp_window(int win);


protected:
    void run() override;


private:
    inline void tfp_enable(void);
    inline void tfp_disable(void);
    inline void tfp_set_recon_window(int value);

private:
//    QQueue<QByteArray>& udp_send_quque;
    ThreadSafeQueue<QByteArray>& udp_send_quque;
    const int TFP_DATA_ADDR = 0x10000000;
    const int TFP_SET_ADDR =  0x80000000;
    char* tfp_data = nullptr;
    char* tfp_set = nullptr;
    char* tfp_ctrl = nullptr;
    char* tfp_set_win = nullptr;
    int mem_fd = -1;
    int tfp_win = 20;

};

#endif // TFPREADTHREAD_H
