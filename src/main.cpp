#include <QCoreApplication>
#include <cmdctrl.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CmdCtrl CmdCtrl(argc, argv);

    return a.exec();
}
