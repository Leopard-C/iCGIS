/****************************************************************
** class name:  PostgresqlConnect
**
** description: 连接PostGIS的对话框，用户输入数据库的信息
**				Ip、port、username、passwd、database等
**
** last change: 2020-01-09
**
** change: Save and read config file
***************************************************************/
#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QString>
#include <QVBoxLayout>


class PostgresqlConnect : public QDialog
{
	Q_OBJECT
public:
	PostgresqlConnect(QWidget *parent);
	~PostgresqlConnect();

public:
	void setupLayout();

signals:
	void btnConnectClicked(const QString& ip, int port, const QString& username,
		const QString& password, const QString& database);

public slots:
	void onConnectPostgresql();

protected:
	void closeEvent(QCloseEvent* ev);

private:
	// 读取、保存配置（用户名，数据库名，ip、port，不保存密码）
	void readConfig(const char* cfgpath);
	void writeConfig(const char* cfgpath);

private:
	QFrame* frame;

	// lineEdit
	// server host
	QLabel* labelServerHost;
	QLineEdit* lineEditServerIp;
	QLineEdit* lineEditServerPort;
	// username
	QLabel* labelUsername;
	QLineEdit* lineEditUserName;
	// passowrd	
	QLabel* labelPassword;
	QLineEdit* lineEditPassword;
	// database	
	QLabel* labelDatabase;
	QLineEdit* lineEditDatabase;

	// btn
	QPushButton* btnConnect;
	QPushButton* btnCancel;

	// spacer item
	QSpacerItem* horizontalSpacer_1;
	QSpacerItem* horizontalSpacer_2;
	QSpacerItem* horizontalSpacer_3;

	// layout
	QHBoxLayout* horizontalLayout_server;
	QHBoxLayout* horizontalLayout_btn;
	QVBoxLayout* verticalLayout;
	QFormLayout* formLayout;
};
