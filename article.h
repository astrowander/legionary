#ifndef ARTICLE_H
#define ARTICLE_H

#include "includes.h"
#include "tinyint.h"

#include <QHash>
#include <QList>
#include <QString>

struct Article
{
    QString title;
    QList<TinyInt> links;
};

typedef QHash<TinyInt, Article>::iterator ArticleIt;

#endif // ARTICLE_H

