#include "mainwindow.h"

#include <iostream>
#include <QFile>
#include <QDir>
#include <QScopedPointer>
#include <QApplication>
#include <QStyleFactory>

static QScopedPointer<QFile> m_log;

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Q_UNUSED(context)
  QTextStream outFile(m_log.data());

  QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
  outFile << datetime;
  std::cout << datetime.toLocal8Bit().toStdString();

  switch (type) {
      case QtDebugMsg:
          std::cout << "[DEBUG] ";
          outFile << "[DEBUG] ";
          break;
      case QtInfoMsg:
          std::cout << "[INFO] ";
          outFile << "[INFO] ";
          break;
      case QtWarningMsg:
          std::cout << "[WARNING] ";
          outFile << "[WARNING] ";
          break;
      case QtCriticalMsg:
          std::cout << "[CRITICAL] ";
          outFile << "[CRITICAL] ";
          break;
      case QtFatalMsg:
          std::cout << "[FATAL] ";
          outFile << "[FATAL] ";
          break;
  }

  outFile << msg << Qt::endl;
  std::cout << msg.toLocal8Bit().toStdString() << std::endl;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

   m_log.reset(new QFile(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/log.txt"));
   m_log.data()->open(QFile::Append | QFile::Text);

#ifdef Q_OS_WIN32
    QApplication::setStyle(QStyleFactory::create("Fusion"));
#endif

#ifdef Q_OS_LINUX
    a.setWindowIcon(QIcon(":resources/images/logo_mini.svg"));
#endif

  qInstallMessageHandler(messageHandler);

  MainWindow w;
  w.show();
  return a.exec();
}
