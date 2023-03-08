#include <QFileDialog>
#include <QMessageBox>
#include "QtWidgetsFL.h"
#include "ex5_mapNavi.h"
#include "SearchLoc.h"
#include "routing.h"

//�Զ����վ�㼰��·�������
QtWidgetsFL::QtWidgetsFL(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);//���Ӱ�ť�ͺ���
	connect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(inputStop()), Qt::UniqueConnection);
	connect(ui.pushButtonLine, SIGNAL(clicked()), this, SLOT(inputLine()), Qt::UniqueConnection);
	connect(ui.pushButtonOrg, SIGNAL(clicked()), this, SLOT(inputOrg()), Qt::UniqueConnection);
	connect(ui.pushButtonDone, SIGNAL(clicked()), this, SLOT(checkFile()), Qt::UniqueConnection);
	connect(ui.pushButtonQuit, SIGNAL(clicked()), this, SLOT(close()), Qt::UniqueConnection);
}
void QtWidgetsFL::myShow(QGraphicsView* p) {
	par = p;
	show();
}
QtWidgetsFL::~QtWidgetsFL()
{
}
void QtWidgetsFL::inputStop() {//�ı������뺯��
	ui.labelHits->setText("");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("*.txt"));
	ui.textEditStop->setText(fileName);
}
void QtWidgetsFL::inputLine() {//�ı������뺯��
	ui.labelHits->setText("");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "lines", tr("*.txt"));
	ui.textEditLine->setText(fileName);
}
void QtWidgetsFL::inputOrg() {//������֯�ļ�
	ui.labelHits->setText("");
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("*.txt"));
	ui.textEditOrg->setText(fname);
}
void QtWidgetsFL::checkFile() {//�����ļ�
	QString  fs = ui.textEditStop->toPlainText();
	QString  fl = ui.textEditLine->toPlainText();
	QString  fo = ui.textEditOrg->toPlainText();
	if (fs.isEmpty() && fl.isEmpty() && fo.isEmpty()) {//����ļ�����
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input any file!")); //����fromLocal8Bit��ʾ����
		ui.textEditStop->setFocus();
		return;
	}
	if (fs.isEmpty()) {
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input stop file!"));
		ui.textEditStop->setFocus();
		return;
	}
	if (fl.isEmpty()) {
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input line file!"));
		ui.textEditLine->setFocus();
		return;
	}
	if (fo.isEmpty()) {
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input org file!"));
		ui.textEditLine->setFocus();
		return;
	}
	//����վ���ļ�
	ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
	ui.labelHits->setText(QString::fromLocal8Bit("have not input file!"));
	try {//����վ�㼰��·�ļ�����ʼ��
		if (gis != 0) delete gis;
		gis = new GIS(fs.toStdString().c_str(), fl.toStdString().c_str(),fo.toStdString().c_str());
		((MyScene*)(par->scene()))->stopLines(par); //�ڱ�����ͼ�ϻ���վ�㼰������·
	}
	catch (...) {//����ʧ��
		gis = 0;
		close();
		QMessageBox::information(NULL, QString::fromLocal8Bit("������ʾ"), QString::fromLocal8Bit("����վ��򹫽���·�ļ����뼰��ʼ��ʧ�ܣ�"));
	}
	ui.labelHits->setText("");
	close();
}