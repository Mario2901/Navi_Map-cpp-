#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ex5_mapNavi.h"
#include <QGraphicsRectItem>
#include "QtWidgetsFL.h"
#include "routing.h"
#include "SearchLoc.h"
#include <vector>
class QtWidgetsFL;
class ex5_mapNavi : public QMainWindow
{
    Q_OBJECT

public:
    ex5_mapNavi(QWidget *parent = 0);
    ~ex5_mapNavi();

private:
    Ui::ex5_mapNaviClass ui;
	QtWidgetsFL* fl;
	SearchLoc* search_p;//ָ�����������ָ��
	QTimer* m_Timer;
	QGraphicsItemGroup* gItem;
	void deleteItems();
protected:
	void closeEvent(QCloseEvent* event);
private slots:
    void loadmap();//���ص�ͼ
    void closewnd();//�رմ���
    void zdjl();//��̾���
    void zszc();//����ת��
	void searchLoc();//�����ص�
};

//�����Լ��ĳ���MyScene���Ա��ܹ�������������¼�
class MyScene :public QGraphicsScene {
public:
	explicit MyScene(QObject* parent = 0);
	void stopLines(QGraphicsView*);
protected:
	QGraphicsView* qgv;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);	//����mousePressEvent�Բ�������¼�
	void mousePressEvent(QGraphicsSceneMouseEvent* event);	//����mousePressEvent�Բ�������¼�
};

class MyItem : public QGraphicsRectItem {
private:
	int cx, cy;			//���ʱ������
	int cf;				//������=1����ʾ�ص㣬�Ҽ����=2��ʾ�յ�
	int cs;				//�����õ���������
	int bs[60];		//���վ���վ����
public:
	MyItem(int x, int y, int f);
	MyItem& operator<<(int s);
	int operator()(int x, int y);
	int& x();
	int& y();
	int& f();
	int& c();
	int& operator[](int);
	int checkAllStops();	//��������վ��
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
};


