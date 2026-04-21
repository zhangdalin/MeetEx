#ifndef AVATAR_GENERATOR_H
#define AVATAR_GENERATOR_H

#include <QColor>
#include <QImage>
#include <QSize>
#include <QString>

struct AvatarOptions {
    QSize size = QSize(512, 512);
    QColor backgroundTop = QColor(34, 52, 90);
    QColor backgroundBottom = QColor(18, 28, 52);
    QColor silhouetteColor = QColor(255, 255, 255, 45);
    QColor textColor = QColor(255, 255, 255);
    int maxInitials = 2;
};

class AvatarGenerator {
public:
    static QString initialsFromName(const QString &userName);
    static QImage generateAvatar(const QString &userName);
};

#endif // AVATAR_GENERATOR_H
