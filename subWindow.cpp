#include "subWindow.h"
#include <QFile>
#include <QFileDialog>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QDialog>
#include "earth_map_demo.h"
subWindow::subWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
    QString qss;
    QFile file(":/qss/flatwhite.css");
    setStyleSheet("QToolButton{background-color:rgba(0,0,0,0);}QToolButton:hover{background-color:rgba(255,255,255,0.5);}QToolButton:pressed{background-color: rgba(100,100,100,1);}");
    if (file.open(QFile::ReadOnly))
    {
        //��readAll��ȡĬ��֧�ֵ���ANSI��ʽ,�����С����creator�򿪱༭���˺ܿ��ܴ򲻿�
        qss = QLatin1String(file.readAll());

        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }
    manager = new QNetworkAccessManager();
    connect(ui.toolButton, &QPushButton::clicked, this ,&subWindow::uploadImage);
    connect(ui.toolButton_2, &QPushButton::clicked, this, &subWindow::uploadImage_2);
    connect(ui.toolButton_3, &QPushButton::clicked, this, &subWindow::uploadImage_3);
    connect(ui.toolButton_4, &QPushButton::clicked, this, &subWindow::uploadImage_4);
    connect(ui.buttonFuse, &QPushButton::clicked, this, &subWindow::startnetRequest);
    connect(ui.buttonMatch, &QPushButton::clicked, this, &subWindow::startmatchRequest);
    ui.progressBar->setVisible(false);
    ui.progressBar->setMinimum(0);
    ui.progressBar->setMaximum(100);
    ui.progressBar->setValue(0);
    timer = new QTimer(this);
    
}

subWindow::~subWindow()
{}

void subWindow::upload(const QString& url, const QString& filepath)
{
    qDebug() << "[upload file]" << url << QFileInfo(filepath).fileName();
    QFile* file = new QFile(filepath);
    if (!file->open(QIODevice::ReadOnly)) {
        file->deleteLater();
        qDebug() << "open file error";
        return;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    //request.setRawHeader("Content-Type","multipart/form-data");

    //QHttpMultiPart��Ҫ��������ɺ��ͷ�
    QHttpMultiPart* multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart file_part;
    file_part.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"myfile\"; filename=\"%1\";")
            .arg(QFileInfo(filepath).fileName())));
    //part.header�������flask.request.files�����õ�form-data����Ϣ
    //ע�ⲻ��request��header
    file_part.setRawHeader("Content-Type", "multipart/form-data");
    file_part.setBodyDevice(file);
    file->setParent(multi_part);
    multi_part->append(file_part);

    QNetworkReply* reply = manager->post(request, multi_part);
    multi_part->setParent(reply); //��ɾ��replyʱһ���ͷ�

    //��Ϊ�ǲ�������ͬ���ȴ�
    QEventLoop eventLoop;
    //�ϴ�����
    connect(reply, &QNetworkReply::uploadProgress,
        this, [this](qint64 bytesSent, qint64 bytesTotal) {
            qDebug() << "[upload file] bytesSend" << bytesSent << "bytesTotal" << bytesTotal;
        });
    //�����˳��¼�ѭ��
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "reply" << status_code << QString(reply->readAll());
    qDebug() << "[upload file] finished";
}
void subWindow::uploadImage() {
    QString path = QFileDialog::getOpenFileName(this, "ѡ����Ƭ�ļ�", "", "imageFile(*.png *.jpg *.jpeg *.gif *.bmp *.tif)");
    upload("http://127.0.0.1:5000/upload", path);
    pathGround = QFileInfo(path).fileName();
    ui.toolButton->setIcon(QIcon(QPixmap(path)));
    ui.toolButton->setIconSize(ui.toolButton->size());
}
void subWindow::uploadImage_2() {
    QString path = QFileDialog::getOpenFileName(this, "ѡ����Ƭ�ļ�", "", "imageFile(*.png *.jpg *.jpeg *.gif *.bmp *.tif)");
    upload("http://127.0.0.1:5000/upload", path);
    pathIr = QFileInfo(path).fileName();
    ui.toolButton_2->setIcon(QIcon(QPixmap(path)));
    ui.toolButton_2->setIconSize(ui.toolButton->size());
}
void subWindow::uploadImage_3() {
    QString path = QFileDialog::getOpenFileName(this, "ѡ����Ƭ�ļ�", "", "imageFile(*.png *.jpg *.jpeg *.gif *.bmp *.tif)");
    upload("http://127.0.0.1:5000/upload", path);
    pathMs1 = QFileInfo(path).fileName();
    ui.toolButton_3->setIcon(QIcon(QPixmap(path)));
    ui.toolButton_3->setIconSize(ui.toolButton->size());
}
void subWindow::uploadImage_4() {
    QString path = QFileDialog::getOpenFileName(this, "ѡ����Ƭ�ļ�", "", "imageFile(*.png *.jpg *.jpeg *.gif *.bmp *.tif)");
    upload("http://127.0.0.1:5000/upload", path);
    pathMs2 = QFileInfo(path).fileName();
    ui.toolButton_4->setIcon(QIcon(QPixmap(path)));
    ui.toolButton_4->setIconSize(ui.toolButton->size());
}
void subWindow::uploadImageArg(QString path) {
    upload("http://127.0.0.1:5000/upload", path);
}
void subWindow::startnetRequest() {
    counter = 0;
    connect(timer, &QTimer::timeout, this, &subWindow::updateProgress);
    timer->start(100); // ��ʱ��ÿ100���봥��һ��
    QString methodFuse= ui.lineFuse->currentText();
    QStringList list;
    list << pathIr << pathGround<< pathMs1<< pathMs2;
    ui.progressBar->setVisible(true);
    QString apiUrl = "http://127.0.0.1:5000/predict"+ methodFuse;  // ����API��URL
    //QString apiUrl = "http://127.0.0.1:5000/predict";
    QUrl url(apiUrl);
    QUrlQuery query;

    for (const QString& fileName : list) {
        query.addQueryItem("fileName", fileName);
    }
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    replyfuse = manager->get(request);
    connect(replyfuse, &QNetworkReply::finished, this, &subWindow::replyFinished);
}
//������ͨ��
void subWindow::replyFinished()
{
    qDebug() << "Reply finished";
    // ��ȡhttp״̬��
    QVariant statusCode = replyfuse->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = replyfuse->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if (reason.isValid())
        qDebug() << "reason=" << reason.toString();
    QNetworkReply::NetworkError err = replyfuse->error();
    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << replyfuse->errorString();
    }
    
    else {
        // ��ȡ��������
        QByteArray responseData = replyfuse->readAll();
        QString responseString = QString::fromUtf8(responseData);
        responseString.remove(QChar('\"'));  // �Ƴ��ַ����е�˫����
        responseString.remove(QChar('\n'));  // �Ƴ��ַ����еĻ��з�
        qDebug() << "Response: " << "E:/pywork/modeldeploy/result/" + responseString;
        QPixmap* pixmap = new QPixmap("E:/pywork/modeldeploy/result/" + responseString );
        pixmap->scaled(ui.label_result->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui.label_result->setScaledContents(true);
        ui.label_result->setPixmap(*pixmap);
    }
    counter = 90;
    replyfuse->deleteLater();
    replyfuse = NULL;
    
}

void subWindow::updateProgress() {
        counter++;
        qDebug() << counter;
        ui.progressBar->setValue(counter);

        if (counter >= 100)
        {
            timer->stop();
            ui.progressBar->setVisible(false);
        }

}



void subWindow::startmatchRequest() {
    counter = 0;
    connect(timer, &QTimer::timeout, this, &subWindow::updateProgress);
    QString methodMatch = ui.lineMatch->currentText();
    if (methodMatch=="CDDFuse")timer->start(150); // ��ʱ��ÿ100���봥��һ��
    else timer->start(90); // ��ʱ��ÿ100���봥��һ��
    QStringList list;
    list << pathIr << pathGround << pathMs1 << pathMs2;
    ui.progressBar->setVisible(true);
    QString apiUrl = "http://127.0.0.1:5000/match"+ methodMatch;  // ����API��URL
    qDebug() << apiUrl;
    //QString apiUrl = "http://127.0.0.1:5000/match";
    QUrl url(apiUrl);
    QUrlQuery query;

    for (const QString& fileName : list) {
        query.addQueryItem("fileName", fileName);
    }
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    replymatch = manager->get(request);
    connect(replymatch, &QNetworkReply::finished, this, &subWindow::replymatchFinished);
}
//������ͨ��
void subWindow::replymatchFinished()
{
    qDebug() << "Reply finished";
    // ��ȡhttp״̬��
    QVariant statusCode = replymatch->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
        qDebug() << "status code=" << statusCode.toInt();

    QVariant reason = replymatch->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if (reason.isValid())
        qDebug() << "reason=" << reason.toString();
    QNetworkReply::NetworkError err = replymatch->error();
    if (err != QNetworkReply::NoError) {
        qDebug() << "Failed: " << replymatch->errorString();
    }

    else {
        // ��ȡ��������
        QByteArray responseData = replymatch->readAll();
        QString responseString = QString::fromUtf8(responseData);
        //qDebug() << "Response: " <<  responseString;
        QString base = "E:/pywork/modeldeploy/matchResult/";
        pathGround = base + "vis.jpg";
        qDebug() << pathGround;
        pathIr = base + "ir.jpg";
        pathMs1 = base + "ms1.jpg";
        pathMs2 = base + "ms2.jpg";
        ui.toolButton->setIcon(QPixmap(pathGround));
        ui.toolButton->setIconSize(ui.toolButton->size());
        ui.toolButton_2->setIcon(QPixmap(pathIr));
        ui.toolButton_2->setIconSize(ui.toolButton_2->size());
        ui.toolButton_3->setIcon(QPixmap(pathMs1));
        ui.toolButton_3->setIconSize(ui.toolButton_3->size());
        ui.toolButton_4->setIcon(QPixmap(pathMs2));
        ui.toolButton_4->setIconSize(ui.toolButton_4->size());
        pathGround =  "vis.jpg";
        pathIr =  "ir.jpg";
        pathMs1 =  "ms1.jpg";
        pathMs2 = "ms2.jpg";
    }
    counter = counter + 40;
    replymatch->deleteLater();
    replymatch = NULL;
}
//������ͨ��
//void subWindow::replymatchFinishedone()
//{
//    qDebug() << "Reply finished";
//    // ��ȡhttp״̬��
//    QVariant statusCode = replymatch->attribute(QNetworkRequest::HttpStatusCodeAttribute);
//    if (statusCode.isValid())
//        qDebug() << "status code=" << statusCode.toInt();
//
//    QVariant reason = replymatch->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
//    if (reason.isValid())
//        qDebug() << "reason=" << reason.toString();
//    QNetworkReply::NetworkError err = replymatch->error();
//    if (err != QNetworkReply::NoError) {
//        qDebug() << "Failed: " << replymatch->errorString();
//    }
//
//    else {
//        // ��ȡ��������
//        QByteArray responseData = replymatch->readAll();
//        QString responseString = QString::fromUtf8(responseData);
//        //qDebug() << "Response: " <<  responseString;
//    }
//    /*ImageDialog dialog("E:/pywork/modeldeploy/matchResult/match.jpg", "�����ģ̬");
//    dialog.exec();*/
//    counter = counter + 40;
//    replymatch->deleteLater();
//    replymatch = NULL;
//}
//void subWindow::startmatchRequestone(QString vispath,int k) {
//    counter = 0;
//    connect(timer, &QTimer::timeout, this, &subWindow::updateProgress);
//    QString methodMatch = ui.lineMatch->currentText();
//    if (methodMatch == "CDDFuse")timer->start(150); // ��ʱ��ÿ100���봥��һ��
//    else timer->start(90); // ��ʱ��ÿ100���봥��һ��
//    QStringList list;
//    pathGround = vispath;
//    list << pathIr << pathGround << pathMs1 << pathMs2;
//    ui.progressBar->setVisible(true);
//    QString apiUrl = "http://127.0.0.1:5000/matchLightglueone";  // ����API��URL
//    qDebug() << apiUrl;
//    //QString apiUrl = "http://127.0.0.1:5000/match";
//    QUrl url(apiUrl);
//    QUrlQuery query;
//
//    for (const QString& fileName : list) {
//        query.addQueryItem("fileName", fileName);  
//    }
//    QString kString = QString::number(k);
//    query.addQueryItem("xuhao", kString);
//    url.setQuery(query);
//    QNetworkRequest request(url);
//    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
//    replymatch = manager->get(request);
//    connect(replymatch, &QNetworkReply::finished, this, &subWindow::replymatchFinishedone);
//}
//void subWindow::setPath(QString a, QString b, QString c, QString d) {
//    pathGround = a;
//    pathIr = b;
//    pathMs1 = c;
//    pathMs2 = d;
//}