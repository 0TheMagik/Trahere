#include "OraCreator.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTextStream>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <utility>
#include <utility> // std::as_const

namespace {

// CRC32 table (polynomial 0xEDB88320)
static const quint32 CRC32_TABLE[256] = {
    0x00000000U,0x77073096U,0xEE0E612CU,0x990951BAU,0x076DC419U,0x706AF48FU,0xE963A535U,0x9E6495A3U,
    0x0EDB8832U,0x79DCB8A4U,0xE0D5E91EU,0x97D2D988U,0x09B64C2BU,0x7EB17CBDU,0xE7B82D07U,0x90BF1D91U,
    0x1DB71064U,0x6AB020F2U,0xF3B97148U,0x84BE41DEU,0x1ADAD47DU,0x6DDDE4EBU,0xF4D4B551U,0x83D385C7U,
    0x136C9856U,0x646BA8C0U,0xFD62F97AU,0x8A65C9ECU,0x14015C4FU,0x63066CD9U,0xFA0F3D63U,0x8D080DF5U,
    0x3B6E20C8U,0x4C69105EU,0xD56041E4U,0xA2677172U,0x3C03E4D1U,0x4B04D447U,0xD20D85FDU,0xA50AB56BU,
    0x35B5A8FAU,0x42B2986CU,0xDBBBC9D6U,0xACBCF940U,0x32D86CE3U,0x45DF5C75U,0xDCD60DCFU,0xABD13D59U,
    0x26D930ACU,0x51DE003AU,0xC8D75180U,0xBFD06116U,0x21B4F4B5U,0x56B3C423U,0xCFBA9599U,0xB8BDA50FU,
    0x2802B89EU,0x5F058808U,0xC60CD9B2U,0xB10BE924U,0x2F6F7C87U,0x58684C11U,0xC1611DABU,0xB6662D3DU,
    0x76DC4190U,0x01DB7106U,0x98D220BCU,0xEFD5102AU,0x71B18589U,0x06B6B51FU,0x9FBFE4A5U,0xE8B8D433U,
    0x7807C9A2U,0x0F00F934U,0x9609A88EU,0xE10E9818U,0x7F6A0DBBU,0x086D3D2DU,0x91646C97U,0xE6635C01U,
    0x6B6B51F4U,0x1C6C6162U,0x856530D8U,0xF262004EU,0x6C0695EDU,0x1B01A57BU,0x8208F4C1U,0xF50FC457U,
    0x65B0D9C6U,0x12B7E950U,0x8BBEB8EAU,0xFCB9887CU,0x62DD1DDFU,0x15DA2D49U,0x8CD37CF3U,0xFBD44C65U,
    0x4DB26158U,0x3AB551CEU,0xA3BC0074U,0xD4BB30E2U,0x4ADFA541U,0x3DD895D7U,0xA4D1C46DU,0xD3D6F4FBU,
    0x4369E96AU,0x346ED9FCU,0xAD678846U,0xDA60B8D0U,0x44042D73U,0x33031DE5U,0xAA0A4C5FU,0xDD0D7CC9U,
    0x5005713CU,0x270241AAU,0xBE0B1010U,0xC90C2086U,0x5768B525U,0x206F85B3U,0xB966D409U,0xCE61E49FU,
    0x5EDEF90EU,0x29D9C998U,0xB0D09822U,0xC7D7A8B4U,0x59B33D17U,0x2EB40D81U,0xB7BD5C3BU,0xC0BA6CADU,
    0xEDB88320U,0x9ABFB3B6U,0x03B6E20CU,0x74B1D29AU,0xEAD54739U,0x9DD277AFU,0x04DB2615U,0x73DC1683U,
    0xE3630B12U,0x94643B84U,0x0D6D6A3EU,0x7A6A5AA8U,0xE40ECF0BU,0x9309FF9DU,0x0A00AE27U,0x7D079EB1U,
    0xF00F9344U,0x8708A3D2U,0x1E01F268U,0x6906C2FEU,0xF762575DU,0x806567CBU,0x196C3671U,0x6E6B06E7U,
    0xFED41B76U,0x89D32BE0U,0x10DA7A5AU,0x67DD4ACCU,0xF9B9DF6FU,0x8EBEEFF9U,0x17B7BE43U,0x60B08ED5U,
    0xD6D6A3E8U,0xA1D1937EU,0x38D8C2C4U,0x4FDFF252U,0xD1BB67F1U,0xA6BC5767U,0x3FB506DDU,0x48B2364BU,
    0xD80D2BDAU,0xAF0A1B4CU,0x36034AF6U,0x41047A60U,0xDF60EFC3U,0xA867DF55U,0x316E8EEFU,0x4669BE79U,
    0xCB61B38CU,0xBC66831AU,0x256FD2A0U,0x5268E236U,0xCC0C7795U,0xBB0B4703U,0x220216B9U,0x5505262FU,
    0xC5BA3BBEU,0xB2BD0B28U,0x2BB45A92U,0x5CB36A04U,0xC2D7FFA7U,0xB5D0CF31U,0x2CD99E8BU,0x5BDEAE1DU,
    0x9B64C2B0U,0xEC63F226U,0x756AA39CU,0x026D930AU,0x9C0906A9U,0xEB0E363FU,0x72076785U,0x05005713U,
    0x95BF4A82U,0xE2B87A14U,0x7BB12BAEU,0x0CB61B38U,0x92D28E9BU,0xE5D5BE0DU,0x7CDCEFB7U,0x0BDBDF21U,
    0x86D3D2D4U,0xF1D4E242U,0x68DDB3F8U,0x1FDA836EU,0x81BE16CDU,0xF6B9265BU,0x6FB077E1U,0x18B74777U,
    0x88085AE6U,0xFF0F6A70U,0x66063BCAU,0x11010B5CU,0x8F659EFFU,0xF862AE69U,0x616BFFD3U,0x166CCF45U,
    0xA00AE278U,0xD70DD2EEU,0x4E048354U,0x3903B3C2U,0xA7672661U,0xD06016F7U,0x4969474DU,0x3E6E77DBU,
    0xAED16A4AU,0xD9D65ADCU,0x40DF0B66U,0x37D83BF0U,0xA9BCAE53U,0xDEBB9EC5U,0x47B2CF7FU,0x30B5FFE9U,
    0xBDBDF21CU,0xCABAC28AU,0x53B39330U,0x24B4A3A6U,0xBAD03605U,0xCDD70693U,0x54DE5729U,0x23D967BFU,
    0xB3667A2EU,0xC4614AB8U,0x5D681B02U,0x2A6F2B94U,0xB40BBE37U,0xC30C8EA1U,0x5A05DF1BU,0x2D02EF8DU
};

static quint32 crc32(const QByteArray &data)
{
    quint32 crc = 0xFFFFFFFFU;
    const uchar *p = reinterpret_cast<const uchar*>(data.constData());
    for (qsizetype i = 0; i < data.size(); ++i) {
        crc = (crc >> 8) ^ CRC32_TABLE[(crc ^ p[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFFU;
}

static void toDosDateTime(const QDateTime &dt, quint16 &dosTime, quint16 &dosDate)
{
    QDate d = dt.date();
    QTime t = dt.time();
    quint16 year = static_cast<quint16>(qMax(1980, d.year()) - 1980);
    dosDate = (year << 9) | (d.month() << 5) | d.day();
    dosTime = (t.hour() << 11) | (t.minute() << 5) | (t.second() / 2);
}

struct ZipEntryMeta {
    QByteArray nameUtf8;
    quint16 flags = 0x0800; // UTF-8
    quint16 method = 0;     // 0=store
    quint16 modTime = 0;
    quint16 modDate = 0;
    quint32 crc = 0;
    quint32 compSize = 0;
    quint32 uncompSize = 0;
    quint32 localHeaderOffset = 0;
};

class SimpleZipWriter {
public:
    bool open(const QString &filePath) {
        f.setFileName(filePath);
        return f.open(QIODevice::WriteOnly);
    }
    bool add(const QString &name, const QByteArray &data) {
        if (!f.isOpen()) return false;
        ZipEntryMeta meta;
        meta.nameUtf8 = name.toUtf8();
        toDosDateTime(QDateTime::currentDateTime(), meta.modTime, meta.modDate);
        meta.crc = crc32(data);
        meta.uncompSize = static_cast<quint32>(data.size());
        meta.compSize = meta.uncompSize; // stored
        meta.localHeaderOffset = static_cast<quint32>(f.pos());

        QDataStream out(&f);
        out.setByteOrder(QDataStream::LittleEndian);
        // Local file header
        out << quint32(0x04034B50)
            << quint16(20)            // version needed to extract
            << meta.flags
            << meta.method
            << meta.modTime
            << meta.modDate
            << meta.crc
            << meta.compSize
            << meta.uncompSize
            << quint16(meta.nameUtf8.size())
            << quint16(0);            // extra len
        f.write(meta.nameUtf8);
        // No extra field
        // File data
        if (meta.compSize) f.write(data);

        entries.push_back(meta);
        return true;
    }
    bool close() {
        if (!f.isOpen()) return false;
        const quint32 centralDirOffset = static_cast<quint32>(f.pos());
        QDataStream out(&f);
        out.setByteOrder(QDataStream::LittleEndian);
        // Central directory
        for (const ZipEntryMeta &e : std::as_const(entries)) {
            out << quint32(0x02014B50)
                << quint16(20) // version made by
                << quint16(20) // version needed to extract
                << e.flags
                << e.method
                << e.modTime
                << e.modDate
                << e.crc
                << e.compSize
                << e.uncompSize
                << quint16(e.nameUtf8.size())
                << quint16(0)  // extra len
                << quint16(0)  // comment len
                << quint16(0)  // disk number start
                << quint16(0)  // internal attrs
                << quint32(0)  // external attrs
                << e.localHeaderOffset;
            f.write(e.nameUtf8);
        }
        const quint32 centralDirEnd = static_cast<quint32>(f.pos());
        const quint32 centralDirSize = centralDirEnd - centralDirOffset;
        // End of central directory
        out << quint32(0x06054B50)
            << quint16(0) // disk
            << quint16(0) // start disk
            << quint16(entries.size())
            << quint16(entries.size())
            << centralDirSize
            << centralDirOffset
            << quint16(0); // comment len
        f.close();
        return true;
    }
private:
    QFile f;
    QVector<ZipEntryMeta> entries;
};

} // namespace

OraCreator::OraCreator(QObject *parent)
    : QObject(parent)
{
}

bool OraCreator::createOra(const QString &destinationPath, int width, int height)
{
    qWarning() << "OraCreator.createOra -> destinationPath:" << destinationPath << ", size:" << width << "x" << height;

    // Ensure destination directory exists
    QFileInfo destInfo(destinationPath);
    const QString destDirPath = destInfo.absolutePath();
    if (!destDirPath.isEmpty()) {
        QDir().mkpath(destDirPath);
    }

    // Prepare in-memory contents
    const int w = width > 0 ? width : 1;
    const int h = height > 0 ? height : 1;

    // Transparent background layer
    QImage layerImg(w, h, QImage::Format_ARGB32);
    layerImg.fill(Qt::transparent);
    QByteArray layerPng;
    {
        QBuffer buf(&layerPng);
        buf.open(QIODevice::WriteOnly);
        if (!layerImg.save(&buf, "PNG")) {
            qWarning() << "Failed to encode layer PNG";
            return false;
        }
    }

    // Thumbnail (recommended by spec) - 256 max dimension
    const int thumbMax = 256;
    QImage thumb = layerImg;
    if (w > thumbMax || h > thumbMax) {
        thumb = layerImg.scaled(thumbMax, thumbMax, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QByteArray thumbPng;
    {
        QBuffer buf(&thumbPng);
        buf.open(QIODevice::WriteOnly);
        thumb.save(&buf, "PNG");
    }

    // stack.xml
    QByteArray stackXml;
    {
        QTextStream out(&stackXml, QIODevice::WriteOnly);
        out.setEncoding(QStringConverter::Utf8);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<image w=\"" << w << "\" h=\"" << h << "\" version=\"0.0.1\">\n";
        out << "  <stack>\n";
        out << "    <layer name=\"Background\" src=\"data/layer0.png\" x=\"0\" y=\"0\" opacity=\"1.0\" visible=\"true\"/>\n";
        out << "  </stack>\n";
        out << "</image>\n";
    }

    // Write ZIP with correct ordering: mimetype first, uncompressed.
    SimpleZipWriter zip;
    if (!zip.open(destinationPath)) {
        qWarning() << "Failed to open destination for writing:" << destinationPath;
        return false;
    }

    // 1) mimetype (must be first, stored, exact content)
    if (!zip.add(QStringLiteral("mimetype"), QByteArray("image/openraster"))) {
        qWarning() << "Failed to add mimetype to zip";
        return false;
    }
    // 2) stack.xml at root
    if (!zip.add(QStringLiteral("stack.xml"), stackXml)) {
        qWarning() << "Failed to add stack.xml to zip";
        return false;
    }
    // 3) data/layer0.png
    if (!zip.add(QStringLiteral("data/layer0.png"), layerPng)) {
        qWarning() << "Failed to add layer PNG to zip";
        return false;
    }
    // 4) Thumbnails/thumbnail.png (optional but recommended)
    if (!zip.add(QStringLiteral("Thumbnails/thumbnail.png"), thumbPng)) {
        qWarning() << "Failed to add thumbnail to zip";
        return false;
    }

    if (!zip.close()) {
        qWarning() << "Failed closing zip";
        return false;
    }

    qWarning() << "Successfully created .ora at" << destinationPath;
    return true;
}

bool OraCreator::createOra(const QUrl &destinationUrl, int width, int height)
{
    if (!destinationUrl.isValid()) {
        qWarning() << "Invalid QUrl passed to createOra:" << destinationUrl;
        return false;
    }
    QString local = destinationUrl.isLocalFile() ? destinationUrl.toLocalFile() : destinationUrl.toString();
    if (!local.endsWith(".ora", Qt::CaseInsensitive)) {
        local += ".ora";
    }
    return createOra(local, width, height);
}

bool OraCreator::saveOra(const QString &destinationPath, const QImage &layerImg)
{
    if (layerImg.isNull()) {
        qWarning() << "saveOra: layer image is null";
        return false;
    }

    QFileInfo destInfo(destinationPath);
    const QString destDirPath = destInfo.absolutePath();
    if (!destDirPath.isEmpty()) QDir().mkpath(destDirPath);

    // Encode layer PNG
    QByteArray layerPng;
    {
        QBuffer buf(&layerPng);
        buf.open(QIODevice::WriteOnly);
        if (!layerImg.save(&buf, "PNG")) {
            qWarning() << "saveOra: failed encode layer PNG";
            return false;
        }
    }

    // Thumbnail (max 256)
    QImage thumb = layerImg;
    const int thumbMax = 256;
    if (thumb.width() > thumbMax || thumb.height() > thumbMax) {
        thumb = thumb.scaled(thumbMax, thumbMax, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    QByteArray thumbPng;
    {
        QBuffer buf(&thumbPng);
        buf.open(QIODevice::WriteOnly);
        thumb.save(&buf, "PNG");
    }

    // stack.xml (single layer)
    QByteArray stackXml;
    {
        QTextStream out(&stackXml, QIODevice::WriteOnly);
        out.setEncoding(QStringConverter::Utf8);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<image w=\"" << layerImg.width() << "\" h=\"" << layerImg.height() << "\" version=\"0.0.1\">\n";
        out << "  <stack>\n";
        out << "    <layer name=\"Layer 0\" src=\"data/layer0.png\" x=\"0\" y=\"0\" opacity=\"1.0\" visible=\"true\"/>\n";
        out << "  </stack>\n";
        out << "</image>\n";
    }

    SimpleZipWriter zip;
    if (!zip.open(destinationPath)) {
        qWarning() << "saveOra: cannot open file" << destinationPath;
        return false;
    }
    if (!zip.add(QStringLiteral("mimetype"), QByteArray("image/openraster"))) return false;
    if (!zip.add(QStringLiteral("stack.xml"), stackXml)) return false;
    if (!zip.add(QStringLiteral("data/layer0.png"), layerPng)) return false;
    if (!zip.add(QStringLiteral("Thumbnails/thumbnail.png"), thumbPng)) return false;
    if (!zip.close()) return false;
    qWarning() << "saveOra: wrote" << destinationPath;
    return true;
}

bool OraCreator::saveOra(const QUrl &destinationUrl, const QImage &layerImg)
{
    if (!destinationUrl.isValid()) return false;
    QString local = destinationUrl.isLocalFile() ? destinationUrl.toLocalFile() : destinationUrl.toString();
    if (!local.endsWith(".ora", Qt::CaseInsensitive)) local += ".ora";
    return saveOra(local, layerImg);
}
