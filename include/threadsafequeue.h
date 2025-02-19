#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

//#include <QQueue>
//#include <QMutex>
//#include <QWaitCondition>


//#include <QThread>
//#include <QLinkedList>
//#include <QMutex>
//#include <QWaitCondition>

//template<typename T>
//class ThreadSafeQueue
//{
//public:
//    ThreadSafeQueue() : m_size(0) {}

//    // 入队
//    void enqueue(const T& item) {
//        QMutexLocker locker(&m_mutex);
//        m_queue.append(item);
//        ++m_size;
//        m_condition.wakeOne(); // 唤醒等待的线程
//    }

//    // 出队
//    T dequeue() {
//        QMutexLocker locker(&m_mutex);
//        while (m_queue.isEmpty()) {
//            m_condition.wait(&m_mutex); // 队列为空时等待
//        }
//        T item = m_queue.takeFirst();
//        --m_size;
//        return item;
//    }

//    // 获取队列大小
//    int size() const {
//        QMutexLocker locker(&m_mutex);
//        return m_size;
//    }

//private:
//    QLinkedList<T> m_queue; // 队列实现使用QLinkedList
//    QMutex m_mutex; // 互斥锁
//    QWaitCondition m_condition; // 条件变量
//    int m_size; // 队列大小
//};


#include <QList>
#include <QMutex>
#include <QWaitCondition>

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    void enqueue(const T& item) {
        enqueue_impl(item);
    }

    void enqueue(T&& item) {
        enqueue_impl(std::move(item));
    }

    T dequeue() {
        QMutexLocker locker(&m_mutex);
        while (m_queue.isEmpty() && !m_shutdown) {
            m_condition.wait(&m_mutex);
        }
        if (m_shutdown) {
            throw std::runtime_error("Queue shutdown");
        }
        return m_queue.takeFirst();
    }

    bool tryDequeue(T& item) {
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty()) return false;
        item = m_queue.takeFirst();
        return true;
    }

    void shutdown() {
        QMutexLocker locker(&m_mutex);
        m_shutdown = true;
        m_condition.wakeAll();
    }

    int size() const {
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }

    ~ThreadSafeQueue() {
        shutdown();
    }

private:
    template<typename U>
    void enqueue_impl(U&& item) {
        QMutexLocker locker(&m_mutex);
        m_queue.append(std::forward<U>(item));
        m_condition.wakeOne();
    }

    QList<T> m_queue;
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_shutdown = false;
};

#endif // THREADSAFEQUEUE_H
