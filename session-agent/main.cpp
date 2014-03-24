
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QApplication>
#include <QtQml/QtQml>
#include <QDebug>

#include "secretagent.h"
#include "interfacemanager.h"
#include "AvailableConnectionsModel.h"
#include "AvailableConnectionsSortModel.h"
#include "IconProvider.h"

using namespace std;

InterfaceManager *manager;

static QObject *manager_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return manager;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(application);

    QCoreApplication::setOrganizationName("7land");
    QCoreApplication::setOrganizationDomain("ceciletti.com.br");
    QCoreApplication::setApplicationName("nm");
    QCoreApplication::setApplicationVersion("2013.12");

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QCoreApplication::installTranslator(&qtTranslator);

    new SecretAgent;
    manager = new InterfaceManager;
    qmlRegisterSingletonType<InterfaceManager>("org.land", 1, 0, "Manager", manager_provider);
    qmlRegisterType<AvailableConnectionsModel>("org.land", 1, 0, "AvailableConnectionsModel");
    qmlRegisterType<AvailableConnectionsSortModel>("org.land", 1, 0, "AvailableConnectionsSortModel");
    manager->init();

    return app.exec();
}
