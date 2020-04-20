#include <QCoreApplication>
#include <iotsystemserver.h>

int main(int argc, char *argv[])
{
    IotSystemServer a(argc, argv);
    return a.exec();
}
