#pragma once

#include <QCompleter>
#include <QModelIndex>
#include <QStringList>
#include <QString>

class SearchCompleter : public QCompleter
{
	Q_OBJECT
public:
	SearchCompleter(QObject *parent);
	~SearchCompleter();

protected:
	virtual QStringList splitPath(const QString& path) const override;
	virtual QString pathFromIndex(const QModelIndex& index) const override;
};
