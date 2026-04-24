#include "avatar_generator.h"

#include <QBrush>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QGradient>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRegularExpression>

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

namespace {

bool isAsciiLetterOrDigit(QChar c) {
    const ushort uc = c.unicode();
    return (uc >= '0' && uc <= '9') || (uc >= 'A' && uc <= 'Z') || (uc >= 'a' && uc <= 'z');
}

bool isCjkUnifiedIdeograph(QChar c) {
    const uint code = c.unicode();
    return (code >= 0x4E00 && code <= 0x9FFF) || (code >= 0x3400 && code <= 0x4DBF);
}

QChar pinyinInitialFromGb2312Code(int areaCode) {
    static const int kThresholds[] = {
        1601, 1637, 1833, 2078, 2274, 2302, 2433, 2594, 2787,
        3106, 3212, 3472, 3635, 3722, 3730, 3858, 4027, 4086,
        4390, 4558, 4684, 4925, 5249
    };
    static const char kLetters[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
        'T', 'W', 'X', 'Y', 'Z'
    };

    for (int i = 22; i >= 0; --i) {
        if (areaCode >= kThresholds[i]) {
            return QChar(kLetters[i]);
        }
    }
    return QChar('X');
}

QChar chineseCharToPinyinInitial(QChar c) {
#ifdef _WIN32
    wchar_t wch = static_cast<wchar_t>(c.unicode());
    char mb[4] = {0};
    const int len = WideCharToMultiByte(936, 0, &wch, 1, mb, static_cast<int>(sizeof(mb)), nullptr, nullptr);
    if (len >= 2) {
        const unsigned char b1 = static_cast<unsigned char>(mb[0]);
        const unsigned char b2 = static_cast<unsigned char>(mb[1]);
        if (b1 >= 0xB0 && b1 <= 0xF7 && b2 >= 0xA1 && b2 <= 0xFE) {
            const int areaCode = (static_cast<int>(b1) - 160) * 100 + (static_cast<int>(b2) - 160);
            return pinyinInitialFromGb2312Code(areaCode);
        }
    }
#endif
    return QChar('X');
}

QString buildInitials(const QString &name) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("NA");
    }

    // Rule 1: For Chinese names, use first two Chinese characters (or one if only one).
    QString chineseChars;
    for (const QChar c : trimmed) {
        if (isCjkUnifiedIdeograph(c)) {
            chineseChars += c;
            if (chineseChars.size() >= 2) {
                return chineseChars;
            }
        }
    }
    if (!chineseChars.isEmpty()) {
        return chineseChars.left(1);
    }

    // Rule 2/3: For words, use initials of first two words; if only one word, use its first letter.
    const QStringList words = trimmed.split(QRegularExpression(QStringLiteral("[^A-Za-z0-9]+")),
                                            Qt::SkipEmptyParts);
    if (!words.isEmpty()) {
        if (words.size() >= 2) {
            return (words[0].left(1) + words[1].left(1)).toUpper();
        }
        return words[0].left(1).toUpper();
    }

    // Fallback: use visible first character.
    return trimmed.left(1).toUpper();
}

void drawDefaultPortraitBackground(QPainter &p, const QSize &size, const AvatarOptions &opt) {
    QLinearGradient bg(0, 0, 0, static_cast<qreal>(size.height()));
    bg.setColorAt(0.0, opt.backgroundTop);
    bg.setColorAt(1.0, opt.backgroundBottom);
    p.fillRect(QRect(QPoint(0, 0), size), bg);

    const qreal w = static_cast<qreal>(size.width());
    const qreal h = static_cast<qreal>(size.height());

    // Subtle vignette for depth.
    QRadialGradient vignette(w * 0.5, h * 0.45, h * 0.7);
    vignette.setColorAt(0.0, QColor(255, 255, 255, 26));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 50));
    p.fillRect(QRect(QPoint(0, 0), size), vignette);

    p.setRenderHint(QPainter::Antialiasing, true);

    p.setPen(Qt::NoPen);
    p.setBrush(opt.silhouetteColor);

    const qreal headRadius = w * 0.05;
    const QPointF headCenter(w * 0.5, h * 0.43);
    p.drawEllipse(headCenter, headRadius, headRadius);

    QPainterPath shoulders;
    shoulders.moveTo(w * 0.375, h * 0.60);
    shoulders.cubicTo(w * 0.39, h * 0.54, w * 0.61, h * 0.54, w * 0.625, h * 0.60);
    shoulders.closeSubpath();
    p.drawPath(shoulders);
}

void applyCircularMask(QImage &image) {
    const int diameter = static_cast<int>(std::min(image.width(), image.height()) * 0.425);
    const int offsetX = (image.width() - diameter) / 2;
    const int offsetY = (image.height() - diameter) / 2;

    QImage masked(image.size(), QImage::Format_ARGB32_Premultiplied);
    masked.fill(Qt::transparent);

    QPainter maskPainter(&masked);
    maskPainter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath clipPath;
    clipPath.addEllipse(QRectF(offsetX, offsetY, diameter, diameter));
    maskPainter.setClipPath(clipPath);
    maskPainter.drawImage(0, 0, image);
    maskPainter.end();

    image = masked;
}

} // namespace

QString AvatarGenerator::initialsFromName(const QString &userName) {
    return buildInitials(userName).toUpper();
}

QImage AvatarGenerator::generateAvatar(const QString &userName) {
    AvatarOptions options;
    const QSize size = options.size.isValid() ? options.size : QSize(512, 512);
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    drawDefaultPortraitBackground(painter, size, options);

    const QString initials = initialsFromName(userName);
    const int pixelSize = std::max(24, static_cast<int>(size.width() * 0.13));

    QFont font(QStringLiteral("Microsoft YaHei"));
    font.setBold(true);
    font.setPixelSize(pixelSize);
    font.setLetterSpacing(QFont::AbsoluteSpacing, std::max(2.0, size.width() * 0.005));
    painter.setFont(font);

    painter.setPen(options.textColor);
    painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter, initials);

    painter.end();

    applyCircularMask(image);

    return image;
}
