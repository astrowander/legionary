#include "database.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Database db;

    QTextStream cin(stdin);
    QTextStream cout(stdout);


    /*while (true)
    {
        cout << "Введите название статьи: ";
        QString token = cin.readLine(255);

        ArticleIt = _di
        if (token == exit)
        {
            break;
        }
        else ()
    }*/

    return a.exec();
}
