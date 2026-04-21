#include "avatar_generator.h"

#include <QGuiApplication>
#include <QDebug>
#include <QStringList>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    const QString userName = "";
    const QString outputFile = "./avatar_out.png";

    const QImage image = AvatarGenerator::generateAvatar(userName);
    if (image.isNull()) {
        qCritical().noquote() << "Failed to generate avatar for" << userName;
        return 2;
    }

    if (!image.save(outputFile, "PNG")) {
        qCritical().noquote() << "Failed to generate avatar for" << userName << "->" << outputFile;
        return 2;
    }

    qInfo().noquote() << "Avatar generated:" << outputFile;
    qInfo().noquote() << "Initials:" << AvatarGenerator::initialsFromName(userName);
    return 0;
}
