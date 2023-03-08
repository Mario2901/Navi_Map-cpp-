#include <QPainter>
#include <QTimer>
#include <QString>
#include <cmath>
#include <iostream>
#include <QDebug>
#include <QDialog>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include "QtTipsDlgView.h"
#include "routing.h"
#include "ex5_mapNavi.h"
#include "QtWidgetsFL.h"
#include "SearchLoc.h"

GIS* gis = 0;        //��ͼ��Ϣ
bool route = false;  //�Ƿ���ʾ��ѯ·��ʱ
bool depart = false; //�Ƿ�ѡȡ�������
bool arrive = false; //�Ƿ�ѡȡ�����յ�
MyItem::MyItem(int x, int y, int f) : QGraphicsRectItem(x - 3, y - 3, 7, 7), cx(x), cy(y), cf(f), cs(0)
{
    //���������������      ����Ҽ��������յ�
    QBrush qbrush(f == 1 ? QColor(0, 220, 20) : QColor(5, 5, 220)); //������ɫ
    QPen qpens(qbrush, 4, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
    QGraphicsRectItem* item = this;
    item->setPen(qpens);
    item->setBrush(qbrush);
    checkAllStops(); //�������վ�㣬�ҳ������վ�����bs��
}
int& MyItem::x() { return cx; }
int& MyItem::y() { return cy; }
int& MyItem::f() { return cf; }
int& MyItem::c() { return cs; }
int& MyItem::operator[](int x)
{ //�����Ѽ��ı��Ϊx��վ����
    if (x < 0 || x >= cs)
        throw "subscription overflow for stops around departure point!";
    return bs[x];
}
int MyItem::operator()(int x, int y)
{ //������Զ��
    return (x - cx) * (x - cx) + (y - cy) * (y - cy);
}
MyItem& MyItem::operator<<(int s)
{
    if (s < 0 || s >= GIS::ns)
        return *this;
    int d = (*this)(GIS::st[s].X(), GIS::st[s].Y()); // dΪ��ǰͼԪ�㵽վ��s�ľ���
    int m;                                           // mΪ��ǰͼԪ�㵽��ǰ�Ѽ��վ��ľ���
    if (cs == 0 || d < (m = (*this)(GIS::st[bs[0]].X(), GIS::st[bs[0]].Y())))
    { //��bsû��Ԫ�ؼ�cs==0�������վ��s����
        bs[0] = s;
        cs = 1;
        return *this;
    }
    if (d == m)
    { //���Ѽ��վ��ȣ������Ϊs��վ��ľ�����ͬʱ
        if (cs == 6)
            return *this;
        bs[cs] = s; //ֻ��������������ͬ��վ��
        cs++;       //������ͬ��վ���������
    }
    return *this;
}
int MyItem::checkAllStops()
{ //�������վ�㣬�ҳ������վ�����bs��
    for (int c = 0; c < GIS::ns; c++)
        operator<<(GIS::st[c].N());
    return cs; //���ؾ����������ͬ��վ�����
}
void MyItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    setSelected(true); //��ǰͼԪ��ѡ��
    QGraphicsRectItem::mousePressEvent(event);
}
//�����Լ��ĳ���MyScene���Ա��ܹ�������������¼�
MyScene::MyScene(QObject* parent) : QGraphicsScene(parent)
{
    clearFocus();
    qgv = Q_NULLPTR; //û�м��ص�ͼ�ļ�ʱ
}
//���ֹͣ��ʾ��Ϣ
void MyScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (qgv == Q_NULLPTR)
    { //���û�м��ص�ͼ�ļ�
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }
    QPointF qpointf = event->scenePos(); //��ȡ����ƶ�ʱ������
    for (int n = 0; n < gis->ns; n++)
    {
        if (fabs(gis->st[n].X() - qpointf.x()) < 8 && fabs(gis->st[n].Y() - qpointf.y()) < 8)
        {
            QtTipsDlgView dlg(QString::fromLocal8Bit("��") + QString::number(n + 1, 10, 0) + QString::fromLocal8Bit("������վ��."));
            // dlg.setAttribute(Qt::WA_ShowModal,true); ������show()���������ޱ߿�,������dlg.exec()���ô��У�
            dlg.startTimer(328); //�Զ�����ͣʱ��328ms
            QPointF m1 = qgv->mapToGlobal(QPoint(qpointf.x(), qpointf.y()));
            dlg.move(QPoint(m1.x(), m1.y()));
            dlg.exec(); //��ʾվ����ʾ��Ϣ
        }
    }
    //��ʾ�ص���Ϣ
    for (int i = 0; i < gis->org_num; i++)
    {
        if (fabs(gis->orgs[i].x() - qpointf.x()) < 5 && fabs(gis->orgs[i].y() - qpointf.y() < 5))
        { //�����һ����Χ��
            QtTipsDlgView tip(gis->orgs[i].org_name());
            tip.startTimer(328);
            QPointF p = qgv->mapToGlobal(QPoint(qpointf.x(), qpointf.y()));
            tip.move(QPoint(p.x(), p.y()));
            tip.exec();
        }
    }
    QGraphicsScene::mouseMoveEvent(event); //�ص���������¼�
}

void MyScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (qgv == Q_NULLPTR || route == true)
    { //δ���ص�ͼ����ʾ��ѯ���ʱ������Ӧ��갴���¼�
        QGraphicsScene::mouseMoveEvent(event);
        return;
    }
    QPointF qpointf = event->scenePos(); //��ȡ�������
    QList<QGraphicsItem*> listItem = items();
    int lb = 0;
    if (event->button() == Qt::LeftButton)
    {
        lb = 1;
        depart = true;
    } //�������Ƿ���
    if (event->button() == Qt::RightButton)
    {
        lb = 2;
        arrive = true;
    } //�������Ƿ���
    for (int i = listItem.length() - 1; i >= 0; i--)
    {
        MyItem* item = (MyItem*)listItem[i];
        if (item->f() == lb)
        {
            listItem.removeAt(i);
            delete item;
            break;
        }
    }
    addItem(new MyItem(qpointf.x(), qpointf.y(), lb));
    QGraphicsScene::mousePressEvent(event); //�ص���������¼�
}
void MyScene::stopLines(QGraphicsView* par)
{ //���ص�ͼվ�����·
    //����ͼgraphicsview��С����scene��ʾ������С
    QSize viewsize = par->size(); //ȡ��graphicsview��ͼ�����С
    MyScene* scene;
    if (par->scene() != Q_NULLPTR)
        delete par->scene();
    scene = new MyScene(par); //����scene
    scene->setSceneRect(0, 0, viewsize.width(), viewsize.height());
    scene->qgv = par;

    //��ʾ���й���վ��
    QBrush qbrush(QColor(200, 120, 10));
    QPen qpens(qbrush, 4, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin); //վ��
    for (int n = 0; n < gis->ns; n++)
    {
        scene->addEllipse(gis->st[n].X(), gis->st[n].Y(), 6, 6, qpens, qbrush);
    }

    //��ʾ�����ض�����
    qbrush.setColor(QColor(220, 0, 0));                                //����Ϊ��ɫ^_^
    QPen peno(qbrush, 4, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin); //��
    for (int n = 0; n < gis->org_num; n++)
        scene->addEllipse(gis->orgs[n].x(), gis->orgs[n].y(), 6, 6, peno, qbrush);

    //��ʾ������·
    qbrush.setColor(QColor(30, 200, 40));                              //����Ϊ��ɫ^_^
    QPen qpenl(qbrush, 3, Qt::DashLine, Qt::SquareCap, Qt::BevelJoin); //��·
    for (int n = 0; n < gis->nl; n++)
    {
        LINE& line = gis->ls[n];
        int stops = line.NOFS();
        for (int m = 1; m < stops; m++)
        {
            STOP s = gis->st[line[m - 1]];
            STOP t = gis->st[line[m]];
            QLineF ql = QLineF(s.X(), s.Y(), t.X(), t.Y());
            scene->addLine(ql, qpenl);
        }
    }

    par->setScene(scene);
    par->show();
}

ex5_mapNavi::ex5_mapNavi(QWidget* parent)
    : QMainWindow(parent)
{
    //��ѯ�Ľ����ʾ��ʱ������
    ui.setupUi(this);
    fl = Q_NULLPTR;
    gItem = Q_NULLPTR;
    search_p = Q_NULLPTR;
    m_Timer = new QTimer(this);
    m_Timer->setSingleShot(true); //��ʱ��ִ��һ��
    connect(ui.action_open, SIGNAL(triggered(bool)), this, SLOT(loadmap()));
    connect(ui.action_exit, SIGNAL(triggered(bool)), this, SLOT(closewnd()));
    connect(ui.action_zszc, SIGNAL(triggered(bool)), this, SLOT(zszc()));
    connect(ui.action_zdjl, SIGNAL(triggered(bool)), this, SLOT(zdjl()));
    connect(ui.action_searchLoc, SIGNAL(triggered(bool)), this, SLOT(searchLoc()));
    connect(m_Timer, &QTimer::timeout, this, [=]()
        {
            QList<QGraphicsItem*> listItem = ui.graphicsView->scene()->items();
            deleteItems();//��ѯ�����ʾʱ��һ������ɾ������������ͼԪ
            route = false;//��ѯ�����ʾ��ϣ���������ѡȡ���������յ� 
        }
            );
        }
        ex5_mapNavi::~ex5_mapNavi()
        {
            if (fl != Q_NULLPTR)
            {
                fl->hide();
                delete fl;
                fl = Q_NULLPTR;
                delete m_Timer;
                deleteItems();
                delete gis;
            }
        }
        void ex5_mapNavi::searchLoc()//�����ص�
        {
            if (fl == Q_NULLPTR|| ui.action_searchLoc->isChecked() == false)
                return;
            ui.action_searchLoc->setChecked(false);//��ֹ��δ���
            if (search_p)
            {
                search_p->show();
                return;
            }
            search_p = new SearchLoc(this);
            search_p->setWindowFlags(Qt::WindowStaysOnTopHint);
            search_p->show();
        }
        void ex5_mapNavi::closewnd()
        {
            if (ui.action_exit->isChecked() == false)
                return;                        //�����һ�δ������Σ��ڶ��δ���ֱ�ӷ���
            ui.action_exit->setChecked(false); //��ֹ��δ���
            if (fl != Q_NULLPTR)
            {
                fl->hide();
                delete fl;
                fl = Q_NULLPTR;
            }
            close();
        }
        void ex5_mapNavi::closeEvent(QCloseEvent * event)
        {
            if (fl != Q_NULLPTR)
            {
                fl->hide();
                delete fl;
                fl = Q_NULLPTR;
            }
            if (search_p)
            {
                search_p->hide();
                delete search_p;
                search_p = Q_NULLPTR;
            }
        }
        void ex5_mapNavi::loadmap()
        {

            if (ui.action_open->isChecked() == false)
                return;                        //������������Σ��ڶ��δ���ֱ�ӷ���
            ui.action_open->setChecked(false); //��ֹ��δ���
            if (fl != Q_NULLPTR)
            {               //�����ǰ�򿪹�վ�㼰��·���봰��
                fl->show(); //��ֱ����ʾ�ô���
                return;
            }
            arrive = depart = false;                      //��ʱδѡȡ���������յ�
            fl = new QtWidgetsFL();                       //�����ǰû�д򿪹�վ�㼰��·���봰��
            fl->setWindowFlags(Qt::WindowStaysOnTopHint); //�����������ʾ
            fl->myShow(ui.graphicsView);
        }
        void ex5_mapNavi::deleteItems()
        { //ɾ������������ͼԪ
            if (gItem == Q_NULLPTR)
                return;
            ui.graphicsView->scene()->removeItem(gItem);
            for (int i = gItem->childItems().size() - 1; i >= 0; i--)
            {
                QGraphicsItem* item = (gItem->childItems())[i];
                gItem->removeFromGroup(item);
                delete item;
            }
            delete gItem;
            gItem = Q_NULLPTR;
        }
        void ex5_mapNavi::zszc()
        {
            //�ȼ�������ת�˵�·��,�Ȼ�����������յ�����
            if (ui.action_zszc->isChecked() == false)
                return;                        //�����һ�δ������Σ��ڶ��δ���ֱ�ӷ���
            ui.action_zszc->setChecked(false); //����һ�δ�������״̬Ϊfalse,��ֹ��2�δ�������
            QList<QGraphicsItem*> listItem;
            if ((depart && arrive) == false)
                return; //��û��ѡ�в��������յ㣬�򷵻�
            listItem = ui.graphicsView->scene()->items();
            MyItem* itemDepart = (MyItem*)listItem[0];
            MyItem* itemArrive = (MyItem*)listItem[1];
            if (itemDepart->f() != 1)
            { //�����ǲ�����㣬�򽻻�
                itemDepart = (MyItem*)listItem[1];
                itemArrive = (MyItem*)listItem[0];
            }
            //��ʼ�齨ͼԪ�飺������ʾת�˷�����·��
            QGraphicsEllipseItem* myEItem;
            QGraphicsLineItem* myLItem;
            MyScene* scene = (MyScene*)(ui.graphicsView->scene());
            QBrush qbrush(QColor(250, 10, 15));                                 //������ɫ
            QPen qpens(qbrush, 4, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin); //վ��ı�
            QPen qpenl(qbrush, 3, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin); //��·�ı�
            route = true;                                                       //�����ѯ·����ʾ�ڼ䣬����Ӧ����������յ�ѡȡ
            int c, n = 0;                                                       // cΪ����ת�˷�������nΪ����ת�˴���
            ROUTE r[100];                                                       //һ�β�ѯ����෵��100������ת�˷���
            for (int d = 0; d < itemDepart->c(); d++)
            {                             //�ӽ��������й���վ��
                int s = (*itemDepart)[d]; //������վ����s
                for (int a = 0; a < itemArrive->c(); a++)
                {                             //�ӽ��յ�����й���վ��
                    int t = (*itemArrive)[a]; //����յ�վ����t
                    if (s == t)
                    {                  //���վ���յ�վ��ͬ����ת��
                        deleteItems(); //ɾ����ǰ��ת�˷���·����ʾ
                        gItem = new QGraphicsItemGroup();
                        myEItem = new QGraphicsEllipseItem(itemDepart->x(), itemDepart->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(qbrush);
                        gItem->addToGroup(myEItem); //��������λ��
                        myLItem = new QGraphicsLineItem(itemDepart->x(), itemDepart->y(), itemArrive->x(), itemArrive->y());
                        myLItem->setPen(qpenl);
                        gItem->addToGroup(myLItem); //�������յ��·��
                        myEItem = new QGraphicsEllipseItem(itemArrive->x(), itemArrive->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(QBrush(QColor(255, 170, 00)));
                        gItem->addToGroup(myEItem); //�����յ��λ��
                        scene->addItem(gItem);
                        ui.graphicsView->setScene(scene);
                        continue;
                    }
                    c = GIS::tra.miniTran(s, t, n, r); //�õ���ת�˷�����
                    for (int m = 0; m < c; m++)
                    { //���ڵ�m��ת�˷�����route=r[m], ת�˴�����Ϊn
                        deleteItems();
                        gItem = new QGraphicsItemGroup();
                        myEItem = new QGraphicsEllipseItem(itemDepart->x(), itemDepart->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(qbrush);
                        gItem->addToGroup(myEItem); //��������λ��
                        int fr = s, to = t;         //��ʼվfr���յ�վto
                        int fm, tm;                 //�ѳ���·fm��zת����·tm
                        int bg, ed;                 //��·�е���ʼվ�����bg,��ֹվ�����ed
                        myLItem = new QGraphicsLineItem(itemDepart->x(), itemDepart->y(), gis->st[s].X(), gis->st[s].Y());
                        myLItem->setPen(qpenl);
                        gItem->addToGroup(myLItem); //�����վ
                        if (n == 1 && r[m][0].S() == -1)
                        {                             //����i·��i·(��ʱS()==-1)����ת��
                            fm = r[m][0].F();         //�ѳ���·���fm
                            bg = GIS::ls[fm].has(fr); //��ʼվ������·�е����
                            ed = GIS::ls[fm].has(to); //��ֹվ������·�е����
                            if (bg > ed)
                            {
                                tm = bg;
                                bg = ed;
                                ed = tm;
                            }
                            for (int y = bg; y < ed; y++) //����ʼվ����һ��ŵ���ֹվ�����
                            {
                                fr = GIS::ls[fm][y];     //�õ���վ����Ŷ�Ӧ��վ����
                                to = GIS::ls[fm][y + 1]; //�õ���վ����Ŷ�Ӧ��վ����
                                myLItem = new QGraphicsLineItem(GIS::st[fr].X(), GIS::st[fr].Y(), GIS::st[to].X(), GIS::st[to].Y());
                                myLItem->setPen(qpenl);
                                gItem->addToGroup(myLItem); //����һվ��·��
                            }
                        }
                        else
                        {
                            for (int y = 0; y < n; y++) //����ÿ��ת��
                            {
                                fm = r[m][y].F(); //����ÿ��ת�˵���ʼ��·
                                bg = GIS::ls[fm].has(fr);
                                to = r[m][y].S(); //������ʼ��·��ת��վ��
                                ed = GIS::ls[fm].has(to);
                                if (bg > ed)
                                {
                                    tm = bg;
                                    bg = ed;
                                    ed = tm;
                                }
                                for (int u = bg; u < ed; u++) //����ʼվ����һ��ŵ���ֹվ�����
                                {
                                    int ff = GIS::ls[fm][u];     //�õ���վ����Ŷ�Ӧ��վ����
                                    int tt = GIS::ls[fm][u + 1]; //�õ���վ����Ŷ�Ӧ��վ����
                                    myLItem = new QGraphicsLineItem(GIS::st[ff].X(), GIS::st[ff].Y(), GIS::st[tt].X(), GIS::st[tt].Y());
                                    myLItem->setPen(qpenl);
                                    gItem->addToGroup(myLItem); //����һվ��·��
                                }
                                fr = to; //��Ϊ��һ���
                            }
                            fm = r[m][n - 1].T(); //��������������·
                            bg = GIS::ls[fm].has(fr);
                            ed = GIS::ls[fm].has(t);
                            if (bg > ed)
                            {
                                tm = bg;
                                bg = ed;
                                ed = tm;
                            }
                            for (int u = bg; u < ed; u++) //����ʼվ����һ��ŵ���ֹվ�����
                            {
                                int ff = GIS::ls[fm][u];     //�õ���վ����Ŷ�Ӧ��վ����
                                int tt = GIS::ls[fm][u + 1]; //�õ���վ����Ŷ�Ӧ��վ����
                                myLItem = new QGraphicsLineItem(GIS::st[ff].X(), GIS::st[ff].Y(), GIS::st[tt].X(), GIS::st[tt].Y());
                                myLItem->setPen(qpenl);
                                gItem->addToGroup(myLItem); //����һվ��·��
                            }
                        }
                        myLItem = new QGraphicsLineItem(GIS::st[t].X(), GIS::st[t].Y(), itemArrive->x(), itemArrive->y());
                        myLItem->setPen(qpenl);
                        gItem->addToGroup(myLItem); //�������յ��·��
                        myEItem = new QGraphicsEllipseItem(itemArrive->x(), itemArrive->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(QBrush(QColor(255, 170, 00)));
                        gItem->addToGroup(myEItem); //�����յ��λ��
                        scene->addItem(gItem);
                        ui.graphicsView->setScene(scene);
                    }
                }
            }
            this->m_Timer->start(5000); //չʾ��ѯ��·�����5��
        }
        void ex5_mapNavi::zdjl()
        {
            //�ȼ�����̾����·��,�Ȼ�����������յ�����
            if (ui.action_zdjl->isChecked() == false)
                return;                        //�����һ�δ������Σ��ڶ��γ���ֱ�ӷ���
            ui.action_zdjl->setChecked(false); //��ֹ��δ���
            QList<QGraphicsItem*> listItem;
            if ((depart && arrive) == false)
                return;
            listItem = ui.graphicsView->scene()->items();
            MyItem* itemDepart = (MyItem*)listItem[0];
            MyItem* itemArrive = (MyItem*)listItem[1];
            if (itemDepart->f() != 1)
            { //��������㣬�򽻻�
                itemDepart = (MyItem*)listItem[1];
                itemArrive = (MyItem*)listItem[0];
            }
            //�齨ͼԪ�飺������ʾת�˷�����·��
            QGraphicsEllipseItem* myEItem;
            QGraphicsLineItem* myLItem;
            MyScene* scene = (MyScene*)(ui.graphicsView->scene());
            QBrush qbrush(QColor(210, 10, 15));                                 //������ɫΪ��ɫ^_^
            QPen qpens(qbrush, 4, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin); //վ��ı�
            QPen qpenl(qbrush, 3, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin); //��·�ı�
            route = true;                                                       //�����ѯ·����ʾ�ڼ䣬����Ӧ����������յ�ѡȡ
            int c = 0;                                                          // cΪ������·��
            double dist = 0;                                                    // distΪ��̾���
            ROUTE r[100];
            for (int d = 0; d < itemDepart->c(); d++)
            {                             //�ӽ��������й���վ��
                int s = (*itemDepart)[d]; //������վ����s
                for (int a = 0; a < itemArrive->c(); a++)
                {                             //�ӽ��յ�����й���վ��
                    int t = (*itemArrive)[a]; //����յ�վ����t
                    if (s == t)
                    {                  //���վ���յ�վ��ͬ����ת��
                        deleteItems(); //ɾ����ǰ��ת�˷���·����ʾ
                        gItem = new QGraphicsItemGroup();
                        myEItem = new QGraphicsEllipseItem(itemDepart->x(), itemDepart->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(qbrush);
                        gItem->addToGroup(myEItem); //��������λ����ʾ
                        myLItem = new QGraphicsLineItem(itemDepart->x(), itemDepart->y(), itemArrive->x(), itemArrive->y());
                        myLItem->setPen(qpenl);
                        gItem->addToGroup(myLItem); //�������յ��·��
                        myEItem = new QGraphicsEllipseItem(itemArrive->x(), itemArrive->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(QBrush(QColor(255, 170, 00)));
                        gItem->addToGroup(myEItem); //�����յ��λ��
                        scene->addItem(gItem);
                        ui.graphicsView->setScene(scene);
                        continue;
                    }
                    c = GIS::tra.miniDist(s, t, dist, r); //�õ���ת�˷�����c
                    for (int m = 0; m < c; m++)
                    { //���ڵ�m��ת�˷�����route=r[m],��̾��붼Ϊdist
                        deleteItems();
                        gItem = new QGraphicsItemGroup();
                        myEItem = new QGraphicsEllipseItem(itemDepart->x(), itemDepart->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(qbrush);
                        gItem->addToGroup(myEItem); //��������λ����ʾ
                        int fr = s, to = t;         //��ʼվfr���յ�վto
                        int fm, tm;                 //�ѳ���·fm��zת����·tm
                        int bg, ed;                 //��·�е���ʼվ�����bg,��ֹվ�����ed
                        myLItem = new QGraphicsLineItem(itemDepart->x(), itemDepart->y(), gis->st[s].X(), gis->st[s].Y());
                        myLItem->setPen(qpenl);
                        gItem->addToGroup(myLItem); //�����վ��·����ʾ
                        int n = r[m];               //��ǰroute�е�ת�˴���
                        if (n == 1 && r[m][0].S() == -1)
                        {                             //����i·��i·(��ʱS()==-1)����ת��
                            fm = r[m][0].F();         //�ѳ���·���fm
                            bg = GIS::ls[fm].has(fr); //��ʼվ������·�е����
                            ed = GIS::ls[fm].has(to); //��ֹվ������·�е����
                            if (bg > ed)
                            {
                                tm = bg;
                                bg = ed;
                                ed = tm;
                            }
                            for (int y = bg; y < ed; y++) //����ʼվ����һ��ŵ���ֹվ�����
                            {
                                fr = GIS::ls[fm][y];     //�õ���վ����Ŷ�Ӧ��վ����
                                to = GIS::ls[fm][y + 1]; //�õ���վ����Ŷ�Ӧ��վ����
                                myLItem = new QGraphicsLineItem(GIS::st[fr].X(), GIS::st[fr].Y(), GIS::st[to].X(), GIS::st[to].Y());
                                myLItem->setPen(qpenl);
                                gItem->addToGroup(myLItem); //����һվ
                            }
                        }
                        else
                        {
                            for (int y = 0; y < n; y++) //����ÿ��ת�˻���·��
                            {
                                fm = r[m][y].F(); //����ÿ��ת�˵���ʼ��·
                                bg = GIS::ls[fm].has(fr);
                                to = r[m][y].S(); //������ʼ��·��ת��վ��
                                ed = GIS::ls[fm].has(to);
                                if (bg > ed)
                                {
                                    tm = bg;
                                    bg = ed;
                                    ed = tm;
                                }
                                for (int u = bg; u < ed; u++) //����ʼվ����һ��ŵ���ֹվ�����
                                {
                                    int ff = GIS::ls[fm][u];     //�õ���վ����Ŷ�Ӧ��վ����
                                    int tt = GIS::ls[fm][u + 1]; //�õ���վ����Ŷ�Ӧ��վ����
                                    myLItem = new QGraphicsLineItem(GIS::st[ff].X(), GIS::st[ff].Y(), GIS::st[tt].X(), GIS::st[tt].Y());
                                    myLItem->setPen(qpenl);
                                    gItem->addToGroup(myLItem); //����һվ��·��
                                }
                                fr = to; //��Ϊ��һ���
                            }
                            fm = r[m][n - 1].T(); //��������������·
                            bg = GIS::ls[fm].has(fr);
                            ed = GIS::ls[fm].has(t);
                            if (bg > ed)
                            {
                                tm = bg;
                                bg = ed;
                                ed = tm;
                            }
                            for (int u = bg; u < ed; u++) //����ʼվ����һ��ŵ���ֹվ�����
                            {
                                int ff = GIS::ls[fm][u];     //�õ���վ����Ŷ�Ӧ��վ����
                                int tt = GIS::ls[fm][u + 1]; //�õ���վ����Ŷ�Ӧ��վ����
                                myLItem = new QGraphicsLineItem(GIS::st[ff].X(), GIS::st[ff].Y(), GIS::st[tt].X(), GIS::st[tt].Y());
                                myLItem->setPen(qpenl);
                                gItem->addToGroup(myLItem); //����һվ��·��
                            }
                        }
                        myLItem = new QGraphicsLineItem(GIS::st[t].X(), GIS::st[t].Y(), itemArrive->x(), itemArrive->y());
                        myLItem->setPen(qpenl);
                        gItem->addToGroup(myLItem); //�������յ��·��
                        myEItem = new QGraphicsEllipseItem(itemArrive->x(), itemArrive->y(), 6, 6);
                        myEItem->setPen(qpens);
                        myEItem->setBrush(QBrush(QColor(205, 180, 10)));
                        gItem->addToGroup(myEItem); //�����յ��λ����ʾ
                        scene->addItem(gItem);
                        ui.graphicsView->setScene(scene);
                    }
                }
            }
            this->m_Timer->start(3280); //չʾ��ѯ��·�����3280ms
        }
