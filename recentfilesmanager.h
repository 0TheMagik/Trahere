#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QFileInfo>
#include <QDateTime>
#include <QFutureWatcher>

struct RecentFileInfo {
    QString fileName;
    QString filePath;
    QString dateModified;
    QDateTime fileDate;
};

class RecentFilesModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        FileNameRole = Qt::UserRole + 1,
        FilePathRole,
        DateModifiedRole
    };

    explicit RecentFilesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void scanForOraFiles();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void startBackgroundScan();
    void removeMissingFiles();

signals:
    void backgroundScanFinished();

private:
    QVector<RecentFileInfo> m_files;
    void addFileIfOra(const QString &path);
    QString formatDateTime(const QDateTime &dt);
    void loadCache();
    void saveCache();
    QFutureWatcher<QList<RecentFileInfo>> *m_watcher = nullptr;
};

#endif // RECENTFILESMANAGER_H
