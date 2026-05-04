#pragma once
#include <QString>
#include <QList>

struct Card {
    int id = 0;
    int columnId = 0;
    QString title;
    QString description;
    int position = 0;
};

struct Column {
    int id = 0;
    QString title;
    int position = 0;
    QList<Card> cards;  // ordered by card.position
};
