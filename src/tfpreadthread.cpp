#include "tfpreadthread.h"
#include <QDebug>
#include <sched.h>

TFPReadThread::TFPReadThread(ThreadSafeQueue<QByteArray>& send_quque, QObject *parent)
    : QThread{parent},
      udp_send_quque(send_quque)
{
    this->mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
    if(this->mem_fd <0) {
        qFatal("can't open /dev/mem");
    }

    this->tfp_data = (char *)mmap(NULL, TFP_IMG_SIZE + 4, PROT_READ, MAP_SHARED, this->mem_fd, this->TFP_DATA_ADDR);
    if(this->tfp_data == nullptr) {
        qDebug() << "tfp_data mmap failed";
    }
    this->tfp_set = (char *)mmap(NULL, 16, PROT_READ | PROT_WRITE, MAP_SHARED, this->mem_fd, this->TFP_SET_ADDR);
    if(this->tfp_set == nullptr) {
        qFatal("tfp_addr is incorrect");
    }

    this->tfp_ctrl = this->tfp_set;
    this->tfp_set_win = this->tfp_set + 1*sizeof(int);
//    qDebug("tfp_data:%#p,tfp_set:%#p,tfp_ctrl:%#p, tfp_set_win:%#p", (void*)this->tfp_data,
//           (void*)this->tfp_set, (void*)this->tfp_ctrl, (void*)this->tfp_set_win);
}

TFPReadThread::~TFPReadThread()
{
    this->tfp_disable();
    if(this->mem_fd != -1) {
        munmap(this->tfp_set, 16);
        munmap(this->tfp_data, TFP_IMG_HEIGHT*TFP_IMG_WDITH + 4);
        close(this->mem_fd);
    }
}

void TFPReadThread::slot_set_tfp_window(int win)
{
    this->tfp_set_recon_window(win);
}

void TFPReadThread::run()
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(0, &cpuSet);
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);

    volatile uint32_t* frame_num = (volatile  uint32_t*)((char*)this->tfp_data + TFP_IMG_SIZE);

    volatile uint32_t last_frame_num = *frame_num;
    volatile uint32_t current_frame_num = *frame_num;
    this->tfp_enable();
    QThread::msleep(1);
    qDebug() << "tfp_win:" << this->tfp_win;
    this->tfp_set_recon_window(this->tfp_win);
    while(!this->isInterruptionRequested()) {
        current_frame_num = *frame_num;
//        qDebug("frame_num:%ld,last_frame_num:%ld", current_frame_num, last_frame_num);
        if(last_frame_num != current_frame_num) {
            this->udp_send_quque.enqueue(QByteArray(this->tfp_data, TFP_IMG_SIZE + 4));
            last_frame_num = current_frame_num;
        }
    }
}

void TFPReadThread::tfp_enable()
{
    if(this->tfp_ctrl == nullptr) return;

    *((uint32_t*)this->tfp_ctrl) = 0xABCDFFFF;
}

void TFPReadThread::tfp_disable()
{
    if(this->tfp_ctrl == nullptr) return;

    *((uint32_t*)this->tfp_ctrl) = 0xABCD0000;

}

void TFPReadThread::tfp_set_recon_window(int value)
{
    if(this->tfp_set_win == nullptr) return;

    *((uint32_t*)this->tfp_set_win) = (0x12340000 | value);

}

int TFPReadThread::getTfp_win() const
{
    return tfp_win;
}

void TFPReadThread::setTfp_win(int newTfp_win)
{
    tfp_win = newTfp_win;
}
