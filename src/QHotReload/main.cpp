#include <qapplication.h>
#include <qdialog.h>
#include <qevent.h>
#include <qmimedata.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qdir.h>
#include <qtextcodec.h>
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
        tmr_.start();
    }

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
        {
            layout->addWidget(lab);
        }

        setLayout(layout);

        resize(500, 500);
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
            if (QFileInfo(filename).suffix().toLower() == "qml") {
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
        conf.qmlMainFile = QDir::toNativeSeparators(conf.qmlMainFile);
        if (QFileInfo fi(conf.qmlMainFile); fi.exists()) {
            conf.qmlMainPath = QDir::toNativeSeparators(fi.absoluteDir().absolutePath());
        }
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

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    app.setOrganizationName("白日做Meng技术无限公司");
    app.setApplicationName(TOP_NAME "UI");
    app.setApplicationVersion(TOP_VERSION);
    app.setWindowIcon(QIcon(":/res/TopUI.png"));

    const auto conf = Config::Parse(argc, argv);
    if (!conf.isValid()) {
        qDebug() << "usage:" << TOP_NAME << "QmlMainFile";
        return 1;
    }
    conf.print();

    if (!QDir::setCurrent(app.applicationDirPath())) {
        qDebug() << "WorkingDirectory";
        return 2;
    }

    if (!QResource::registerResource("rcc/qml.rcc")) {
        qDebug() << "registerResource qml.rcc failed";
        return 3;
    }

    qDebug() << "[Version]";
    qDebug() << "\tQt:" << QT_VERSION_STR;
    qDebug() << "";

    QQmlApplicationEngine engine;

    FW::FileWatcher       fileWatcher;
    FileWatchListenerImpl fileWatchListener(conf.qmlMainFile);
    fileWatcher.addWatch(conf.qmlMainPath.toStdString(), &fileWatchListener, true);
    engine.rootContext()->setContextProperty("fileWatchListener", &fileWatchListener);

    engine.load("qrc:/qml/App.qml");
    if (engine.rootObjects().isEmpty()) {
        qDebug() << "QQmlApplicationEngine rootObjects isEmpty";
        return -1;
    }

    return app.exec();
}
