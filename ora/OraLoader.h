#pragma once
#include <QObject>
#include <QSize>
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
    // Image dimensions from <image w="" h=""> in stack.xml (0,0 if unknown)
    Q_INVOKABLE int imageWidth() const { return m_imageSize.width(); }
    Q_INVOKABLE int imageHeight() const { return m_imageSize.height(); }

private:
    QString m_stackXml;
    QString m_rootDir;
    QSize m_imageSize; // parsed from stack.xml
    // Holds the lifetime of the extracted .ora archive. Re-created on each loadOra() call.
    std::unique_ptr<QTemporaryDir> m_tmpDir; 
};

