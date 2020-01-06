#include "searchcompleter.h"

SearchCompleter::SearchCompleter(QObject *parent)
	: QCompleter(parent)
{
}

SearchCompleter::~SearchCompleter()
{
}

QStringList SearchCompleter::splitPath(const QString& path) const
{

	return QStringList();
}

QString SearchCompleter::pathFromIndex(const QModelIndex& index) const
{
	return QString("");
}
