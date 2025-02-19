#include "socketworkerthread.h"
#include <QtMath>
#include "common.h"
#include <QCoreApplication>


SocketWorkerThread::SocketWorkerThread(ThreadSafeQueue<QByteArray> &send_queue, QObject *parent)
    :QThread{parent},
    send_quque(send_queue)
{
}

SocketWorkerThread::~SocketWorkerThread()
{
    if(this->sockfd != -1) {
        close(this->sockfd);
    }
}

void SocketWorkerThread::run()
{
//    cpu_set_t cpuSet;
//    CPU_ZERO(&cpuSet);
//    CPU_SET(1, &cpuSet);
//    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);


    this->is_tcp_client_connect = false;

    if(this->m_udp_socket != nullptr) {
        delete this->m_udp_socket;
        this->m_udp_socket = nullptr;
    }


    if(this->socket_class == UDP) {
        this->m_udp_socket = new QUdpSocket();
        this->m_udp_target_ipaddr = QHostAddress(this->target_addr_string);
        this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (this->sockfd < 0) {
            return ;
        }
        memset(&this->servaddr, 0, sizeof(servaddr));

        // 填充服务器信息
        this->servaddr.sin_family = AF_INET; // 使用IPv4
        this->servaddr.sin_port = htons(this->m_udp_target_port); // 服务器端口

        // 将IPv4和IPv6地址从文本转换成二进制形式
        QByteArray target_ip_addr = this->target_addr_string.toUtf8();
        if(inet_pton(AF_INET, target_ip_addr.data(), &servaddr.sin_addr)<=0) {
            return;
        }
    }

    while(!this->isInterruptionRequested()) {
            this->send_byte_data(this->send_quque.dequeue());
    }
}

void SocketWorkerThread::slot_connect_udp(QString ip_addr, int port)
{
    this->target_addr_string = ip_addr;
    this->m_udp_target_port = port;
    this->socket_class = UDP;
    this->start();
}

void SocketWorkerThread::slot_connect_tcp(QString ip_addr, int port)
{
    this->target_addr_string = ip_addr;
    this->m_tcp_target_port = port;
    this->socket_class = TCP;
    this->start();
}

uint32_t reverseEndian(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
}

void SocketWorkerThread::send_byte_data(QByteArray send_data)
{

    int dataLength = send_data.size() - 4;
    int standPacketSize = dataLength / TFP_ONE_IMG_SEND_PART;
    int send_packet_size = FRAME_HEADER_SIZE + standPacketSize + FRAME_NUM_SIZE +
            FRAME_LENGTH_SIZE + FRAME_SERIAL_SIZE + FRAME_END_SIZE;

    int packetSize = 0;
    int packetNum = qCeil(dataLength/(standPacketSize * 1.0));//向上取整

    const char* data = send_data.constData();


//    qDebug() << "frame_num:" << frame_num;
//    qDebug() << "packetNum:"<< packetNum << "dataLength:" << dataLength;

    for(int i = 0; i < packetNum; i++) {
        if(standPacketSize*(i+1) <= dataLength) {
            packetSize = standPacketSize;
        } else {
            packetSize = dataLength - standPacketSize*i;
        }

#if(FRAME_HEADER_SIZE != 0)
        *(quint32*)(this->send_buf) = FRAME_HEADER;
#endif

#if(FRAME_END_SIZE != 0)
        *(quint32*)(this->send_buf + send_packet_size - FRAME_END_SIZE) = FRAME_END;
#endif

#if(FRAME_NUM_SIZE != 0)
    uint8_t frame_num = *(uint8_t*)(data + TFP_IMG_SIZE);
    this->send_buf[FRAME_HEADER_SIZE] = frame_num;
#endif

#if(FRAME_SERIAL_SIZE != 0)
    this->send_buf[FRAME_HEADER_SIZE + 1] = (char)i;
#endif

#if(FRAME_LENGTH_SIZE != 0)
        *(quint16*)(this->send_buf+ FRAME_HEADER_SIZE + FRAME_SERIAL_SIZE + FRAME_NUM_SIZE) = send_packet_size;
#endif

        memcpy(this->send_buf + FRAME_HEADER_SIZE + FRAME_SERIAL_SIZE + FRAME_LENGTH_SIZE + FRAME_NUM_SIZE,
           data, packetSize);

        sendto(this->sockfd, (const char *)this->send_buf, send_packet_size,
                0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

        data = data + packetSize;
    }

}

void SocketWorkerThread::tcp_send_byte_data(QByteArray send_data)
{
    int dataLength = send_data.size() - 4;
    int standPacketSize = dataLength/ TFP_ONE_IMG_SEND_PART;
    int send_packet_size = standPacketSize + FRAME_NUM_SIZE + FRAME_LENGTH_SIZE + FRAME_SERIAL_SIZE + FRAME_HEADER_SIZE;

//    int packetSize = 0;
    int packetNum = qCeil(dataLength/(standPacketSize * 1.0));//向上取整

    const char* data = send_data.constData();

    uint32_t frame_num = *(uint32_t*)(data + TFP_IMG_SIZE);
//    qDebug() << "frame_num:" << frame_num;

    for(int i = 0; i < packetNum; i++) {

        this->send_buf[0] = (char)0xA5;
        *(quint32*)(this->send_buf+1) = frame_num;
        this->send_buf[5] = (char)i;
        *(quint16*)(this->send_buf+6) = send_packet_size;

        memcpy(this->send_buf+FRAME_HEADER_SIZE + FRAME_SERIAL_SIZE + FRAME_LENGTH_SIZE + FRAME_NUM_SIZE,
           data, standPacketSize);


        data = data + standPacketSize;
    }

}
