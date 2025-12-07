#include <QtTest>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include "ora/OraLoader.h"

static QUrl makeFileUrl(const QString &absPath) {
    return QUrl::fromLocalFile(absPath);
}
static bool fileExists(const QString &absPath) {
    QFileInfo fi(absPath);
    return fi.exists() && fi.isFile();
}
static bool dirExists(const QString &absPath) {
    QDir d(absPath);
    return d.exists();
}

class TestOraLoader : public QObject {
    Q_OBJECT
private slots:
    void initial_state_is_empty() {
        OraLoader loader;
        QCOMPARE(loader.stackXmlPath(), QString());
        QCOMPARE(loader.rootDir(), QString());
        QCOMPARE(loader.layerImagePaths(), QStringList());
        // OraLoader defaults to invalid QSize (-1,-1) when unknown
        QCOMPARE(loader.imageWidth(), -1);
        QCOMPARE(loader.imageHeight(), -1);
    }

    void loadOra_with_empty_url_fails() {
        OraLoader loader;
        const QString res = loader.loadOra(QUrl());
        QCOMPARE(res, QString());
        QCOMPARE(loader.rootDir(), QString());
        QCOMPARE(loader.stackXmlPath(), QString());
        QCOMPARE(loader.layerImagePaths(), QStringList());
        QCOMPARE(loader.imageWidth(), -1);
        QCOMPARE(loader.imageHeight(), -1);
    }

    void loadOra_with_nonexistent_file_fails() {
        OraLoader loader;
        const QString bogusPath = QDir::temp().filePath("nonexistent_12345.ora");
        QVERIFY(!QFileInfo::exists(bogusPath));
        const QString res = loader.loadOra(makeFileUrl(bogusPath));
        QCOMPARE(res, QString());
        QCOMPARE(loader.rootDir(), QString());
        QCOMPARE(loader.stackXmlPath(), QString());
        QCOMPARE(loader.layerImagePaths(), QStringList());
        QCOMPARE(loader.imageWidth(), -1);
        QCOMPARE(loader.imageHeight(), -1);
    }

    void loadOra_with_non_ora_extension_fails() {
        QTemporaryFile tmp("XXXXXX.txt");
        QVERIFY(tmp.open());
        tmp.write("not an ora archive");
        tmp.flush();

        OraLoader loader;
        const QString res = loader.loadOra(makeFileUrl(tmp.fileName()));
        QCOMPARE(res, QString());
        QCOMPARE(loader.rootDir(), QString());
        QCOMPARE(loader.stackXmlPath(), QString());
        QCOMPARE(loader.layerImagePaths(), QStringList());
        QCOMPARE(loader.imageWidth(), -1);
        QCOMPARE(loader.imageHeight(), -1);
    }

    void loadOra_valid_archive_succeeds_and_parses_stack() {
        const QString sample = QFINDTESTDATA("samples/valid.ora");
        if (sample.isEmpty() || !QFileInfo::exists(sample)) {
            QSKIP("samples/valid.ora not found; skipping functional test.");
        }

        OraLoader loader;
        const QString root = loader.loadOra(makeFileUrl(sample));
        QVERIFY(!root.isEmpty());
        QVERIFY(dirExists(root));

        const QString stackPath = loader.stackXmlPath();
        QVERIFY(!stackPath.isEmpty());
        QVERIFY(fileExists(stackPath));
        QCOMPARE(loader.rootDir(), root);

        const QStringList layers = loader.layerImagePaths();
        QVERIFY(!layers.isEmpty());
        for (const QString &p : layers) {
            QVERIFY2(fileExists(p), qPrintable(QString("Layer image missing: %1").arg(p)));
            QVERIFY(QFileInfo(p).isAbsolute());
            QVERIFY(p.startsWith(root));
        }

        QVERIFY(loader.imageWidth() > 0);
        QVERIFY(loader.imageHeight() > 0);
    }

    void loadOra_archive_without_stack_xml_handles_gracefully() {
        const QString sample = QFINDTESTDATA("samples/no_stack_xml.ora");
        if (sample.isEmpty() || !QFileInfo::exists(sample)) {
            QSKIP("samples/no_stack_xml.ora not found; skipping functional test.");
        }

        OraLoader loader;
        const QString root = loader.loadOra(makeFileUrl(sample));
        QVERIFY(!root.isEmpty());
        QVERIFY(dirExists(root));

        QCOMPARE(loader.rootDir(), root);
        QCOMPARE(loader.stackXmlPath(), QString());
        QCOMPARE(loader.layerImagePaths(), QStringList());
        // Unknown image size should be non-positive (e.g., -1)
        QVERIFY(loader.imageWidth() <= 0);
        QVERIFY(loader.imageHeight() <= 0);
    }

    void repeated_loads_replace_temp_dir_and_state() {
        const QString sample1 = QFINDTESTDATA("samples/valid.ora");
        const QString sample2 = QFINDTESTDATA("samples/valid2.ora");
        if (sample1.isEmpty() || !QFileInfo::exists(sample1) ||
            sample2.isEmpty() || !QFileInfo::exists(sample2)) {
            QSKIP("samples/valid.ora or samples/valid2.ora not found; skipping test.");
        }

        OraLoader loader;
        const QString root1 = loader.loadOra(makeFileUrl(sample1));
        QVERIFY(!root1.isEmpty());
        QVERIFY(dirExists(root1));
        const int w1 = loader.imageWidth();
        const int h1 = loader.imageHeight();
        const QStringList layers1 = loader.layerImagePaths();
        QVERIFY(w1 > 0 && h1 > 0 && !layers1.isEmpty());

        const QString root2 = loader.loadOra(makeFileUrl(sample2));
        QVERIFY(!root2.isEmpty());
        QVERIFY(dirExists(root2));
        QVERIFY(root2 != root1);
        const int w2 = loader.imageWidth();
        const int h2 = loader.imageHeight();
        const QStringList layers2 = loader.layerImagePaths();
        QVERIFY(w2 > 0 && h2 > 0 && !layers2.isEmpty());

        QCOMPARE(loader.rootDir(), root2);
        QVERIFY(loader.stackXmlPath().startsWith(root2));
    }
};

QTEST_MAIN(TestOraLoader)

#include "test_ora_loader.moc"

