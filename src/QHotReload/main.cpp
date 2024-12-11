#include <qapplication.h>
#include <qdialog.h>
#include <qevent.h>
#include <qmimedata.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qdir.h>
#if (QT_VERSION_MAJOR == 5)
#    include <qtextcodec.h>
#endif
#include <qresource.h>
#include <qicon.h>
#include <qtimer.h>
#include <qdebug.h>
#include "SimpleFileWatcher/FileWatcher.h"
#include "QHotReload/config/config.h"
#include "QHotReload/config/version.h"

class FileWatchListenerImpl final : public QObject, public FW::FileWatchListener
{
    Q_OBJECT
public:
    explicit FileWatchListenerImpl(const QString& qmlMainFile, QObject* parent = nullptr)
        : QObject(parent)
        , FileWatchListener()
        , qmlMainFile_(qmlMainFile)
    {
        tmr_.setInterval(500);
        tmr_.setSingleShot(true);
        QObject::connect(
            &tmr_, &QTimer::timeout, this, [this]() { emit fileChanged(qmlMainFile_); });
    }

    void notifyFileChanged() { emit fileChanged(qmlMainFile_); }

signals:
    void fileChanged(const QString& filename);

private:
    virtual void handleFileAction(FW::WatchID watchid, const FW::String& dir,
                                  const FW::String& filename, FW::Action action) override
    {
        Q_UNUSED(watchid);
        Q_UNUSED(dir);
        Q_UNUSED(filename);
        Q_UNUSED(action);

        tmr_.start();
    }

    QTimer        tmr_;
    const QString qmlMainFile_;
};

class Win final : public QDialog
{
public:
    explicit Win(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setAcceptDrops(true);

        auto lab = new QLabel("请拖拽QML主文件至此窗口", this);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(lab);
        setLayout(layout);

        resize(500, 500);
        setWindowTitle(qApp->applicationName() + ' ' + qApp->applicationVersion());
    }

    auto qmlMainFile() const { return qmlMainFile_; }

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
        for (const auto& url : event->mimeData()->urls()) {
            const auto filename = url.toLocalFile();
            if (QFileInfo(filename).suffix().compare("qml", Qt::CaseInsensitive) == 0) {
                qmlMainFile_ = filename;
                event->accept();
                return;
            }
        }
        event->ignore();
    }
    virtual void dropEvent(QDropEvent* event) override
    {
        if (!qmlMainFile().isEmpty()) {
            QDialog::accept();
        }
    }

private:
    QString qmlMainFile_;
};
#include "main.moc"

struct Config
{
public:
    explicit Config() {}

    QString qmlMainFile;
    QString qmlMainPath;

    bool isValid() const
    {
        if (qmlMainFile.isEmpty() || qmlMainPath.isEmpty()) {
            return false;
        }
        return true;
    }

    void print() const
    {
        qDebug() << "[Config]";
        qDebug() << "\tqmlMainFile:" << qmlMainFile;
        qDebug() << "\tqmlMainPath:" << qmlMainPath;
        qDebug() << "";
    }

public:
    static Config Parse(int argc, char* argv[])
    {
        Config conf;
        if (argc == 1) {
            Win w;
            if (w.exec() == QDialog::Accepted) {
                conf.qmlMainFile = w.qmlMainFile();
            }
        }
        else {
            switch (argc) {
            case 2: conf.qmlMainFile = argv[1];
            }
        }
        if (QFileInfo fi(conf.qmlMainFile); fi.exists()) {
            conf.qmlMainPath = fi.absoluteDir().absolutePath();
        }
        conf.qmlMainFile.replace("\\", "/");
        conf.qmlMainPath.replace("\\", "/");
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
    app.setWindowIcon(QIcon(":/res/Top.png"));

    const auto conf = Config::Parse(argc, argv);
    if (!conf.isValid()) {
        qDebug() << "usage:"
                 << TOP_NAME
#ifdef _WIN32
            ".exe"
#endif
                 << "QmlMainFile";
        return 1;
    }
    conf.print();

    if (!QDir::setCurrent(conf.qmlMainPath)) {
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
    FileWatchListenerImpl fileWatchListener(conf.qmlMainFile);
    fileWatcher.addWatch(conf.qmlMainPath.toStdString(), &fileWatchListener, true);
    engine.rootContext()->setContextProperty("fileWatchListener", &fileWatchListener);

    QObject::connect(&fileWatchListener, &FileWatchListenerImpl::fileChanged, [&engine]() {
        engine.clearComponentCache();
    });

    const auto kAppFile = QUrl(
#if (QT_VERSION_MAJOR == 5)
#    if (QT_VERSION_MINOR == 6)
        "qrc:/qml/App-5.6.qml"
#    elif (QT_VERSION_MINOR == 12)
        "qrc:/qml/App-5.12.qml"
#    elif (QT_VERSION_MINOR == 15)
        "qrc:/qml/App-5.15.qml"
#    endif
#elif (QT_VERSION_MAJOR == 6)
        "qrc:/qml/App-6.qml"
#endif
    );
    engine.load(kAppFile);
    if (engine.rootObjects().isEmpty()) {
        qDebug() << "QQmlApplicationEngine rootObjects isEmpty";
        return -1;
    }
    fileWatchListener.notifyFileChanged();

    return app.exec();
}
