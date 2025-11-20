#include "OraLoader.h"
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
OraLoader::~OraLoader() = default;

OraLoader :: OraLoader(QObject *parent) : QObject(parent) {}

QString OraLoader :: loadOra(const QUrl &sourceUrl) {
    m_stackXml.clear();
    m_rootDir.clear();
    if (!sourceUrl.isValid()) {
        qWarning() << "OraLoader: invalid url";
        return{};
    }

    QString path = sourceUrl.isLocalFile() ? sourceUrl.toLocalFile() : sourceUrl.toString();
    if (!path.endsWith(".ora", Qt::CaseInsensitive)) {
        qWarning() << "OraLoader: not an .ora file:" << path;
        return {};
    }

    if (!QFileInfo :: exists(path)) {
        qWarning() << "OraLoader: file doesnot exist" << path;
        return {};
    }

    // Create (or re-create) a temporary directory whose lifetime matches this OraLoader instance.
    m_tmpDir = std::make_unique<QTemporaryDir>();
    if (!m_tmpDir->isValid()) {
        qWarning() << "OraLoader: failed to create temp dir";
        return {};
    }
    QString outDir = m_tmpDir->path();
    m_rootDir = outDir;

    // Expand-Archive only supports .zip extension. Copy the ORA to a temp .zip first.
    const QString zipPath = outDir + "/archive.zip";
    if (QFile::exists(zipPath)) {
        QFile::remove(zipPath);
    }
    if (!QFile::copy(path, zipPath)) {
        qWarning() << "OraLoader: failed to copy ORA to temp ZIP:" << zipPath;
        return {};
    }

    QString ps = "Expand-Archive -LiteralPath \"" + zipPath + "\" -DestinationPath \"" + outDir + "\" -Force";
    QProcess proc;
    proc.start("powershell", QStringList() << "-NoProfile" << "-Command" << ps);
    if (!proc.waitForFinished(15000)) {
        qWarning() << "OraLoader: unzip timeout";
        return {};
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        qWarning() << "OraLoader: unzip failed" << proc.readAllStandardError();
        return {};
    }

    QString stack = outDir + "/stack.xml";
    if (QFileInfo::exists(stack)) {
        m_stackXml = stack;
    } else {
        qWarning() << "OraLoader: missing stack.xml";
    }
    return outDir;
}

QStringList OraLoader::layerImagePaths() const {
    QStringList result;
    if (m_stackXml.isEmpty()) return result;
    QFile f(m_stackXml);
    if (!f.open(QIODevice::ReadOnly)) return result;
    const QString xml = QString::fromUtf8(f.readAll());
    QRegularExpression re("src=\"([^\"]+)\"");
    auto it = re.globalMatch(xml);
    while (it.hasNext()) {
        auto m = it.next();
        QString rel = m.captured(1);
        if (!rel.isEmpty()) {
            QString abs = QDir(m_rootDir).absoluteFilePath(rel);
            result << abs;
        }
    }
    return result;
}


