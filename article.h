#ifndef ARTICLE_H
#define ARTICLE_H

#include "includes.h"

#include <QHash>
#include <QList>
#include <QString>

struct Article
{
    QString title;
    QList<Article*> links;
};

typedef QHash<uint32_t, Article>::iterator ArticleIt;

#endif // ARTICLE_H

