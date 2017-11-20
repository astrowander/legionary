#ifndef DATABASE_H
#define DATABASE_H

#include "article.h"

#include <functional>

#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>

class Database
{
    QHash<uint32_t, Article> _articles;
    QHash<QString, Article*> _dictionary;

    ArticleIt InsertNewArticle(uint32_t id)
    {
        return _articles.insert(id, Article());
    }

    ArticleIt FindArticleById(uint32_t id)
    {
        return _articles.find(id);
    }

    void AddTitleToDict(ArticleIt it, const QString & title)
    {
        it->title = title;
        _dictionary.insert(title, &it.value());
    }

    void AddLink(ArticleIt it, const QString & title)
    {
        auto linkIt = _dictionary.find(title);
        if (linkIt != _dictionary.end())
            it->links.append(linkIt.value());
    }

    void ProcessSqlFile(const QString & path, std::function<ArticleIt(uint32_t)> getIterator, std::function<void(ArticleIt, const QString &)> processIterator)
    {
        QFile pageFile(path);
        if (!pageFile.open(QIODevice::ReadOnly))
            throw std::runtime_error("Could not open file");

        QTextStream pageStream(&pageFile);

        QString word;
        QChar ch;

        while (ch!= '/')
        {
            word.clear();
            while (word != "VALUES")
                pageStream >> word;

            pageStream >> ch; //skip a space

            while (ch != ';')
            {
                pageStream >> ch; //skip '('

                uint32_t id = 0;
                pageStream >> id;

                auto it = getIterator(id);

                if (it == _articles.end())
                    continue;

                while (ch != '\'') // search beginning of the title
                {
                    pageStream >> ch;
                }


                QString title;

                pageStream >> ch; //*reading the first symbol of the title
                do
                {
                    title.append(ch);
                    pageStream >> ch;
                } while (ch != '\'');

                processIterator(it, title);

                while (ch != ')') //skipping extra data
                {
                    pageStream >> ch;
                }

                pageStream >> ch; //read the next char, it may be ',' or ';'
            }

            pageStream >> ch; //skip '.'
            pageStream >> ch; //read the next char, it may be 'I'[insert next chunk of data] or '/' [end of table]
        }

        pageFile.close();
    }

    void ExtractAbstracts(const QString & path)
    {
        QFile xmlData(path);
        if (!xmlData.open(QIODevice::ReadOnly))
            throw std::runtime_error("Could not open file");

        QXmlStreamReader reader(&xmlData);

        while (!reader.atEnd())
        {
            auto token = reader.readNext();

            if (token == QXmlStreamReader::StartElement && reader.name() == "page")
            {
                ParsePage(reader);
            }

        }

        xmlData.close();
    }

    void ParsePage(QXmlStreamReader & reader)
    {
        ArticleIt it = _articles.end();

        while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == "page"))
        {
            if (reader.tokenType() == QXmlStreamReader::StartElement)
            {
                if (reader.name() == "id" && it == _articles.end())
                {
                    reader.readNext();
                    it = _articles.find(reader.text().toInt());
                }
                else if (reader.name() == "text")
                {
                    reader.readNext();
                    ParseWikiText(it, reader.text());
                }
            }

            reader.readNext();
        }
    }

    void ParseWikiText(ArticleIt it, QStringRef wikiText)
    {
        qDebug() << "stop";
    }

public:
    Database()
    {
        using namespace std::placeholders;

        ProcessSqlFile("./../wikidb/ruwiki-latest-page.sql", std::bind(&Database::InsertNewArticle, this, _1), std::bind(&Database::AddTitleToDict, this, _1, _2));
        ProcessSqlFile("./../wikidb/ruwiki-latest-pagelinks.sql", std::bind(&Database::FindArticleById, this, _1), std::bind(&Database::AddLink, this, _1, _2));
        //ExtractAbstracts("./../wikidb/ruwiki-latest-pages-articles1.xml");
    }
};

#endif // DATABASE_H
