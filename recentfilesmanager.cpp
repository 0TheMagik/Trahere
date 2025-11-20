#include "recentfilesmanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <algorithm>
#include <QSet>
#include <QProcessEnvironment>
#include <QtConcurrent/QtConcurrent>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <deque>

RecentFilesModel::RecentFilesModel(QObject *parent)
    : QAbstractListModel(parent) {
    // Load cached recent files immediately for instant UI feedback
    loadCache();

    // Start a background scan to refresh the cache and model
    startBackgroundScan();
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_files.count();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_files.count()) return QVariant();

    const RecentFileInfo &file = m_files.at(index.row());

    switch (role) {
        case FileNameRole: return file.fileName;
        case FilePathRole: return file.filePath;
        case DateModifiedRole: return file.dateModified;
        default: return QVariant();
    }
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const {
    return {
        {FileNameRole, "fileName"},
        {FilePathRole, "filePath"},
        {DateModifiedRole, "dateModified"}
    };
}

QString RecentFilesModel::formatDateTime(const QDateTime &dt) {
    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = dt.secsTo(now);

    if (secondsDiff < 60) return "Just now";
    if (secondsDiff < 3600) return QString::number(secondsDiff / 60) + "m ago";
    if (secondsDiff < 86400) return QString::number(secondsDiff / 3600) + "h ago";
    if (secondsDiff < 604800) return QString::number(secondsDiff / 86400) + "d ago";

    return dt.toString("MMM d, yyyy");
}

void RecentFilesModel::addFileIfOra(const QString &path) {
    if (!path.toLower().endsWith(".ora")) return;

    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) return;

    RecentFileInfo file;
    file.fileName = info.fileName();
    file.filePath = info.absoluteFilePath();
    file.fileDate = info.lastModified();
    file.dateModified = formatDateTime(file.fileDate);

    m_files.append(file);
}

void RecentFilesModel::scanForOraFiles() {
    // Synchronous (full) scan - kept for explicit refresh
    beginResetModel();
    m_files.clear();

    // Scan standard document locations (cross-platform)
    QStringList docPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    docPaths << QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    docPaths << QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);

#ifdef Q_OS_WIN
    QString userName = qEnvironmentVariable("USERNAME");
    if (!userName.isEmpty()) {
        docPaths << QString("C:/Users/%1/Documents").arg(userName);
        docPaths << QString("C:/Users/%1/Pictures").arg(userName);
        docPaths << QString("C:/Users/%1/Downloads").arg(userName);
        QString oneDrivePath = qEnvironmentVariable("OneDrive");
        if (!oneDrivePath.isEmpty()) {
            docPaths << oneDrivePath;
        }
    }
#endif

    // Add project images path as a fast path
    docPaths << "C:/Users/REY/OneDrive/Tugas Rey/Tugas Rey/5. RPL/KEL/Project/Trahere/Images";

    // Use name-filtered recursive search (synchronous)
    for (const QString &docPath : docPaths) {
        QDir dir(docPath);
        if (!dir.exists()) continue;
        QDirIterator it(dir.absolutePath(), QStringList() << "*.ora", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            addFileIfOra(it.next());
        }
    }

    // Remove duplicates and sort
    QSet<QString> seen;
    QList<RecentFileInfo> uniqueFiles;
    for (const RecentFileInfo &file : m_files) {
        if (!seen.contains(file.filePath)) {
            seen.insert(file.filePath);
            uniqueFiles.append(file);
        }
    }
    m_files = uniqueFiles;
    std::sort(m_files.begin(), m_files.end(), [](const RecentFileInfo &a, const RecentFileInfo &b){ return a.fileDate > b.fileDate; });
    if (m_files.size() > 20) m_files.resize(20);
    endResetModel();
    qDebug() << "Found" << m_files.count() << "recent .ora files (sync)";
}

// Load cached list from app data
void RecentFilesModel::loadCache()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(cacheDir);
    QString cacheFile = QDir(cacheDir).absoluteFilePath("recent_files.json");
    QFile f(cacheFile);
    if (!f.exists()) return;
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument d = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!d.isArray()) return;
    QJsonArray arr = d.array();
    QVector<RecentFileInfo> files;
    for (const QJsonValue &v : arr) {
        if (!v.isObject()) continue;
        QJsonObject o = v.toObject();
        RecentFileInfo rf;
        rf.fileName = o.value("fileName").toString();
        rf.filePath = o.value("filePath").toString();
        rf.dateModified = o.value("dateModified").toString();
        rf.fileDate = QDateTime::fromString(o.value("fileDate").toString(), Qt::ISODate);
        files.append(rf);
    }
    if (!files.isEmpty()) {
        beginResetModel();
        m_files = files;
        endResetModel();
    }
    // Remove any files that no longer exist
    removeMissingFiles();
}

// Remove files that no longer exist from the list
void RecentFilesModel::removeMissingFiles()
{
    QVector<RecentFileInfo> validFiles;
    bool changed = false;
    
    for (const RecentFileInfo &file : m_files) {
        if (QFileInfo::exists(file.filePath)) {
            validFiles.append(file);
        } else {
            qDebug() << "Removed missing file from recent list:" << file.filePath;
            changed = true;
        }
    }
    
    if (changed) {
        beginResetModel();
        m_files = validFiles;
        endResetModel();
        saveCache();
        qDebug() << "Recent files list cleaned:" << validFiles.count() << "files remain";
    }
}

void RecentFilesModel::saveCache()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(cacheDir);
    QString cacheFile = QDir(cacheDir).absoluteFilePath("recent_files.json");
    QJsonArray arr;
    for (const RecentFileInfo &rf : m_files) {
        QJsonObject o;
        o["fileName"] = rf.fileName;
        o["filePath"] = rf.filePath;
        o["dateModified"] = rf.dateModified;
        o["fileDate"] = rf.fileDate.toString(Qt::ISODate);
        arr.append(o);
    }
    QJsonDocument d(arr);
    QFile f(cacheFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(d.toJson());
        f.close();
    }
}

// Start an asynchronous bounded BFS scan and update model on completion
void RecentFilesModel::startBackgroundScan()
{
    if (m_watcher) return; // already running or set up
    m_watcher = new QFutureWatcher<QList<RecentFileInfo>>(this);

    // Prepare roots
    QStringList roots = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    roots << QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    roots << QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
#ifdef Q_OS_WIN
    QString userName = qEnvironmentVariable("USERNAME");
    if (!userName.isEmpty()) roots << QString("C:/Users/%1/Documents").arg(userName);
#endif
    roots << QString("C:/Users/REY/OneDrive/Tugas Rey/Tugas Rey/5. RPL/KEL/Project/Trahere/Images");

    auto worker = [roots]() -> QList<RecentFileInfo> {
        QList<RecentFileInfo> found;
        // BFS bounded scan
        std::deque<QString> queue;
        for (const QString &r : roots) queue.push_back(r);
        const int maxDirs = 2000;
        const int perDirLimit = 40;
        int scanned = 0;
        QSet<QString> seenDirs;
        while (!queue.empty() && scanned < maxDirs) {
            QString dpath = queue.front(); queue.pop_front();
            if (dpath.isEmpty()) continue;
            QDir dir(dpath);
            if (!dir.exists()) continue;
            QString abs = dir.absolutePath();
            if (seenDirs.contains(abs)) continue;
            seenDirs.insert(abs);

            // find .ora files in this directory
            QStringList files = dir.entryList(QStringList() << "*.ora", QDir::Files);
            for (const QString &f : files) {
                QFileInfo info(dir.filePath(f));
                RecentFileInfo rf;
                rf.fileName = info.fileName();
                rf.filePath = info.absoluteFilePath();
                rf.fileDate = info.lastModified();
                rf.dateModified = rf.fileDate.toString(Qt::ISODate);
                found.append(rf);
            }

            // enqueue subdirs up to perDirLimit
            QStringList subs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            int added = 0;
            for (const QString &s : subs) {
                if (added >= perDirLimit) break;
                queue.push_back(dir.filePath(s));
                ++added;
            }
            ++scanned;
        }
        return found;
    };

    QFuture<QList<RecentFileInfo>> future = QtConcurrent::run(worker);
    m_watcher->setFuture(future);
    connect(m_watcher, &QFutureWatcher<QList<RecentFileInfo>>::finished, this, [this]() {
        QList<RecentFileInfo> result = m_watcher->result();
        if (!result.isEmpty()) {
            // merge with existing, remove duplicates, sort
            QSet<QString> seen;
            QVector<RecentFileInfo> merged;
            for (const RecentFileInfo &r : result) {
                if (!seen.contains(r.filePath)) { seen.insert(r.filePath); merged.append(r); }
            }
            // Append existing entries that weren't found to keep them available
            for (const RecentFileInfo &r : m_files) {
                if (!seen.contains(r.filePath)) { seen.insert(r.filePath); merged.append(r); }
            }
            std::sort(merged.begin(), merged.end(), [](const RecentFileInfo &a, const RecentFileInfo &b){ return a.fileDate > b.fileDate; });
            if (merged.size() > 20) merged.resize(20);
            beginResetModel();
            m_files = merged;
            endResetModel();
            // Remove any files that no longer exist
            removeMissingFiles();
            qDebug() << "Background scan finished, found" << m_files.count() << ".ora files";
        } else {
            qDebug() << "Background scan finished, no .ora files found.";
        }
        emit backgroundScanFinished();
    });
}

void RecentFilesModel::refresh() {
    // Refresh using the background scan instead of blocking the UI
    // Clear the old watcher if it exists and start a fresh scan
    if (m_watcher) {
        delete m_watcher;
        m_watcher = nullptr;
    }
    startBackgroundScan();
    qDebug() << "Refresh initiated - background scan started";
}
