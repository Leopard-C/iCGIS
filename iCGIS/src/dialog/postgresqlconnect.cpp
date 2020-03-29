#include "dialog/postgresqlconnect.h"
#include "util/logger.h"

#include <QDebug>
#include <QMessageBox>

#include <fstream>

#include <jsoncpp/json/json.h>


PostgresqlConnect::PostgresqlConnect(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle(tr("PostgreSQL Connection"));
    this->setWindowIcon(QIcon("res/icons/postgresql.ico"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    setupLayout();
    connect(this->btnConnect, &QPushButton::clicked, this, &PostgresqlConnect::onConnectPostgresql);

    readConfig("userdata/config/postgis.json");
}

PostgresqlConnect::~PostgresqlConnect()
{
}

void PostgresqlConnect::setupLayout()
{
    // frame
    frame = new QFrame(this);
    frame->setFrameShape(QFrame::Panel);
    frame->setFrameShadow(QFrame::Raised);

    // label
    labelServerHost = new QLabel("ServerPort", frame);
    labelUsername = new QLabel("Username:", frame);
    labelPassword = new QLabel("Password", frame);
    labelDatabase = new QLabel("Database", frame);

    // lineEdit
    lineEditServerIp = new QLineEdit("localhost", frame);
    lineEditServerPort = new QLineEdit("5432", frame);
    lineEditUserName = new QLineEdit(frame);
    lineEditPassword = new QLineEdit(frame);
    lineEditPassword->setEchoMode(QLineEdit::Password);
    lineEditDatabase = new QLineEdit(frame);

    // pushButton
    btnConnect = new QPushButton("Connect");
    btnCancel = new QPushButton("Cancel");

    // spacer
    horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // horizontalLayout1 for server ip and port
    horizontalLayout_server = new QHBoxLayout();
    horizontalLayout_server->setSpacing(6);
    horizontalLayout_server->addWidget(lineEditServerIp);
    horizontalLayout_server->addWidget(lineEditServerPort);
    QSizePolicy sizePolicy_serverip(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy_serverip.setHorizontalStretch(3);
    lineEditServerIp->setSizePolicy(sizePolicy_serverip);
    QSizePolicy sizePolicy_serverport(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy_serverport.setHorizontalStretch(2);
    lineEditServerPort->setSizePolicy(sizePolicy_serverport);

    // horizontalLayout2 for btnConnect and btnCancel
    horizontalLayout_btn = new QHBoxLayout();
    horizontalLayout_btn->setSpacing(6);
    horizontalLayout_btn->addItem(horizontalSpacer_1);
    horizontalLayout_btn->addWidget(btnConnect);
    horizontalLayout_btn->addItem(horizontalSpacer_2);
    horizontalLayout_btn->addWidget(btnCancel);
    horizontalLayout_btn->addItem(horizontalSpacer_3);

    // form layout
    formLayout = new QFormLayout(frame);
    formLayout->setSpacing(6);
    formLayout->setContentsMargins(11, 11, 11, 11);
    formLayout->setWidget(0, QFormLayout::LabelRole, labelServerHost);
    formLayout->setLayout(0, QFormLayout::FieldRole, horizontalLayout_server);
    formLayout->setWidget(1, QFormLayout::LabelRole, labelUsername);
    formLayout->setWidget(1, QFormLayout::FieldRole, lineEditUserName);
    formLayout->setWidget(2, QFormLayout::LabelRole, labelPassword);
    formLayout->setWidget(2, QFormLayout::FieldRole, lineEditPassword);
    formLayout->setWidget(3, QFormLayout::LabelRole, labelDatabase);
    formLayout->setWidget(3, QFormLayout::FieldRole, lineEditDatabase);

    // vertical layout
    // main layout
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->addWidget(frame);
    verticalLayout->addLayout(horizontalLayout_btn);
}


void PostgresqlConnect::onConnectPostgresql()
{
    QString emptyItem;
    if (lineEditServerIp->text().isEmpty())
        emptyItem = "Server IP";
    else if (lineEditServerPort->text().isEmpty())
        emptyItem = "Server Port";
    else if (lineEditUserName->text().isEmpty())
        emptyItem = "User name";
    else if (lineEditPassword->text().isEmpty())
        emptyItem = "Password";
    else if (lineEditDatabase->text().isEmpty())
        emptyItem = "Database";

    if (!emptyItem.isNull()) {
        QString msg = "Please input " + emptyItem + "!";
        QMessageBox::critical(this, "Error", msg, QMessageBox::Ok);
        return;
    }

    QString ip = lineEditServerIp->text();
    int port = lineEditServerPort->text().toInt();
    QString username = lineEditUserName->text();
    QString password = lineEditPassword->text();
    QString database = lineEditDatabase->text();

    emit btnConnectClicked(ip, port, username, password, database);
    this->close();
}

void PostgresqlConnect::closeEvent(QCloseEvent* ev)
{
    writeConfig("userdata/config/postgis.json");
}

// Read last connection's info from file
void PostgresqlConnect::readConfig(const char* cfgpath)
{
    Json::Reader reader;
    Json::Value root;
    std::ifstream ifs;
    ifs.open(cfgpath);
    if (!ifs.is_open())
        return;

    if (!reader.parse(ifs, root, false)) {
        ifs.close();
        return;
    }

    if (root["username"].isNull() || root["ip"].isNull() ||
        root["port"].isNull() || root["database"].isNull())
    {
        ifs.close();
        return;
    }

    lineEditServerIp->setText(QString::fromStdString(root["ip"].asString()));
    lineEditServerPort->setText(QString::number(root["port"].asInt()));
    lineEditUserName->setText(QString::fromStdString(root["username"].asString()));
    lineEditDatabase->setText(QString::fromStdString(root["database"].asString()));

    ifs.close();
}

// Write connection info to file
void PostgresqlConnect::writeConfig(const char* cfgpath)
{
    Json::Value root;
    root["username"] = lineEditUserName->text().toStdString();
    root["ip"] = lineEditServerIp->text().toStdString();
    root["port"] = lineEditServerPort->text().toInt();
    root["database"] = lineEditDatabase->text().toStdString();

    Json::StyledWriter sw;
    std::ofstream ofs;
    ofs.open(cfgpath);
    ofs << sw.write(root);
    ofs.close();
}
