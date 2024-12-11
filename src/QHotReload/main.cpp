#include <qapplication.h>
#include <qdialog.h>
#include <qevent.h>
#include <qmimedata.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qdir.h>
#include <qdiriterator.h>
#if (QT_VERSION_MAJOR == 5)
#    include <qtextcodec.h>
#endif
#include <qresource.h>
#include <qicon.h>
#include <qtimer.h>
#include <qprocess.h>
#include <qdebug.h>
#include "SimpleFileWatcher/FileWatcher.h"
#include "QHotReload/config/config.h"
#include "QHotReload/config/version.h"

class FileWatchListenerImpl final : public QObject, public FW::FileWatchListener
{
    Q_OBJECT
public:
    explicit FileWatchListenerImpl(const QString& qmlRootPath, const QString& qmlMainFile,
                                   QObject* parent = nullptr)
        : QObject(parent)
        , FileWatchListener()
        , qmlMainFile_(qmlMainFile)
    {
        tmr_.setInterval(500);
        tmr_.setSingleShot(true);
        QObject::connect(
            &tmr_, &QTimer::timeout, this, &FileWatchListenerImpl::onNotifyFileChanged);

        QDirIterator it(
            qmlRootPath, QStringList() << "*.qrc", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            qrcFiles_.append(it.next());
        }
    }

signals:
    void fileChanged(const QString& filename);

public slots:
    void onNotifyFileChanged()
    {
        qrcFiles_.removeDuplicates();
        for (const auto& qrc : qrcFiles_) {
            onReloadRcc(qrc);
        }
        qrcFiles_.clear();
        emit fileChanged(qmlMainFile_);
    }

private:
    virtual void handleFileAction(FW::WatchID watchid, const FW::String& dir,
                                  const FW::String& filename, FW::Action action) override
    {
        Q_UNUSED(watchid);
        Q_UNUSED(action);

        if (QFileInfo(filename.c_str()).suffix().compare("qrc", Qt::CaseInsensitive) == 0) {
            qrcFiles_.append(QString("%1/%2").arg(dir.c_str()).arg(filename.c_str()));
        }
        tmr_.start();
    }

    void onReloadRcc(const QString& qrcName)
    {
        const auto rccPath = QString("%1/%2").arg(QDir::tempPath()).arg(qApp->applicationName());
        const auto rccName = QString("%1.rcc").arg(QFileInfo(qrcName).baseName());
        const auto cmd = QString("rcc -binary %1 -o %2/%3").arg(qrcName).arg(rccPath).arg(rccName);
        if (QDir dir(rccPath); !dir.exists()) {
            if (!dir.mkpath(rccPath)) {
                qDebug() << "onReloadRcc mkpath(" << rccPath << ") failed...";
                return;
            }
        }

        const auto rccFilename = QString("%1/%2").arg(rccPath).arg(rccName);
        QResource::unregisterResource(rccFilename);
        const auto code = QProcess::execute(cmd);
        if (code == 0) {
            if (!QResource::registerResource(rccFilename)) {
                qDebug() << "onReloadRcc registerResource(" << rccFilename << ") failed...";
            }
        }
        else {
            qDebug() << "onReloadRcc execute(" << cmd << ") code:" << code;
        }
    }

    QTimer        tmr_;
    QStringList   qrcFiles_;
    const QString qmlMainFile_;
};

class ConfigDialog final : public QDialog
{
public:
    explicit ConfigDialog(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setAcceptDrops(true);

        leQmlMainFile = new QLineEdit(this);
        leQmlRootPath = new QLineEdit(this);
        auto lab      = new QLabel("可拖拽至此窗口完成配置项", this);
        auto btn      = new QPushButton("确定", this);

        leQmlMainFile->setPlaceholderText("请配置QML主文件");
        leQmlRootPath->setPlaceholderText("请配置QML顶层目录");
        QObject::connect(btn, &QPushButton::clicked, this, [this]() {
            if (leQmlMainFile->text().isEmpty()) {
                QMessageBox::information(this, "配置", "请至少配置QML主文件");
                return;
            }
            if (!QFileInfo(leQmlMainFile->text()).exists()) {
                QMessageBox::information(this, "配置", "QML主文件不存在");
                return;
            }
            if (!leQmlRootPath->text().isEmpty()) {
                if (QFileInfo(leQmlRootPath->text()).isDir()) {
                    QMessageBox::information(this, "配置", "QML顶层目录不存在");
                    return;
                }
            }
            QDialog::accept();
        });

        auto layout = new QVBoxLayout(this);
        layout->addWidget(leQmlMainFile);
        layout->addWidget(leQmlRootPath);
        layout->addWidget(lab);
        layout->addWidget(btn);
        setLayout(layout);

        resize(500, 300);
        setWindowTitle(qApp->applicationName() + ' ' + qApp->applicationVersion());
    }

    auto qmlMainFile() const { return leQmlMainFile->text(); }
    auto qmlRootPath() const { return leQmlRootPath->text(); }

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override
    {
        if (!event->mimeData()->hasUrls()) {
            event->ignore();
            return;
        }
        if (event->mimeData()->urls().size() != 1) {
            event->ignore();
            return;
        }
        event->accept();
    }
    virtual void dropEvent(QDropEvent* event) override
    {
        for (const auto& url : event->mimeData()->urls()) {
            const auto localFile = url.toLocalFile();
            const auto fileinfo  = QFileInfo(localFile);
            if (fileinfo.isFile()) {
                if (fileinfo.suffix().compare("qml", Qt::CaseInsensitive) == 0) {
                    leQmlMainFile->setText(localFile);
                    event->accept();
                    return;
                }
            }
            else if (fileinfo.isDir()) {
                leQmlRootPath->setText(localFile);
                event->accept();
                return;
            }
        }
        event->ignore();
    }

private:
    QLineEdit* leQmlMainFile;
    QLineEdit* leQmlRootPath;
};
#include "main.moc"

struct Config
{
public:
    explicit Config() {}

    QString qmlMainFile;
    QString qmlRootPath;

    bool isValid() const
    {
        if (qmlMainFile.isEmpty() || qmlRootPath.isEmpty()) {
            return false;
        }
        if (!qmlMainFile.startsWith(qmlRootPath)) {
            return false;
        }
        return true;
    }

    void print() const
    {
        qDebug() << "[Config]";
        qDebug() << "\tqmlMainFile:" << qmlMainFile;
        qDebug() << "\tqmlRootPath:" << qmlRootPath;
        qDebug() << "";
    }

public:
    static Config Parse(int argc, char* argv[])
    {
        Config conf;
        if (argc == 1) {
            ConfigDialog dlg;
            if (dlg.exec() == QDialog::Accepted) {
                conf.qmlMainFile = dlg.qmlMainFile();
                conf.qmlRootPath = dlg.qmlRootPath();
            }
        }
        else {
            switch (argc) {
            case 3: conf.qmlRootPath = argv[2]; [[fallthrough]];
            case 2: conf.qmlMainFile = argv[1];
            }
        }
        if (conf.qmlRootPath.isEmpty()) {
            if (QFileInfo fi(conf.qmlMainFile); fi.exists()) {
                conf.qmlRootPath = fi.absoluteDir().absolutePath();
            }
        }
        conf.qmlMainFile.replace("\\", "/");
        conf.qmlRootPath.replace("\\", "/");
        return conf;
    }
};

/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
 *                  / \\|||  :  |||// \
 *                 / _||||| -:- |||||- \
 *                |   | \\\  - /// |   |
 *                | \_|  ''\---/''  |_/ |
 *                \  .-\__  '-'  ___/-. /
 *              ___'. .'  /--.--\  `. .'___
 *           ."" '<  `.___\_<|>_/___.' >' "".
 *          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *          \  \ `_.   \_ __\ /__ _/   .-` /  /
 *      =====`-.____`.___ \_____/___.-`___.-'=====
 *                        `=---='
 *
 *
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *            佛祖保佑       永不宕机     永无BUG
 */
int main(int argc, char* argv[])
{
    qputenv("QT_OPENGL", "angle");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

#if (QT_VERSION_MAJOR == 5)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    app.setOrganizationName("白日做Meng技术无限公司");
    app.setApplicationName(TOP_NAME);
    app.setApplicationVersion(TOP_VERSION);
    app.setWindowIcon(QIcon("qrc:/QHotReload/res/Top.png"));

    const auto conf = Config::Parse(argc, argv);
    if (!conf.isValid()) {
        qDebug() << "usage:"
                 << TOP_NAME
#ifdef _WIN32
            ".exe"
#endif
                 << "QmlMainFile"
                 << "QmlRootPath";
        return 1;
    }
    conf.print();

    if (!QDir::setCurrent(conf.qmlRootPath)) {
        qDebug() << "WorkingDirectory";
        return 2;
    }

    qDebug() << "[Version]";
    qDebug() << QString("\t%1: %2")
                    .arg(app.applicationName())
                    .arg(app.applicationVersion())
                    .toUtf8()
                    .data();
    qDebug() << "\tQt:" << QT_VERSION_STR;
    qDebug() << "";

    QQmlApplicationEngine engine;

    FW::FileWatcher       fileWatcher;
    FileWatchListenerImpl fileWatchListener(conf.qmlRootPath, conf.qmlMainFile);
    fileWatcher.addWatch(conf.qmlRootPath.toStdString(), &fileWatchListener, true);
    engine.rootContext()->setContextProperty("fileWatchListener", &fileWatchListener);

    QObject::connect(&fileWatchListener, &FileWatchListenerImpl::fileChanged, [&engine]() {
        engine.clearComponentCache();
    });

    const auto kAppFile = QUrl(
#if (QT_VERSION_MAJOR == 5)
#    if (QT_VERSION_MINOR == 6)
        "qrc:/QHotReload/qml/App-5.6.qml"
#    elif (QT_VERSION_MINOR == 12)
        "qrc:/QHotReload/qml/App-5.12.qml"
#    elif (QT_VERSION_MINOR == 15)
        "qrc:/QHotReload/qml/App-5.15.qml"
#    endif
#elif (QT_VERSION_MAJOR == 6)
        "qrc:/QHotReload/qml/App-6.qml"
#endif
    );
    engine.load(kAppFile);
    if (engine.rootObjects().isEmpty()) {
        qDebug() << "QQmlApplicationEngine rootObjects isEmpty";
        return -1;
    }
    fileWatchListener.onNotifyFileChanged();

    return app.exec();
}
