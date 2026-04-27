#include "historylogger.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

HistoryLogger::HistoryLogger(const QString& filePath)
    : m_filePath(filePath) {}

QString HistoryLogger::filePath() const {
    return m_filePath;
}

QString HistoryLogger::log(const QString& username, const QString& action, const QString& details) {
    const QString ts = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    const QString line = QString("%1 | %2 | %3 | %4").arg(ts, username, action, details);

    QFile f(m_filePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << line << "\n";
        f.close();
    }
    return line;
}

QStringList HistoryLogger::readAllLines() const {
    QStringList lines;

    QFile f(m_filePath);
    if (!f.exists()) return lines;
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return lines;

    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (!line.trimmed().isEmpty())
            lines.append(line);
    }
    f.close();

    return lines;
}