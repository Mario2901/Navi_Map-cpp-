#include "SearchLoc.h"
#include "ex5_mapNavi.h"
#include <QCompleter>
extern GIS* gis;

SearchLoc::SearchLoc(QMainWindow* par, QWidget* parent): QWidget(parent), begin({ -1,-1 }), end({ -1,-1 })
{
	ui.setupUi(this);
	connect(ui.pushButtonQuit, SIGNAL(clicked()), this, SLOT(close()), Qt::UniqueConnection);
	connect(ui.pushButtonDone, SIGNAL(clicked()), this, SLOT(getstr()), Qt::UniqueConnection);
	connect(ui.pushButtonDone, SIGNAL(clicked()), par, SLOT(showPath()), Qt::UniqueConnection);
	if (gis)//��������ģ��ƥ��Ĵʱ�
	{
		for (int i = 0; i < gis->org_num; i++)
			wordList << gis->orgs[i].org_name();
	}
	QCompleter* MyInforom = new QCompleter(wordList);//�Զ���书��
	MyInforom->setFilterMode(Qt::MatchContains);
	ui.lineEditStart->setCompleter(MyInforom);//��ʼ���Զ����
	ui.lineEditEnd->setCompleter(MyInforom);//����λ���Զ����
}
//��������
SearchLoc::~SearchLoc()
{
	//���ǿյ�
}
//��ȡ�����ַ���
void SearchLoc::getstr()
{
	ui.labelHit->setText("");
	QString s = ui.lineEditStart->text();//��ʼ���ַ���
	QString e = ui.lineEditEnd->text();
	if (s.isEmpty() || e.isEmpty()) //����Ϊ��
	{  
		ui.labelHit->setStyleSheet("color: rgb(235, 10, 10)");
		ui.labelHit->setText(QString::fromLocal8Bit("������ʾ�������������յ㣡"));
		return;
	}

	//��������ĵص�
	for (int i = 0; i < gis->org_num; i++) //��������
	{
		Organization& find1 = gis->orgs[i];
		//�ҵ��˾���������
		if (s == find1.org_name()) { begin.setX(find1.x()); begin.setY(find1.y()); }
		if (e == find1.org_name()) { end.setX(find1.x()); end.setY(find1.y()); }
	}

	//����ʧ��
	if (begin.x() == -1 || end.x() == -1) {
		ui.labelHit->setStyleSheet("color: rgb(235, 10, 10)");
		ui.labelHit->setText(QString::fromLocal8Bit("������ʾ����������ȷ�������յ㣡"));
		return;
	}
	ui.labelHit->setText("");
	close();
}

