#include "cmdctrl.h"
#include <iostream>
#include <QKeyEvent>

CmdCtrl::CmdCtrl( int argc, char* argv[], QObject *parent)
    : QObject{parent}
{
    this->udp_send_queue = new ThreadSafeQueue<QByteArray>();
    this->tfp_read_thread = new TFPReadThread(*this->udp_send_queue, this);
    this->udp_send_thread = new SocketWorkerThread(*this->udp_send_queue, this);

    connect(this, &CmdCtrl::signal_set_new_udp_connect, this->udp_send_thread, &SocketWorkerThread::slot_connect_udp);
    connect(this, &CmdCtrl::signal_set_new_tcp_connect, this->udp_send_thread, &SocketWorkerThread::slot_connect_tcp);

    this->tfp_read_thread->setTfp_win(this->tfp_win);

    this->cmd_handler(argc, argv);
}

CmdCtrl::~CmdCtrl()
{
    this->tfp_read_thread->requestInterruption();
    this->udp_send_thread->requestInterruption();
    this->tfp_read_thread->wait(100);
    this->udp_send_thread->wait(100);
}

void CmdCtrl::cmd_handler(int argc, char *argv[])
{
    if(argc < 7) {
        std::cerr << "Usage: " << argv[0] << " -t <trans type USB UDP> -i <target_ip_addr> -p <target_ip_port> -w <tfp_windows_size>" << std::endl;
        qFatal("The number of arguments is incorrect.");
    }

    // 解析参数
    for (int i = 1; i < argc; i += 2) {
        QString option = argv[i];
        if(option == "-t" || option == "-T") {
            QString set_trans_type = argv[i + 1];
            set_trans_type = set_trans_type.toUpper();
            if(set_trans_type == "UDP") {
                this->trans_type = UDP;
            } else if(set_trans_type == "TCP") {
                this->trans_type =  TCP;
            } else if(set_trans_type == "USB") {
                this->trans_type =  USB;
            }
        }

        if(option == "-i" || option == "-I") {
            if(this->trans_type == USB) {
                qFatal("usb not input ipaddr");
            }
            this->ip_addr = argv[i + 1];
            QHostAddress address;
            if (!address.setAddress(this->ip_addr)) {
                qFatal("ip_addr is incorrect");
            }
        }

        if(option == "-p" || option == "-P") {
            if(this->trans_type == USB) {
                qFatal("usb not input network port");
            }
            QString port = argv[i + 1];
            bool is_ok = false;
            this->port = port.toInt(&is_ok);
            if(this->port <= 0 || this->port > 65535 || !is_ok) {
                qFatal("network port is incorrect");
            }
        }

        if(option == "-w" || option == "-W") {
            QString tfp_window = argv[i + 1];
            bool is_ok = false;
            this->tfp_win = tfp_window.toInt(&is_ok);
            if(this->tfp_win <= 0 || this->tfp_win > 1000 || !is_ok) {
                qFatal("tfp win is incorrect");
            }
        }

    }

    switch(this->trans_type) {
    case UDP:
        emit this->signal_set_new_udp_connect(this->ip_addr, this->port);
        this->tfp_read_thread->setTfp_win(this->tfp_win);
        this->tfp_read_thread->start();
    break;

    case TCP:
        emit this->signal_set_new_tcp_connect(this->ip_addr, this->port);
        this->tfp_read_thread->start();
        break;

    case USB:
        break;
    }
}
