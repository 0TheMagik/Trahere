#pragma once
#include <QObject>
#include <memory>

class QUrl;              // fwd decl to avoid heavy includes in header
class QTemporaryDir;     // fwd decl; complete type used in .cpp

class OraLoader : public QObject
{
    Q_OBJECT
public:
    explicit OraLoader(QObject *parent = nullptr);
    ~OraLoader();

    // Load .ora (QUrl from FileDialog). Returns temp directory path with extracted content or empty on failure.
    Q_INVOKABLE QString loadOra(const QUrl &sourceUrl);

    //call this to get absolute path to stack.xml (empty if not present).
    Q_INVOKABLE QString stackXmlPath() const { return m_stackXml; }
    Q_INVOKABLE QString rootDir() const { return m_rootDir; }
    // Return absolute paths to layer image files referenced in stack.xml (if loaded)
    Q_INVOKABLE QStringList layerImagePaths() const;

private:
    QString m_stackXml;
    QString m_rootDir;
    // Holds the lifetime of the extracted .ora archive. Re-created on each loadOra() call.
    std::unique_ptr<QTemporaryDir> m_tmpDir; 
};

