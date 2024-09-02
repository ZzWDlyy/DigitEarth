#pragma execution_character_set("utf-8")
#include "earth_map_demo.h"
#include <osgQOpenGL/osgQOpenGLWidget>
#include <osgEarth/MapNode>
#include <osgEarth/GDAL>
#include <osgEarth/ImageLayer>
#include <osgEarth/XYZ>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtCore/QString>
#include <osgEarth/OGRFeatureSource>
#include <osgEarth/FeatureModelLayer>
#include <osgEarth/FeatureImageLayer>
#include <osgEarth/Style>
#include "Location.h"
#include <osgEarth/PlaceNode>
#include <osgEarth/LabelNode>
#include <osgDB/ConvertUTF>
#include <osgEarth/Sky>
#include "MousePositionEvenHandler.h"
#include <QMouseEvent>
#include <QtWidgets/QApplication>
#include <QFile>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "CSVHandler.h"
#include <QStandardItemModel>
#include <QtWidgets/QStyledItemDelegate>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QDirIterator>
#include <QTimer>
#include "Registration.h"
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <QWheelEvent>
#include <osgEarth/FeatureNode>
#include <osgEarth/AnnotationLayer>
#include <osgEarth/GLUtils>
#include <ogrsf_frmts.h>
#include <QThread>
#include <osgEarth/GeoData>
#include <osgEarth/ImageOverlay>
#include <osgEarth/AnnotationLayer>
#include <QTextCodec>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrlQuery>
#include <osgEarth/Viewpoint>

osg::ref_ptr<osgEarth::XYZImageLayer> xyzLayer1 = new osgEarth::XYZImageLayer();
osg::ref_ptr<osgEarth::XYZImageLayer> xyzLayer2 = new osgEarth::XYZImageLayer();
std::vector<std::pair<std::string, osg::ref_ptr<osgEarth::GDALImageLayer>>>MapList;

earth_map_demo::earth_map_demo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //ui���
    this->setWindowTitle("���˻���ģ̬Ӱ��ϵͳ");
    //������ʽ��(Ĭ�ϱ�����ɫ����ͣʱ������ɫ������ʱ������ɫ)
    setStyleSheet("QToolButton{background-color:rgba(0,0,0,0);}QToolButton:hover{background-color:rgba(255,255,255,0.5);}QToolButton:pressed{background-color: rgba(100,100,100,1);}");
    setWindowFlags(Qt::FramelessWindowHint);    //���ر��������ޱ߿�

    //setWindowOpacity(0.7); //���ô���͸����
        //������ʽ��
    QString qss;
    QFile file(":/qss/flatwhite.css");

    if (file.open(QFile::ReadOnly))
    {
        //��readAll��ȡĬ��֧�ֵ���ANSI��ʽ,�����С����creator�򿪱༭���˺ܿ��ܴ򲻿�
       qss = QLatin1String(file.readAll());

       QString paletteColor = qss.mid(20, 7);
       qApp->setPalette(QPalette(QColor(paletteColor)));
       qApp->setStyleSheet(qss);
       file.close();
    }
    ui.tbn_min->setIconSize(QSize(35, 35));
    ui.tbn_close->setIconSize(QSize(35, 35));
    //setStyleSheet(".earth_map_demoClass{background - color: rgb(129, 129, 129);");
    osgEarth::initialize();
    widget = new osgQOpenGLWidget(this);
    Location location;
    countries = location.get_countries();
    //�����һ�ε�����
    addFirst(countries);
    connect(widget, &osgQOpenGLWidget::initialized, this, &earth_map_demo::onInitialized);
    connect(ui.add_Button, &QPushButton::clicked, this, &earth_map_demo::addMapLayer);
    connect(ui.countryBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &earth_map_demo::onCountryComboBoxIndexChanged);
    connect(ui.provinceCom, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &earth_map_demo::onProComboBoxIndexChanged);
    connect(ui.cityCom, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &earth_map_demo::onCityComboBoxIndexChanged);
    connect(ui.addPosition, &QPushButton::clicked, this, &earth_map_demo::createAddPositionDialog);
    connect(ui.pushButton, &QPushButton::clicked, this, &earth_map_demo::onViewChanged);
    connect(ui.collapse, &QToolButton::clicked, this, &earth_map_demo::onCollapseButtonClicked);
    connect(ui.startRegistration, &QToolButton::clicked, this, &earth_map_demo::startRegistration);
    connect(ui.tbn_close, &QPushButton::clicked, this, &earth_map_demo::closeW);
    connect(ui.replaceImage, &QPushButton::clicked, this, &earth_map_demo::replaceImage);
    connect(ui.clear, &QPushButton::clicked, this, &earth_map_demo::remove_layer);
    connect(ui.pb_ground, &QPushButton::clicked, this, &earth_map_demo::showImage1);
    connect(ui.pb_hongwai, &QPushButton::clicked, this, &earth_map_demo::showImage2);
    connect(ui.pb_duoguangpu, &QPushButton::clicked, this, &earth_map_demo::showImage3);
    connect(ui.closeRange, &QPushButton::clicked, this, &earth_map_demo::stopThead);
    connect(ui.removePosition, &QPushButton::clicked, [=]() {
        QModelIndex currentIndex = ui.listView->currentIndex();
        if (currentIndex.isValid()) {
            // ��ȡѡ���е��к�
            int row = currentIndex.row();
            CSVHandler csvHandler("./data/data.csv");
            // �� CSV �ļ����Ƴ���Ӧ��������
            csvHandler.removeRowFromCSV(row);
            initialList();
        }
        });
    connect(ui.changeTileMapButton, &QPushButton::clicked, this, &earth_map_demo::onchangeTileMapButtonClicked);
    connect(ui.home, &QPushButton::clicked, this, &earth_map_demo::toHome);
    //connect(ui.getRequest, &QPushButton::clicked, this, &earth_map_demo::startnetRequest);
    connect(ui.subWidget, &QPushButton::clicked, this, &earth_map_demo::showsubWidget);
    //connect(ui.matchAndFuse, &QPushButton::clicked, this, &earth_map_demo::matchAndFuse);
    connect(ui.tbn_max, &QPushButton::clicked, this, &earth_map_demo::ontbnmaxclicked);
    connect(ui.MapLayerList, &QListWidget::itemChanged, [=](QListWidgetItem* item) {
        int ChangeIndex = ui.MapLayerList->row(item);
        if (item->checkState() == Qt::Checked) {
            MapList[ChangeIndex].second->setOpacity(1);
        }
        else {
            MapList[ChangeIndex].second->setOpacity(0);
        }
        });
    QVBoxLayout* layout = new QVBoxLayout;
    initialList();
    layout->addWidget(widget);
    ui.frame->setLayout(layout);
    subW = new subWindow(this);
    subW->setGeometry(310, 100, 1540, 870);
    subW->setVisible(false);
    subW->setWindowFlag(Qt::FramelessWindowHint);
    ui.progressBar->setMinimum(0);
    ui.progressBar->setMaximum(100);
    ui.progressBar->setValue(0);
    timer = new QTimer(this);
    
}
void earth_map_demo::toHome() {
    ui.frame->setVisible(true);
    subW->setVisible(false);
    ui.frame->raise();
    osgEarth::Viewpoint vp("demo",
        0,  // ���㾭�� ����λ�Ƕȡ�
        0,  // ����ά�� ����λ�Ƕȡ�
        0,   // ���θ߶� ��λ�ס�
        0,   // Heading ���ָ�򽹵�Ƕȣ���λ�Ƕȡ�
        -90,   // pitch �����Խ��㸩���Ƕȣ���λ�Ƕȡ�
        2E7 // ���뽹����룬�����ʾ����ر�γ�ȵ�ľ��룬��λ�ס�
    );
    vp.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), 100.7, 3.1, 0, osgEarth::ALTMODE_ABSOLUTE);
    earthManipulator->setViewpoint(vp, 1.5);
    earthManipulator->setAutoComputeHomePosition(true);
}

earth_map_demo::~earth_map_demo()
{
   

}

void earth_map_demo::onInitialized()
{
    OGRRegisterAll();
    GDALAllRegister();
    CPLSetConfigOption("GDAL_DATA", "../../Data/gdal_data");
    CPLSetConfigOption("CPL_DEBUG", "YES");
    CPLSetConfigOption("CPL_LOG", "../LOG/gdal.log");
    mRoot = new osg::Group;
    
    //Map
    map = new osgEarth::Map();
    //XYZ
    std::string xyzFile1 = "E:/mapdata/all/all/{z}/{x}/{y}.png";
    xyzLayer1->setName("xyz");
    xyzLayer1->setURL(osgEarth::URI(xyzFile1));
    xyzLayer1->setProfile(osgEarth::Profile::create(osgEarth::Profile::SPHERICAL_MERCATOR)); 
    map->addLayer(xyzLayer1);
    //addnew
    std::string xyzFile2 = "E:/mapdata/newAd/fourth/{z}/{x}/{y}.png";
    xyzLayer2->setName("xyz");
    xyzLayer2->setURL(osgEarth::URI(xyzFile2));
    xyzLayer2->setProfile(osgEarth::Profile::create(osgEarth::Profile::SPHERICAL_MERCATOR));
    //map->addLayer(xyzLayer2);

    // ������Ч������ѡ��
    osgEarth::DateTime dateTime(2023, 12, 16, 19);
    osg::ref_ptr<osgEarth::SkyNode> sky_node = osgEarth::SkyNode::create();
    osgEarth::SkyOptions skyOptions;
    skyOptions.coordinateSystem() = osgEarth::SkyOptions::COORDSYS_ECI;
    skyOptions.ambient() = 0.4;
    sky_node->attach(widget->getOsgViewer(),0);
    sky_node->setDateTime(dateTime);
    sky_node->setName("sky");
    sky_node->getSunLight()->setAmbient(osg::Vec4(0.5, 0.5, 0.5, 1.0));
    //mRoot->addChild(sky_node);
    // ��ӿؼ�
    bottomStateLabel = new osgEarth::Util::Controls::LabelControl("state", osg::Vec4f(1, 1, 1, 1), 14);
    bottomStateLabel->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
    bottomStateLabel->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
    bottomStateLabel->setBackColor(0, 0, 0, 0.8);
    bottomStateLabel->setEncoding(osgText::String::ENCODING_UTF8);
    bottomStateLabel->setPadding(5);
    mRoot->addChild(osgEarth::Util::Controls::ControlCanvas::get(widget->getOsgViewer()));
    osgEarth::Util::Controls::ControlCanvas *canvas = osgEarth::Util::Controls::ControlCanvas::get(widget->getOsgViewer());
    canvas->addControl(bottomStateLabel);    
    
    mapNode = new osgEarth::MapNode(map.get());
    annoGroup = new osg::Group();
    mRoot->addChild(annoGroup);

    //geotiff
    //std::string tifFile = "./data/world.tif";
    //mRoot->addChild(earthTransform);
    mRoot->addChild(sky_node);
    //mRoot->addChild(mapNode);
    Cv::EventHandlers::MousePositionEvenHandler* labelHandler = new Cv::EventHandlers::MousePositionEvenHandler(mapNode, bottomStateLabel);
    widget->getOsgViewer()->addEventHandler(labelHandler);
    widget->setMouseTracking(true);

    mRoot->addChild(mapNode);
    

    annoLayer = new osgEarth::AnnotationLayer;
    map->addLayer(annoLayer);
    //mRoot->addChild(sky_node);
    if (mapNode == nullptr)
    {
        std::cout << "Failed to create MapNode." << std::endl;
        return;
    }
   
    ui.progressBar->setVisible(false);
    earthManipulator = new osgEarth::Util::EarthManipulator();
    osgEarth::GLUtils::setGlobalDefaults(widget->getOsgViewer()->getCamera()->getOrCreateStateSet());
    widget->getOsgViewer()->setCameraManipulator(earthManipulator);
    widget->getOsgViewer()->setSceneData(mRoot);
    osgEarth::Viewpoint vp("demo",
        0,  // ���㾭�� ����λ�Ƕȡ�
        0,  // ����ά�� ����λ�Ƕȡ�
        0,   // ���θ߶� ��λ�ס�
        0,   // Heading ���ָ�򽹵�Ƕȣ���λ�Ƕȡ�
        -90,   // pitch �����Խ��㸩���Ƕȣ���λ�Ƕȡ�
        2E7 // ���뽹����룬�����ʾ����ر�γ�ȵ�ľ��룬��λ�ס�
    );
    vp.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), 100.7, 3.1, 0, osgEarth::ALTMODE_ABSOLUTE);
    earthManipulator->setViewpoint(vp, 0);
    earthManipulator->setAutoComputeHomePosition(true);
    //onViewChanged();

    //startnetRequest(list);
    
}

void earth_map_demo::addMapLayer() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Image", "", "Image Files (*.tif *.jpg *.bmp)");
    // ��ȡ�ļ�����������·����
    QFileInfo fileInfo(fileName);
    QString selectedFileName = fileInfo.fileName();
    // ���δѡ���ļ�������
    if (fileName.isEmpty())
        return;

    else {
        //�����ļ�
        std::string addFile = fileName.toStdString();
        osgEarth::GDALImageLayer::Options gdalOpt;
        gdalOpt.url() = addFile;
        osg::ref_ptr<osgEarth::GDALImageLayer> imgLayer = new osgEarth::GDALImageLayer(gdalOpt);
        //��ͼ�������ͼ�������
        MapList.push_back(std::make_pair(selectedFileName.toStdString(), imgLayer));
        QListWidgetItem* ListItem = new QListWidgetItem(selectedFileName);
        QSize customSize(100, 50); 
        QFont q(QStringLiteral("HGHT_CNKI"), 18);
        // ���ø�ѡ���������С
        ListItem->setFont(q);
        ListItem->setSizeHint(customSize);
        ListItem->setFlags(ListItem->flags() | Qt::ItemIsUserCheckable);
        ListItem->setCheckState(Qt::Checked);
        ui.MapLayerList->addItem(ListItem);
        mapNode->getMap()->addLayer(imgLayer);
        widget->getOsgViewer()->setSceneData(mRoot);
    }
}






//���������
void earth_map_demo::addCountryline() {
    // shp layer
    osgEarth::OGRFeatureSource* features = new osgEarth::OGRFeatureSource();
    features->setURL("D:/OSGcore/build/OpenSceneGraphic/osgearth-3.2/data/world.shp");
    map->addLayer(features);
    if (!features) {
        return;
    }
    osgEarth::Style style;
    osgEarth::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::LineSymbol>();
    ls->stroke()->color() = osgEarth::Color::Yellow; //��ɫ
    ls->stroke()->width() = 4;// �߿�Ĭ�ϵ�λ������
    ls->tessellationSize()->set(10, osgEarth::Units::KILOMETERS);//Tessellate the line geometry such that no segment is longer than this value
    osgEarth::AltitudeSymbol* alt = style.getOrCreate<osgEarth::AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;// ��������
    alt->technique() = alt->TECHNIQUE_DRAPE;
    osgEarth::FeatureModelLayer* layer = new osgEarth::FeatureModelLayer();
    layer->setFeatureSource(features);
    osgEarth::StyleSheet* styleSheet = new osgEarth::StyleSheet();
    styleSheet->addStyle(style);
    layer->setStyleSheet(styleSheet);
    layer->getOrCreateStateSet()->addUniform(new osg::Uniform("oe_GL_LineStipplePattern", (int)0xffff), osg::StateAttribute::ON);
    
}

//��ӵ�������
void earth_map_demo::addFirst(std::vector<Country*> countries ) {
    
    for (const auto& country : countries) {
        QString countryName = QString::fromStdString(country->name);
        ui.countryBox->addItem(countryName);
    }

    if (!countries.empty()) {
        const auto& provinces = countries[0]->provinces;
        for (const auto& province : provinces) {
            QString provinceName = QString::fromStdString(province->name);
            ui.provinceCom->addItem(provinceName);
        }
    }

    if (!countries.empty()&& !countries[0]->provinces.empty()) {
        const auto& cities = countries[0]->provinces[0]->cities;
        for (const auto& city : cities) {
            QString cityName = QString::fromStdString(city->name);
            ui.cityCom->addItem(cityName);
        }
    }

    if (!countries.empty() && !countries[0]->provinces.empty() && !countries[0]->provinces[0]->cities.empty()) {
        const auto& districts = countries[0]->provinces[0]->cities[0]->districts;
        for (const auto& district : districts) {
            QString districtName = QString::fromStdString(district->name);
            ui.districtCom->addItem(districtName);
        }
    }
    
}
//��ӵ�������
void earth_map_demo::addFirstDialog(std::vector<Country*> countries, QComboBox &a, QComboBox &b, QComboBox &c, QComboBox &d) {

    for (const auto& country : countries) {
        QString countryName = QString::fromStdString(country->name);
        a.addItem(countryName);
    }

    if (!countries.empty()) {
        const auto& provinces = countries[0]->provinces;
        for (const auto& province : provinces) {
            QString provinceName = QString::fromStdString(province->name);
            b.addItem(provinceName);
        }
    }

    if (!countries.empty() && !countries[0]->provinces.empty()) {
        const auto& cities = countries[0]->provinces[0]->cities;
        for (const auto& city : cities) {
            QString cityName = QString::fromStdString(city->name);
            c.addItem(cityName);
        }
    }

    if (!countries.empty() && !countries[0]->provinces.empty() && !countries[0]->provinces[0]->cities.empty()) {
        const auto& districts = countries[0]->provinces[0]->cities[0]->districts;
        for (const auto& district : districts) {
            QString districtName = QString::fromStdString(district->name);
            d.addItem(districtName);
        }
    }

}
void earth_map_demo::onCountryComboBoxIndexChanged(int index) {
    
   
    Country* selectedCountry = countries[index];
    if (selectedCountry->provinces.empty())
    {
        ui.provinceCom->clear();
        ui.provinceCom->addItem("����");
    }
    // �����ѡʡ�ݵĳ������ݵ� cityComboBox
    else {
        ui.provinceCom->clear();
        for (const auto& province : selectedCountry->provinces) {
            QString provinceName = QString::fromUtf8(province->name.c_str());
            ui.provinceCom->addItem(provinceName);
        }
    }
}

void earth_map_demo::onProComboBoxIndexChanged(int index) { 
    
    
    int selectedCountryeIndex = ui.countryBox->currentIndex();
    Country* selectedCountry = countries[selectedCountryeIndex];
    if (selectedCountry->provinces.empty())
    {
        ui.cityCom->clear();
        ui.cityCom->addItem("����");
    }
    else {
        if (index == -1)index = 0;
        Province* selectedProvince = selectedCountry->provinces[index];
        if (selectedProvince->cities.empty())
        {
            ui.cityCom->clear(); ui.cityCom->addItem("����");
        }
        // �����ѡʡ�ݵĳ������ݵ� cityComboBox
        else {
            ui.cityCom->clear();
            for (const auto& city : selectedProvince->cities) {
                QString cityName = QString::fromUtf8(city->name.c_str());
                ui.cityCom->addItem(cityName);
            }
        }
    }
}
void earth_map_demo::onCityComboBoxIndexChanged(int index) {
    // ����ɵ���������
    ui.districtCom->clear();
    // ��ȡ��ǰѡ���ʡ�ݺͳ���
    int selectedCountryIndex = ui.countryBox->currentIndex();
    Country* selectedCountry = countries[selectedCountryIndex];
    if (selectedCountry->provinces.empty()) {
        ui.districtCom->addItem("����");
    }
    else {
        int selectedProvinceIndex = ui.provinceCom->currentIndex();
        if (selectedProvinceIndex == -1)selectedProvinceIndex = 0;
        Province* selectedProvince = selectedCountry->provinces[selectedProvinceIndex];
        if (selectedProvince->cities.empty()) {
            ui.districtCom->addItem("����");
        }
        else {
            if (index == -1) index = 0;
            City* selectedCity = selectedCountry->provinces[selectedProvinceIndex]->cities[index];
            if (selectedCity->districts.empty())ui.districtCom->addItem("����");
            else {
                // �����ѡ���е��������ݵ� districtComboBox
                for (const auto& district : selectedCity->districts) {
                    QString districtName = QString::fromUtf8(district->name.c_str());
                    ui.districtCom->addItem(districtName);
                }
            }
        }
    }
}



void earth_map_demo::onViewChanged() {
    int index0 = ui.countryBox->currentIndex();
    int index1 = ui.provinceCom->currentIndex();
    int index2 = ui.cityCom->currentIndex();
    int index3 = ui.districtCom->currentIndex();
    if (ui.districtCom->currentText() == "����") {
        QMessageBox::warning(this, "����", "���޸����ݣ�������ѡ��");
        return;
    }
    double log = countries[index0]->provinces[index1]->cities[index2]->districts[index3]->longitude;
    double lat = countries[index0]->provinces[index1]->cities[index2]->districts[index3]->latitude;
    //�����ӵ������
    osgEarth::Viewpoint vp("demo",
        0,  // ���㾭�� ����λ�Ƕȡ�
        0,  // ����ά�� ����λ�Ƕȡ�
        0,   // ���θ߶� ��λ�ס�
        0,   // Heading ���ָ�򽹵�Ƕȣ���λ�Ƕȡ�
        -90,   // pitch �����Խ��㸩���Ƕȣ���λ�Ƕȡ�
        1E5 // ���뽹����룬�����ʾ����ر�γ�ȵ�ľ��룬��λ�ס�
    );
    vp.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), log, lat, 0, osgEarth::ALTMODE_ABSOLUTE);
    earthManipulator->setViewpoint(vp,1);
    earthManipulator->setAutoComputeHomePosition(true);
    //��ת����ʾ�߽�
    osgEarth::OGRFeatureSource* features = new osgEarth::OGRFeatureSource();
    QString filename = ui.districtCom->currentText();
    std::string  filepath = "./data/bound/" + filename.toStdString() + "/"+filename.toStdString()+ "_Polygon.shp";
    features->setURL(filepath);
    map->addLayer(features);
    osgEarth::Style style;
    //����
    osgEarth::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::LineSymbol>();
    ls->stroke()->color() = osgEarth::Color::Red; //��ɫ
    ls->stroke()->width() = 3;// �߿�Ĭ�ϵ�λ������
    ls->tessellationSize()->set(10, osgEarth::Units::KILOMETERS);//Tessellate the line geometry such that no segment is longer than this value
    //����
    osgEarth::AltitudeSymbol* alt = style.getOrCreate<osgEarth::AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;// ��������
    alt->technique() = alt->TECHNIQUE_DRAPE;
    //��������
    osgEarth::PolygonSymbol* polygonSymbol = style.getOrCreateSymbol<osgEarth::PolygonSymbol>();
    polygonSymbol->fill()->color() = osgEarth::Color(255.0f / 255, 192.0f / 255, 203.0f / 255, 0.8f);
    polygonSymbol->outline() = true;
    if (featureValid)remove_layer();
    featurelayer = new osgEarth::FeatureImageLayer();
    featurelayer->setFeatureSource(features);
    featureValid = true;
    osgEarth::StyleSheet* styleSheet = new osgEarth::StyleSheet();
    styleSheet->addStyle(style);
    featurelayer->setStyleSheet(styleSheet);
    featurelayer->setOpacity(0.5);
    map->addLayer(featurelayer);
}

void earth_map_demo::remove_layer() {
    map->removeLayer(featurelayer);
    //featurelayer->setOpacity(0);
    featureValid = false;
}

//max-restore
void earth_map_demo::ontbnmaxclicked()
{
  if(!is_max){
      this->showFullScreen();
      QRect geo = ui.tbn_close->geometry();
      ui.tbn_close->move(geo.x() + 60, geo.y());
      geo = ui.tbn_min->geometry();
      ui.tbn_min->move(geo.x() + 60, geo.y());
      geo = ui.tbn_max->geometry();
      ui.tbn_max->move(geo.x() + 60, geo.y());
      ui.frame1->setGeometry(0,0,2000,100);
      ui.frame->setGeometry(300, -21, 1630, 1010);
      subW->setGeometry(310, 100, 1540, 1000);
      subW->ui.frame->setGeometry(0, 0, 1540, 1000);
      
  }
  else {
      //this->showMinimized();
      this->showNormal();
      QRect geo = ui.tbn_close->geometry();
      ui.tbn_close->move(geo.x() - 60, geo.y());
      geo = ui.tbn_min->geometry();
      ui.tbn_min->move(geo.x() - 60, geo.y());
      geo = ui.tbn_max->geometry();
      ui.tbn_max->move(geo.x() - 60, geo.y());
      ui.frame->setGeometry(300, -21, 1560, 905);
      subW->setGeometry(310, 100, 1540, 870);
      //subW->ui.frame->setGeometry(0, 0, 1540, 875);
  }
  is_max = !is_max;
}

//�����������ѡ��
void earth_map_demo::onCollapseButtonClicked()
{
    if (!isCollapse) {
        // �۵�
        isCollapse = !isCollapse;
        ui.tabWidget->move(- ui.tabWidget->width(), 80); // �滻Ϊ��Ҫ�۵����صĿؼ�
        // �����۵�����...
        ui.frame->setGeometry(-15,-10,1877,895);
        subW->setGeometry(60, 100, 1800, 1000);
        subW->ui.frame->setGeometry(0, 0, 1800, 1000);
    }
    else {
        // չ��
        isCollapse = !isCollapse;
        ui.tabWidget->move(0, 0);// �滻Ϊ��Ҫ�۵����صĿؼ�
        // ����չ������...
        ui.tabWidget->show();
        ui.frame->setGeometry(300, -10, 1650, 900);
        //ui.frame_3->setGeometry(-5,95, 2100, 895);
        subW->setGeometry(310, 100, 1540, 1000);
        subW->ui.frame->setGeometry(0, 0, 1540, 1000);
    }
}

void earth_map_demo::createAddPositionDialog()
{
    CSVHandler csvHandler("./data/data.csv");
    if (!csvHandler.fileExists()) {
        csvHandler.createFile();
    }
    // �����Ի���
    QDialog dialog(this);
    dialog.setWindowTitle("���λ��");
    QFormLayout formLayout(&dialog);
    QComboBox countryComboBox;
    formLayout.addRow("����:", &countryComboBox);
    QComboBox provinceComboBox;
    formLayout.addRow("ʡ��:", &provinceComboBox);
    QComboBox cityComboBox;
    formLayout.addRow("��:", &cityComboBox);
    QComboBox countyComboBox;
    formLayout.addRow("��(��):", &countyComboBox);
    QPushButton customLocationButton("�Զ���λ��");
    formLayout.addRow("", &customLocationButton);
    connect(&customLocationButton, &QPushButton::clicked, [&]() {
        QDialog customDialog(&dialog);
        customDialog.setWindowTitle("�Զ���λ��");
        QFormLayout customFormLayout(&customDialog);
        QLineEdit locationNameLineEdit;
        customFormLayout.addRow("����:", &locationNameLineEdit);
        QLineEdit longitudeLineEdit;
        customFormLayout.addRow("����:", &longitudeLineEdit);
        QLineEdit latitudeLineEdit;
        customFormLayout.addRow("γ��:", &latitudeLineEdit);
        QDialogButtonBox customButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        customFormLayout.addRow(&customButtonBox);

        //
        connect(customButtonBox.button(QDialogButtonBox::Ok), &QPushButton::clicked, [&]() {
            QString locationName = locationNameLineEdit.text();
            QString longitude = longitudeLineEdit.text();
            QString latitude = latitudeLineEdit.text();

            // �����ݲ�д��.csv��
            std::vector<std::string> line;
            line.push_back("false"); line.push_back(locationName.toStdString());
            line.push_back(latitude.toStdString()); line.push_back(longitude.toStdString());
            std::vector< std::vector<std::string>>data;
            data.push_back(line);
            csvHandler.writeCSV(data);
            initialList();
            customDialog.close();
            dialog.close();
            });

        // ����ȡ����ť�ĵ���¼�
        connect(customButtonBox.button(QDialogButtonBox::Cancel), &QPushButton::clicked, [&]() {
            customDialog.reject(); // �ر��Զ���λ�öԻ���
            });

        // ��ʾ�Զ���λ�öԻ���
        if (customDialog.exec() == QDialog::Accepted) {
            // �Զ���λ�öԻ���ȷ�ϣ�ִ����Ӧ����
           
        }
        else {
            // �Զ���λ�öԻ���ȡ����ر�
        }
        });

    addFirstDialog(countries, countryComboBox, provinceComboBox, cityComboBox, countyComboBox);
    // �����������index�仯�ź�
    connect(&countryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
        // ������ұ仯
        Country* selectedCountry = countries[index];
        if (selectedCountry->provinces.empty())
        {
            provinceComboBox.clear();
            provinceComboBox.addItem("����");
        }
        // �����ѡʡ�ݵĳ������ݵ� cityComboBox
        else {
            provinceComboBox.clear();
            for (const auto& province : selectedCountry->provinces) {
                QString provinceName = QString::fromUtf8(province->name.c_str());
                provinceComboBox.addItem(provinceName);
            }
        }
        });
    connect(&provinceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
        // ����ʡ�ݱ仯
        int selectedCountryeIndex = countryComboBox.currentIndex();
        Country* selectedCountry = countries[selectedCountryeIndex];
        if (selectedCountry->provinces.empty())
        {
            cityComboBox.clear();
            cityComboBox.addItem("����");
        }
        else {
            if (index == -1)index = 0;
            Province* selectedProvince = selectedCountry->provinces[index];
            if (selectedProvince->cities.empty())
            {
                cityComboBox.clear(); cityComboBox.addItem("����");
            }
            // �����ѡʡ�ݵĳ������ݵ� cityComboBox
            else {
                cityComboBox.clear();
                for (const auto& city : selectedProvince->cities) {
                    QString cityName = QString::fromUtf8(city->name.c_str());
                    cityComboBox.addItem(cityName);
                }
            }
        }
        });
    connect(&cityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
        // �����б仯
        // ����ɵ���������
        countyComboBox.clear();
        // ��ȡ��ǰѡ���ʡ�ݺͳ���
        int selectedCountryIndex = countryComboBox.currentIndex();
        Country* selectedCountry = countries[selectedCountryIndex];
        if (selectedCountry->provinces.empty()) {
            countyComboBox.addItem("����");
        }
        else {
            int selectedProvinceIndex = provinceComboBox.currentIndex();
            if (selectedProvinceIndex == -1)selectedProvinceIndex = 0;
            Province* selectedProvince = selectedCountry->provinces[selectedProvinceIndex];
            if (selectedProvince->cities.empty()) {
                countyComboBox.addItem("����");
            }
            else {
                if (index == -1) index = 0;
                City* selectedCity = selectedCountry->provinces[selectedProvinceIndex]->cities[index];
                if (selectedCity->districts.empty())ui.districtCom->addItem("����");
                else {
                    // �����ѡ���е��������ݵ� districtComboBox
                    for (const auto& district : selectedCity->districts) {
                        QString districtName = QString::fromUtf8(district->name.c_str());
                        countyComboBox.addItem(districtName);
                    }
                }
            }
        }
        });
   
    // ��ӶԻ����ȷ�Ϻ�ȡ����ť
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout.addRow(&buttonBox);

    // ����ȷ�ϰ�ť�ĵ���¼�
    connect(buttonBox.button(QDialogButtonBox::Ok), &QPushButton::clicked, [&]() {
        int countryIndex = countryComboBox.currentIndex();
        int provinceIndex = provinceComboBox.currentIndex();
        int cityIndex = cityComboBox.currentIndex();
        int countyIndex = countyComboBox.currentIndex();
        QString county = countyComboBox.currentText();
        if (county == "����") {
            // �����Ի�����ʾ���޸����ݣ�������ѡ��
            QMessageBox::warning(&dialog, "����", "���޸����ݣ�������ѡ��");
            return;
        }
        double lat = countries[countryIndex]->provinces[provinceIndex]->cities[cityIndex]->districts[countyIndex]->latitude;
        double log = countries[countryIndex]->provinces[provinceIndex]->cities[cityIndex]->districts[countyIndex]->longitude;
        // �����ȡ����λ������
        std::vector<std::string> line;
        line.push_back("true"); line.push_back(county.toStdString());
        line.push_back(std::to_string(lat)); line.push_back(std::to_string(log));
        std::vector<std::vector<std::string>>data;
        data.push_back(line);
        csvHandler.writeCSV(data);
       /* QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(ui.listView->model());
        if (model) {
            QStringList rowData;
            rowData << county;
            model->appendRow(new QStandardItem(rowData.join(",")));
        }*/
        initialList();
        dialog.accept(); // �رնԻ���
        });

    // ����ȡ����ť�ĵ���¼�
    connect(buttonBox.button(QDialogButtonBox::Cancel), &QPushButton::clicked, [&]() {
        dialog.reject(); // �رնԻ���
        });

    // ��ʾ�Ի���
    if (dialog.exec() == QDialog::Accepted) {
        // �Ի���ȷ�ϣ�ִ����Ӧ����
    }
    else {
        // �Ի���ȡ����ر�
    }
}

void earth_map_demo::initialList() {
    // ���λ�����ݺ��б���ͼ��ģ��
    locationData.clear();
    if (itemModel != nullptr) {
        delete itemModel;
    }
    CSVHandler csvHandler("./data/data.csv");
    // ��� QListView �е�������
    if (ui.listView->model() != nullptr) {
        QAbstractItemModel* model = ui.listView->model();
        model->removeRows(0, model->rowCount());
    }
    // �� CSV �ļ���ȡ����
    std::vector<std::vector<std::string>> data = csvHandler.readCSV();
    // �����Զ���ģ�����洢λ���б�
    itemModel = new QStandardItemModel();
    ui.listView->setModel(itemModel);

    // �������ݣ�Ϊÿ��λ�ô������ӵ�ģ����
    for (const auto& row : data) {
        if (row.size() >= 4) {
            std::string iscustom = row[0];
            std::string locationName = row[1];
            std::string latitude = row[2];
            std::string longitude = row[3];

            // �����Զ���λ�ö���
            PLocation location;
            location.iscustom = iscustom;
            location.locationName = locationName;
            location.latitude = latitude;
            location.longitude = longitude;

            // ��λ�ö�����ӵ�������
            locationData.push_back(location);

            // ����������ı�Ϊλ������
            QStandardItem* item = new QStandardItem(QString::fromStdString(locationName));
            itemModel->appendRow(item);

            // ʹ��ί����������ĵ���¼��������
            ui.listView->setItemDelegateForRow(itemModel->rowCount() - 1, new QStyledItemDelegate(ui.listView));
            connect(ui.listView, &QListView::clicked, [=](const QModelIndex& index) {
                if (index.isValid()) {
                    // ��ȡ��Ӧλ�ö��������
                    int locationIndex = index.row();
                    if (locationIndex >= 0 && locationIndex < locationData.size()) {
                        const PLocation& clickedLocation = locationData[locationIndex];
                        // �������¼���ʹ�ö�Ӧ��λ�ö���
                        onViewChanged_2(clickedLocation.iscustom, clickedLocation.locationName, std::stod(clickedLocation.latitude), std::stod(clickedLocation.longitude));
                    }
                }
                });
        }
    }
}

void earth_map_demo::onViewChanged_2(std::string custom ,std::string name, double lat1, double log1) {
    double log = log1;
    double lat = lat1;
    //�����ӵ������
    osgEarth::Viewpoint vp("demo",
        0,  // ���㾭�� ����λ�Ƕȡ�
        0,  // ����ά�� ����λ�Ƕȡ�
        0,   // ���θ߶� ��λ�ס�
        0,   // Heading ���ָ�򽹵�Ƕȣ���λ�Ƕȡ�
        -90,   // pitch �����Խ��㸩���Ƕȣ���λ�Ƕȡ�
        1E5 // ���뽹����룬�����ʾ����ر�γ�ȵ�ľ��룬��λ�ס�
    );
    vp.focalPoint()->set(osgEarth::SpatialReference::get("wgs84"), log, lat, 0, osgEarth::ALTMODE_ABSOLUTE);
    earthManipulator->setViewpoint(vp, 1);
    earthManipulator->setAutoComputeHomePosition(true);
    if (custom == "true") {
        //��ת����ʾ�߽�
        osgEarth::OGRFeatureSource* features = new osgEarth::OGRFeatureSource();
        std::string filename = name;
        std::string  filepath = "./data/bound/" + filename + "/" + filename + "_Polygon.shp";
        features->setURL(filepath);
        map->addLayer(features);
        osgEarth::Style style;
        //����
        osgEarth::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::LineSymbol>();
        ls->stroke()->color() = osgEarth::Color::Red; //��ɫ
        ls->stroke()->width() = 3;// �߿�Ĭ�ϵ�λ������
        ls->tessellationSize()->set(10, osgEarth::Units::KILOMETERS);//Tessellate the line geometry such that no segment is longer than this value
        //����
        osgEarth::AltitudeSymbol* alt = style.getOrCreate<osgEarth::AltitudeSymbol>();
        alt->clamping() = alt->CLAMP_TO_TERRAIN;// ��������
        alt->technique() = alt->TECHNIQUE_DRAPE;
        //��������
        osgEarth::PolygonSymbol* polygonSymbol = style.getOrCreateSymbol<osgEarth::PolygonSymbol>();
        polygonSymbol->fill()->color() = osgEarth::Color(255.0f / 255, 192.0f / 255, 203.0f / 255, 0.8f);
        polygonSymbol->outline() = true;
        if (featureValid)remove_layer();
        featurelayer = new osgEarth::FeatureImageLayer();
        featurelayer->setFeatureSource(features);
        featureValid = true;
        osgEarth::StyleSheet* styleSheet = new osgEarth::StyleSheet();
        styleSheet->addStyle(style);
        featurelayer->setStyleSheet(styleSheet);
        featurelayer->setOpacity(0.5);
        map->addLayer(featurelayer);
    }
}

void earth_map_demo::onchangeTileMapButtonClicked() {


    QString folderPath = QFileDialog::getExistingDirectory(this, "ѡ���ļ���", "");

    if (!folderPath.isEmpty()) {
        // ��ʾͼƬ��ʽѡ��Ի���
        QStringList formatOptions;
        formatOptions << "png" << "jpg" << "jpeg"; // �������Ҫ��ͼƬ��ʽѡ��

        bool ok;
        QString selectedFormat = QInputDialog::getItem(this, "ѡ��ͼƬ��ʽ", "ͼƬ��ʽ:", formatOptions, 0, false, &ok);

        if (ok && !selectedFormat.isEmpty()) {
            // ����ѡ���ͼƬ��ʽ���� xyzLayer1
            mapNode->getMap()->removeLayer(xyzLayer1);
            std::string newXYZFile = folderPath.toStdString() + "/{z}/{x}/{y}." + selectedFormat.toStdString();
            xyzLayer1 = new osgEarth::XYZImageLayer();
            //*std::string xyzFile = "F:/ArcGIS/data1/{z}/{x}/{y}.jpg";*/
            std::string xyzFile = newXYZFile;
            xyzLayer1->setURL(osgEarth::URI(newXYZFile));
            xyzLayer1->setProfile(osgEarth::Profile::create(osgEarth::Profile::SPHERICAL_MERCATOR));
            mapNode->getMap()->removeLayer(xyzLayer1);
            mapNode->getMap()->addLayer(xyzLayer1);
            widget->getOsgViewer()->setSceneData(mRoot);
            // ִ�н������ĺ���
        }
        else {
            // �û�ȡ����ѡ��ͼƬ��ʽ
            QMessageBox::warning(this, "����", "��û��ѡ��ͼƬ��ʽ��");
        }
    }
}

void earth_map_demo::startRegistration() {
    if (calculationThread!=nullptr) {
        calculationThread->finalstop();
        calculationThread->wait();
        delete calculationThread;
        calculationThread = nullptr;
    }
    ui.add_Button->setEnabled(false);
    ui.startRegistration->setEnabled(false);
    ui.changeTileMapButton->setEnabled(false);
    ui.closeRange->setEnabled(false);
    //�ͷ��ڴ�
    //calculationThread->stop();
    currentIndex = 0;
    //����ϴ���׼������
    fileNames.clear();
    pts.clear();
    center.clear();
    //
    for (auto node : featurenode) {
        annoGroup->removeChild(node);
    }
    for (auto node : placenode) {
        annoGroup->removeChild(node);
    }
    for (auto node : overlaynode) {
        mRoot->removeChild(node);
    }
    featurenode.clear();
    placenode.clear();
    ui.tabWidget->setCurrentIndex(1);
    // �����Ի���ѡ�����׼ͼƬtif�ļ�
    //��ʼ����׼
    p = new Registration();
    QString tifFile = QFileDialog::getOpenFileName(nullptr, "ѡ�����׼ͼƬ", "", "TIFF�ļ� (*.tif)");
    if (tifFile.isEmpty()) {
        qDebug() << "δѡ���κ��ļ�";
        return;
    }
    //��ȡ��ͼ
    filepath = tifFile.toStdString();
    p->readBigMap(tifFile.toStdString());

    osgEarth::GDALImageLayer::Options gdalOpt;
    gdalOpt.url() = tifFile.toStdString();
    osg::ref_ptr<osgEarth::GDALImageLayer> imgLayer = new osgEarth::GDALImageLayer(gdalOpt);
    mapNode->getMap()->addLayer(imgLayer);
    widget->getOsgViewer()->setSceneData(mRoot);
    QFileDialog dialog(this, "ѡ��Ŀ¼");
    dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowStaysOnTopHint);
    // �����Ի���ѡ��Ŀ¼
    QString directory = dialog.getExistingDirectory(this, "ѡ��Ŀ¼");
    if (directory.isEmpty()) {
        qDebug() << "δѡ���κ�Ŀ¼";
        return;
    }

   // ����Ŀ¼�µ��ļ��������ļ�·��
    QDirIterator it(directory, QStringList() << "*.jpg" << "*.png" <<"*tif", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        fileNames.append(filePath);
    }
    // ������ʱ����ÿ��һ��ʱ�䴥��һ��
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &earth_map_demo::displayNextImage);
    // ���ö�ʱ���������ʱ�䣨���룩
    int interval = 1000; // ÿ1����ʾһ��ͼƬ
    timer->setInterval(interval);
    // ������ʱ��
    timer->start();

    is_Stop = false;  // ����Ϊ false ��ʾ���¿�ʼ
    QString newImagePath = ":/icon_3axusv3ezyt/closeRange.png";
    ui.label_10->setText("�رշ�Χ");

    QString currentStyleSheet = ui.closeRange->styleSheet();
// �滻��ʽ���еı���ͼ��·��
    QString newStyleSheet = currentStyleSheet.replace(
    QRegExp("url\\(.*\\)"), QString("url(%1)").arg(newImagePath)
);
// ���°�ť�ı�����ʽ��
        ui.closeRange->setStyleSheet(newStyleSheet);
    
}

void earth_map_demo::displayNextImage() {

    if (currentIndex != 0 ) {
        featurenode[currentIndex - 1]->setNodeMask(0);
        //mRoot->removeChild(placenode[currentIndex - 1]);
    }
    if (currentIndex >= fileNames.size()) {
        // ����ͼƬ����ʾ��ϣ�ֹͣ��ʱ��
        
        QTimer* timer = qobject_cast<QTimer*>(sender());
        if (timer) {
            timer->stop();
            timer->deleteLater();
            //������̨����
            ui.add_Button->setEnabled(true);
            ui.startRegistration->setEnabled(true);
            ui.changeTileMapButton->setEnabled(true);
            ui.closeRange->setEnabled(true);
            osgEarth::Viewpoint vp = earthManipulator->getViewpoint();
            osgEarth::optional<osgEarth::GeoPoint>result = vp.focalPoint();
            osgEarth::GeoPoint gp = result.value();
            calculationThread = new CalculationThread(gp.x(),gp.y(),center, this);
            connect(calculationThread, &CalculationThread::calculationResult, this, &earth_map_demo::handleCalculationResult);
            calculationThread->start();
        }
        return;
    }
    // Get the current image file name
    QString imagePath = fileNames.at(currentIndex);
    // Load and display the image in QLabel
    QPixmap image(imagePath);
    QSize labelSize = ui.label_4->size();
    QPixmap scaledImage = image.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui.label_4->setPixmap(scaledImage);

    //�������ʸ��������ÿ��ͼƬ�����ľ��롢�Լ���Ӧʸ����ָ��
    center.push_back(p->readMap(imagePath.toStdString()));
    PixelCoordinate pix = p->convertToPixelCoordinate();
    //�����ͼ

    ImageCoordinate coordinate = p->registerImages(filepath, pix.x, pix.y);
    //ImageCoordinate coordinate = p->registerImages("F:/source/repos/earth_map_demo/earth_map_demo/data/xidian1.tif", pix.x, pix.y);
    pts.push_back(coordinate);
    osgEarth::Geometry* geom = new osgEarth::Polygon();
    geom->push_back(osg::Vec3d(coordinate.result[0].longitude, coordinate.result[0].latitude, 0));
    geom->push_back(osg::Vec3d(coordinate.result[1].longitude, coordinate.result[1].latitude, 0));
    geom->push_back(osg::Vec3d(coordinate.result[2].longitude, coordinate.result[2].latitude, 0));
    geom->push_back(osg::Vec3d(coordinate.result[3].longitude, coordinate.result[3].latitude, 0));

    osgEarth::Feature* feature = new osgEarth::Feature(geom, osgEarth::SpatialReference::get("wgs84"));
    feature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

    osgEarth::Style geomStyle;
    geomStyle.getOrCreate<osgEarth::RenderSymbol>()->depthOffset()->enabled() = true;
    geomStyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->color() = osgEarth::Color::Red;
    geomStyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->width() = 7;
    geomStyle.getOrCreate<osgEarth::PolygonSymbol>()->fill()->color() = osgEarth::Color(osgEarth::Color::White, 0.2);
    geomStyle.getOrCreate<osgEarth::PolygonSymbol>()->outline() = true;
    geomStyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->smooth() = true;
    geomStyle.getOrCreate<osgEarth::AltitudeSymbol>()->clamping() = osgEarth::AltitudeSymbol::CLAMP_TO_TERRAIN;
    geomStyle.getOrCreate<osgEarth::AltitudeSymbol>()->technique() = osgEarth::AltitudeSymbol::TECHNIQUE_DRAPE;

    osgEarth::FeatureNode* fnode = new osgEarth::FeatureNode(feature, geomStyle);
    featurenode.push_back(fnode);
    annoGroup->addChild(fnode);
    //�ر�
    osgEarth::Style pm;
    osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile("./icon_3axusv3ezyt/weizhi.png");
    osgEarth::PlaceNode* pNode = new osgEarth::PlaceNode(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"),
        center[currentIndex].x, center[currentIndex].y, 20), "", pm, pImage);
    placenode.push_back(pNode);
    is_replace.push_back(false);
    annoGroup->addChild(pNode);
    currentIndex++;
 }
    
void earth_map_demo::wheelEvent(QWheelEvent* event)    // �����¼�
{
   /* double sheer = earthManipulator->getDistance();
    osgEarth::Viewpoint vp = earthManipulator->getViewpoint();
    osgEarth::optional<osgEarth::GeoPoint>result = vp.focalPoint();
    osgEarth::GeoPoint gp= result.value();
    gp.makeGeographic();
    if (sheer <= 2500) {
    
    }*/
}
void earth_map_demo::handleCalculationResult(int bestIndex) {
    currentIndex = bestIndex;
    osgEarth::Viewpoint vp = earthManipulator->getViewpoint();
    double sheer = earthManipulator->getDistance();
    osgEarth::optional<osgEarth::GeoPoint>result = vp.focalPoint();
    osgEarth::GeoPoint gp = result.value();
    calculationThread->set(gp.x(), gp.y());
    // Get the current image file name
    QString imagePath = fileNames.at(currentIndex);
    // Load and display the image in QLabel
    QPixmap image(imagePath);
    QSize labelSize = ui.label_4->size();
    QPixmap scaledImage = image.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui.label_4->setPixmap(scaledImage);
    //ui.label_9->setText(QString::number(featurenode[currentIndex]->referenceCount()));
    if (sheer <= 2500) {
        if (lastIndex != currentIndex) 
        { 
            if(!is_replace[currentIndex])featurenode[currentIndex]->setNodeMask(1);//�������ڵ�
            if (lastIndex != -1)
                featurenode[lastIndex]->setNodeMask(0);//���
            lastIndex = currentIndex;
        }
    }
    
}
void earth_map_demo::closeW() {
    if (calculationThread != nullptr) {
        calculationThread->finalstop();
        calculationThread->wait();
    }
    this->close();
}

void earth_map_demo::replaceImage() {
    QString imagePath = fileNames.at(currentIndex);
    osgEarth::ImageOverlay* imageOverlay = new osgEarth::ImageOverlay(mapNode,
        osgDB::readImageFile(imagePath.toStdString())
    );
    ImageCoordinate coordinate = pts[currentIndex];
    imageOverlay->setCorners(osg::Vec2d(coordinate.result[1].longitude, coordinate.result[1].latitude), osg::Vec2d(coordinate.result[2].longitude, coordinate.result[2].latitude),
        osg::Vec2d(coordinate.result[0].longitude, coordinate.result[0].latitude), osg::Vec2d(coordinate.result[3].longitude, coordinate.result[3].latitude));
    mRoot->insertChild(0, imageOverlay);
    is_replace[currentIndex] = true;
    placenode[currentIndex]->setNodeMask(0);
    featurenode[currentIndex]->setNodeMask(0);
    overlaynode.push_back(imageOverlay);
}
void earth_map_demo::replaceFuseImage() {
    QString imagePath = "E:/pywork/modeldeploy/result/cddfuse.png";
    osgEarth::ImageOverlay* imageOverlay = new osgEarth::ImageOverlay(mapNode,
        osgDB::readImageFile(imagePath.toStdString())
    );
    ImageCoordinate coordinate = pts[currentIndex];
    imageOverlay->setCorners(osg::Vec2d(coordinate.result[1].longitude, coordinate.result[1].latitude), osg::Vec2d(coordinate.result[2].longitude, coordinate.result[2].latitude),
        osg::Vec2d(coordinate.result[0].longitude, coordinate.result[0].latitude), osg::Vec2d(coordinate.result[3].longitude, coordinate.result[3].latitude));
    mRoot->insertChild(0, imageOverlay);
    is_replace[currentIndex] = true;
    placenode[currentIndex]->setNodeMask(0);
    featurenode[currentIndex]->setNodeMask(0);
    overlaynode.push_back(imageOverlay);
}
void earth_map_demo::showImage1() {
    // ������Ϣ��
    QString imagePath = fileNames.at(currentIndex);
    ImageDialog dialog(imagePath,"��ѧģ̬");
    dialog.exec();
}
//������ϴ��ļ�����ֱ�Ӷ�ȡ�����ļ�����ͼƬ�ŵ����������ϴ��ļ���
void earth_map_demo::showImage2() {
    // ������Ϣ��
    QString fileName = fileNames.at(currentIndex);
    QString filepath(fileName);
    QString infraredImagePath = fileName.replace("ground truth", "ir");
    ImageDialog dialog(infraredImagePath, "����ģ̬");
    dialog.exec();
    //subW->uploadImageArg(infraredImagePath);//�ϴ��ļ����������У�ע���ļ�����Ҫ�ظ����Ḳ�Ǳ���ļ���
    /*QFileInfo infraredFileInfo(infraredImagePath);
    QString infraredBaseName = infraredFileInfo.fileName();
    subW->pathIr = "ir"+infraredBaseName;
    subW->startmatchRequestone(filepath,0);*/
    /*ImageDialog dialog("E:/pywork/modeldeploy/matchResult/match.jpg", "����ģ̬");
    dialog.exec();*/
}

void earth_map_demo::showImage3() {
   
    QString fileName = fileNames.at(currentIndex);
    QString filepath(fileName);
    QString multiSpectralImagePath450 = fileName.replace("ground truth", "ms450");
    ImageDialog dialog(multiSpectralImagePath450, "�����ģ̬");
    dialog.exec();
    //subW->uploadImageArg(multiSpectralImagePath450);
    /*QFileInfo multiSpectralImageFileInfo(multiSpectralImagePath450);
    QString multiSpectralBaseName = multiSpectralImageFileInfo.fileName();
    subW->pathMs1 = "ms450"+multiSpectralBaseName;
    subW->startmatchRequestone(filepath, 2);*/
}
void earth_map_demo::stopThead() {
    QString currentStyleSheet = ui.closeRange->styleSheet();
    QString newImagePath;
    if (!is_Stop) {
        is_Stop = true;  // ����Ϊ true ��ʾֹͣ
        calculationThread->stop();
        for (auto node : featurenode) {
            node->setNodeMask(0);
        }
        newImagePath = ":/icon_3axusv3ezyt/start_use.png";
        ui.label_10->setText("�򿪷�Χ");
    }
    else {
        is_Stop = false;  // ����Ϊ false ��ʾ���¿�ʼ
        calculationThread->restart();
        newImagePath = ":/icon_3axusv3ezyt/closeRange.png";
        ui.label_10->setText("�رշ�Χ");
    }

    // �滻��ʽ���еı���ͼ��·��
    QString newStyleSheet = currentStyleSheet.replace(
        QRegExp("url\\(.*\\)"), QString("url(%1)").arg(newImagePath)
    );
    // ���°�ť�ı�����ʽ��
    ui.closeRange->setStyleSheet(newStyleSheet);
}


void earth_map_demo::showsubWidget() {
    ui.frame->setVisible(false);
    subW->setVisible(true);
    subW->raise();
}

//void earth_map_demo::matchAndFuse() {
//    counter = 0;
//    connect(timer, &QTimer::timeout, this, &earth_map_demo::updateProgress);
//    timer->start(1000); // ��ʱ��ÿ100���봥��һ��
//    ui.progressBar->setVisible(true);
//    ui.progressBar->setValue(0);
//    QString fileName = fileNames.at(currentIndex);
//    QString groundName = fileName;
//    QString infraredImagePath = fileName.replace("ground truth", "ir");
//    QString multiSpectralImagePath450 = fileName.replace("ir", "ms450");
//    QString multiSpectralImagePath840 = fileName.replace("ms450", "ms840");
//
//    QFileInfo groundFileInfo(groundName);
//    QString groundBaseName = groundFileInfo.fileName();
//    QFileInfo infraredFileInfo(infraredImagePath);
//    QString infraredBaseName = infraredFileInfo.fileName();
//    QFileInfo multiSpectral450FileInfo(multiSpectralImagePath450);
//    QString multiSpectral450BaseName = multiSpectral450FileInfo.fileName();
//    QFileInfo multiSpectral840FileInfo(multiSpectralImagePath840);
//    QString multiSpectral840BaseName = multiSpectral840FileInfo.fileName();
//    infraredImagePath = infraredImagePath.replace(groundBaseName, "ir" + infraredBaseName);
//    multiSpectralImagePath450 = multiSpectralImagePath450.replace(groundBaseName, "ms450" + multiSpectral450BaseName);
//    multiSpectralImagePath840 = multiSpectralImagePath840.replace(groundBaseName, "ms840" + multiSpectral840BaseName);
//    
//    subW->setPath(groundBaseName, "ir" + infraredBaseName, "ms450" + multiSpectral450BaseName, "ms840" + multiSpectral840BaseName);
//    subW->uploadImageArg(groundName);
//    subW->uploadImageArg(infraredImagePath);
//    subW->uploadImageArg(multiSpectralImagePath450);
//    subW->uploadImageArg(multiSpectralImagePath840);
//
//    //��׼
//    subW->startmatchRequest();
//    ////�ں�
//    subW->startnetRequest();
//    counter = 95;
//    ui.progressBar->setVisible(false);
//    replaceFuseImage();
//}


void earth_map_demo::updateProgress() {
    counter++;
    qDebug() << counter;
    ui.progressBar->setValue(counter);

    if (counter >= 100)
    {
        timer->stop();
        ui.progressBar->setVisible(false);
    }

}
