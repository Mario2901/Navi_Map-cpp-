#pragma once
#include <QString>
#include <vector>
class STOP {//������վ
    int numb;//����վ���
    int x;//x����
    int y;//y����
public:
    //���캯��
    STOP(int numb = 0, int x = 0, int y = 0);
    //���ݽӿں���
    virtual int& N();//����վ���
    virtual int& X();//x����
    virtual int& Y();//y����
};


class LINE {//��������·
    const int numb;//���������
    int* const stop;//�����¼����·�ϵ�����վ��
    const int nofs;//��ͣ��վ����
public:
    LINE(int numb = 0, int nofs = 0, int* stop = 0);
    LINE(const LINE& r);//�������
    LINE(LINE&& r) noexcept;//�ƶ�����
    LINE& operator=(const LINE& r);//�����ֵ
    LINE& operator=(LINE&& r)noexcept;//�ƶ���ֵ
    virtual int has(int s)const;//�Ƿ����վ����s�������򷵻ذ����ڼ�վ�Σ�-1����δ����
    virtual int cross(const LINE& b) const;//�ж�������·�ཻ���ཻ����1
    virtual operator int() const;//ȡ��·���
    virtual int NOFS() const;//ȡ������վ������
    virtual double dist(int d, int a) const;//��·��վd��վa�ľ���
    virtual int& operator[](int x);//ȡ��ĳ��վ�ε�վ����
    virtual ~LINE() noexcept;//��������
};

class TRAN {//����һ��ת�ˣ���from��stop����to
    int from;//���ڳ����ı��
    int to;  //��Ҫת�˵ı��
    int stop;//��stops.txt�����վ����
public:
    TRAN(int from = 0, int to = 0, int stop = 0);//���캯��
    int operator==(const TRAN& t)const;//�ж��Ƿ���ͬ
    virtual int& F();//���ڳ�����
    virtual int& T();//��������Ҫ�����ĵ�
    virtual int& S();//վ����
};

class ROUTE {//һ��ת��·��
    TRAN* const tran;//��·���ϵ�����ת��վ��
    const int noft; //��·����ת�˴���
public:
    ROUTE(TRAN* tran = 0, int noft = 0);//���캯��
    ROUTE(const TRAN& t);//��һ��ת�˹���
    ROUTE(const ROUTE& r);//��ת��·���������
    ROUTE(ROUTE&& r) noexcept;//�ƶ�����

    virtual operator int() const;//ת�˴���
    virtual int operator-(const ROUTE& n) const;//ת��·������ת�˴���
    virtual ROUTE operator*() const;//ȥ���ظ��Ĺ���ת�˷���
    virtual TRAN& operator[](int);//һ��·�������еĻ���վ��
    virtual ROUTE operator+(const ROUTE& r) const;//ת��·������
    virtual ROUTE& operator=(const ROUTE& r);//�����ֵ
    virtual ROUTE& operator=(ROUTE&& r) noexcept;//�ƶ���ֵ
    virtual ROUTE& operator+=(const ROUTE& r);//·������
    virtual bool operator==(const ROUTE& r);//�ж�����·���Ƿ���ͬ
    virtual ~ROUTE() noexcept;//��������
    virtual int print() const;//��ӡת��·��
};

class ELEM {        //�հ�����Ԫ�أ����ص�ת�˴�������·
    ROUTE* const p; //�հ�����r*c��Ԫ�ؼ��ص���ת��·������
    int n;          //�հ�����r*c��Ԫ�ؼ��ص���ת��·��������
public:
    ELEM(ROUTE* p, int n);//���캯��
    ELEM(int n = 0);//ȱʡת�˷����Ĺ���
    ELEM(const ELEM& n);//�������
    ELEM(ELEM&& n) noexcept;//�ƶ�����
    virtual ELEM operator*() const;//ȥ��ת���еĻ�
    virtual ELEM operator+(const ROUTE& n) const;//Ԫ������·��
    virtual ELEM operator+(const ELEM& n) const;//Ԫ��·������
    virtual ELEM operator*(const ELEM& n) const;//Ԫ��·��ת������
    virtual ELEM& operator=(const ELEM& n);//�����ֵ
    virtual ELEM& operator+=(const ELEM& n);//Ԫ��·�����
    virtual ELEM& operator+=(const ROUTE& n);//Ԫ������·��
    virtual ELEM& operator*=(const ELEM& n);//Ԫ��·��ת������
    virtual ELEM& operator=(ELEM&& n) noexcept;//�ƶ���ֵ
    virtual ROUTE& operator[](int x);//��õ�x��ת��·��
    virtual operator int& ();//���ؿ�ת��·����
    virtual ~ELEM() noexcept;//��������
    virtual void print() const;//��ӡ����Ԫ��
};

class MAT {//���й���ת��Ԫ�صıհ�����
    ELEM* const p;//ָ��հ������r*c��Ԫ��
    const int r, c;//�հ����������������
public:
    MAT(int r = 0, int c = 0);//���캯��
    MAT(const MAT& a);//�������
    MAT(MAT&& a) noexcept;//�ƶ�����
    virtual ~MAT();//��������
    virtual bool notZero() const;//���ɴ�㷵��false
    //���b�յ�e��������ת�˵�·����
    virtual int miniTran(int b, int e, int& noft, ROUTE(&r)[100]) const;
    //���b�յ�e������̾����·����
    virtual int miniDist(int b, int e, double& dist, ROUTE(&r)[100]) const;
    static double getDist(int b, int e, ROUTE& r);//��b��e����·��r�ľ���
    virtual ELEM* operator[](int r);//ȡ����r�е��׵�ַ
    virtual int& operator()(int r, int c);//��r��c��ת��·����Ŀ
    virtual MAT operator*(const MAT& a) const;//�հ��˷�
    virtual MAT operator+(const MAT& a) const;//�հ��ӷ�
    virtual MAT& operator=(const MAT& a);//�����ֵ
    virtual MAT& operator=(MAT&& a);//�ƶ���ֵ
    virtual MAT& operator+=(const MAT& a);//�հ��ӷ�
    virtual MAT& operator*=(const MAT& a);//�հ��˷�
    virtual MAT& operator()(int r, int c, const ROUTE& a);//��·��a�������Ԫ��
    virtual void print() const;//���ת�˾���
};

class Organization {//����һ����֯
private:
    QString orgname;//��֯��		
    int X;//X����
    int Y;//Y����
public:
    Organization(QString& org1, int x1, int y1);
    Organization() = default;//ʹ��Ĭ���޲ι��캯������������
    ~Organization() = default;
    const QString& org_name();		//������֯��
    int& x();		//����X	
    int& y();		//����Y
};

struct GIS {//����������Ϣϵͳ
//Ĭ��public
    static STOP* st;//���й���վ��
    static LINE* ls;//���й�����·
    static int ns;//����վ��
    static int nl;//������·��
    static MAT raw;//ԭʼת�˾���raw��
    static MAT tra;//�հ�ת�˾���tra
    static int obs;//GIS�Ķ�������
    int org_num=0;//��������
    std::vector<Organization>orgs;//��¼����
    GIS();//���캯��
    GIS(const char* flstop, const char* flline, const char* florg);//��վ�㼰��·�ļ����ص�ͼ
    int miniTran(int fx, int fy, int tx, int ty, int& f, int& t, int& n, ROUTE(&r)[100]);//����ת��
    int miniDist(int fx, int fy, int tx, int ty, int& f, int& t, double& d, ROUTE(&r)[100]);//��̾���
    ~GIS();//��������
};
extern GIS* gis;
