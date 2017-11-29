#ifndef DATABASE_H
#define DATABASE_H

#include "article.h"
#include "tinyint.h"

#include <functional>

#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>

class Database
{
    QHash<TinyInt, Article> _articles;
    QHash<QString, TinyInt> _dictionary;

    ArticleIt InsertNewArticle(uint32_t id)
    {
        return _articles.insert(TinyInt(id), Article());
    }

    ArticleIt FindArticleById(uint32_t id)
    {
        return _articles.find(TinyInt(id));
    }

    void AddTitleToDict(ArticleIt it, const QString & title)
    {
        it->title = title;
        if (!_dictionary.contains(title))
            _dictionary.insert(title, it.key());
    }

    void AddLink(ArticleIt it, const QString & title)
    {
        auto linkIt = _dictionary.find(title);
        if (linkIt != _dictionary.end() && !it->links.contains(linkIt.value()))
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

               pageStream >> ch; //skip ','

                int nspace = 0;
                pageStream >> nspace;

                auto it = getIterator(id);

                if (nspace == 0 && it != _articles.end())
                {
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
                }

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
        int bracketLevel = 0;
        int i = 0;

        QString abstract;

        while (i < wikiText.length())
        {
            if (wikiText.at(i) == '{')
            {
                ++bracketLevel;
            }
            else if (wikiText.at(i) == '}')
            {
                --bracketLevel;
            }
            else if (wikiText.at(i) == '\'' && bracketLevel == 0)
            {
                int n = 0;
                abstract = ParseParagraph(wikiText.mid(i, wikiText.indexOf('\n', i) - i), n);
                qDebug() << abstract;
                break;
            }

            ++i;
        }
    }

    QString ParseParagraph(QStringRef paragraph, int & i, int depth = 0)
    {
        QString result;

        while (i < paragraph.length())
        {
            QStringRef chars[4];
            for (int j = 0; j < 4; ++j)
            {
                chars[j]  = paragraph.mid(i, j + 1);
            }

            if  (chars[0] == ")"  && depth != 0)
            {
                i += 1;
                return QString();
            }
            else if (chars[1] == "}}"  && depth != 0)
            {
                i += 2;
                return QString();
            }
            else if (chars[0] == "(")
            {
                result += ParseRoundBrackets(paragraph, i, depth + 1);
                continue;
            }
            else if (chars[1] == "{{")
            {
                result += ParceBraces(paragraph, i, depth + 1);
                continue;
            }
            else if (chars[1] == "[[")
            {
                result +=ParseSquareBrackets(paragraph, i);
                continue;
            }
            else if (chars[3] == "&lt;")
            {
                result +=ParseWikiTag(paragraph, i);
                continue;
            }
            else if (chars[2] == "'''")
            {
                auto index = paragraph.indexOf('\'', i + 3);
                auto addition = paragraph.mid(i + 3,  index - i - 3).toString();
                i += addition.size() + 6;

                addition.remove(QChar(769));
                result += addition;

                continue;
            }
            else if (chars[1] == "''")
            {
                auto index = paragraph.indexOf('\'', i + 2);
                auto addition = paragraph.mid(i + 2, index - i - 2).toString();
                i += addition.size() + 4;

                addition.remove(QChar(769));
                result += addition;

                continue;
            }

            result.append(paragraph.at(i++));
        }

        return result;
    }

    QStringRef ParseRoundBrackets(QStringRef paragraph, int & i, int depth)
    {
        //i = paragraph.indexOf(')', i) + 1;
        ParseParagraph(paragraph, ++i, depth);
        return QStringRef();
    }

    QStringRef ParceBraces(QStringRef paragraph, int & i, int depth)
    {
        //i = paragraph.indexOf("}}", i) + 2;
        i += 2;
        ParseParagraph(paragraph, i, depth);
        return QStringRef();
    }

    QStringRef ParseSquareBrackets(QStringRef paragraph, int & i)
    {
        QStringRef str = paragraph.mid(i + 2, paragraph.indexOf(']', i) - i - 2);
        i += str.size() + 4;

        int mast =  str.indexOf('|');

        if (mast != -1)
        {
            QStringRef link = str.left(mast);
            return str.right(mast);
        }
        else
        {
            return str;
        }
    }

    QStringRef ParseWikiTag(QStringRef paragraph, int & i)
    {
        i = paragraph.indexOf("&gt;", i);

        i = paragraph.indexOf("&gt;", i);

        i+=4;

        return QStringRef();
    }

public:
    Database()
    {
        using namespace std::placeholders;

        ProcessSqlFile("./../wikidb/ruwiki-latest-page.sql", std::bind(&Database::InsertNewArticle, this, _1), std::bind(&Database::AddTitleToDict, this, _1, _2));
       // ProcessSqlFile("./../wikidb/ruwiki-latest-pagelinks.sql", std::bind(&Database::FindArticleById, this, _1), std::bind(&Database::AddLink, this, _1, _2));
        ExtractAbstracts("./../wikidb/ruwiki-latest-pages-articles1.xml");
    }
};

#endif // DATABASE_H
