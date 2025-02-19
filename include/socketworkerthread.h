#ifndef SOCKETWORKERTHREAD_H
#define SOCKETWORKERTHREAD_H

#include <QThread>
#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include <common.h>
#include <threadsafequeue.h>
#include <sched.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class SocketWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit SocketWorkerThread(ThreadSafeQueue<QByteArray> &send_queue,
                           QObject *parent = nullptr);
    ~SocketWorkerThread();

protected:
    void run() override;

public slots:
    void slot_connect_udp(QString ip_addr, int port);
    void slot_connect_tcp(QString ip_addr, int port);

private:
    void send_byte_data(QByteArray data);
    void tcp_send_byte_data(QByteArray data);

private:
    enum SocketClass{
        TCP = 0,
        UDP = 1
    };

    QUdpSocket  *m_udp_socket = nullptr;

    QString target_addr_string;
    QHostAddress m_udp_target_ipaddr;
    int m_udp_target_port = -1;
    int m_tcp_target_port = -1;
    int sockfd = -1;

    char send_buf[TFP_IMG_SIZE / TFP_ONE_IMG_SEND_PART + FRAME_NUM_SIZE + FRAME_LENGTH_SIZE + FRAME_SERIAL_SIZE + FRAME_HEADER_SIZE + FRAME_END_SIZE];
    ThreadSafeQueue<QByteArray>& send_quque;
    SocketClass socket_class = UDP;
    struct sockaddr_in servaddr;
    bool is_tcp_client_connect = false;

};

#endif // SOCKETWORKERTHREAD_H
