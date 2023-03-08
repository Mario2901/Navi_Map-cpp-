#define _CRT_SECURE_NO_WARNINGS
#include "routing.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QDebug>
using namespace std;

STOP* GIS::st = 0; //����վ
LINE* GIS::ls = 0; //������·
int GIS::ns = 0;   //����վ��
int GIS::nl = 0;   //������·��
int GIS::obs = 0;  // GIS��������
MAT GIS::raw;     //ԭʼת��ͼ
MAT GIS::tra;     //�հ�ת��ͼ

//##################STOP���ʵ��########################

//���캯��
STOP::STOP(int n, int x, int y)
{
    this->numb = n;
    this->x = x;
    this->y = y;
}
//��ȡx����
int& STOP::X()
{
    return x;
}
//��ȡy����
int& STOP::Y()
{
    return y;
}
//��ȡ����վ���
int& STOP::N()
{
    return numb;
}


//#################LINE���ʵ��###################

//���캯��
LINE::LINE(int n, int c, int* stop) : numb(n), stop(c ? new int[c] : nullptr), nofs(stop ? c : 0)
{
    if (LINE::stop == nullptr && nofs != 0)
        throw "Memory allocation for bus line error!";
    for (int x = 0; x < nofs; x++)
        LINE::stop[x] = stop[x];
}
//�������
LINE::LINE(const LINE& r) : numb(r.numb), stop(r.nofs ? new int[r.nofs] : nullptr), nofs(stop ? r.nofs : 0)
{
    if (stop == nullptr && r.nofs != 0)
        throw "Memory allocation for bus line error!";
    for (int x = 0; x < nofs; x++)
        stop[x] = r.stop[x];
}
//�ƶ�����
LINE::LINE(LINE&& r) noexcept : numb(r.numb), stop(r.stop), nofs(r.nofs)
{
    (int&)(r.numb) = (int&)(r.nofs) = 0;
    (int*&)(r.stop) = nullptr;
}
//�����ֵ
LINE& LINE::operator=(const LINE& r)
{
    if (this == &r) //��ֹ�Կ���
        return *this;
    if (stop) //���������ɾ��
    {
        delete[] stop;
    }
    (int&)numb = r.numb;
    (int&)nofs = r.nofs;
    (int*&)stop = new int[r.nofs];
    if (stop == 0)
        throw "memory allocation error";
    for (int i = 0; i < nofs; i++) //����ֵ����
        this->stop[i] = r.stop[i];
    return *this;
}
//�ƶ���ֵ
LINE& LINE::operator=(LINE&& r) noexcept
{
    if (this == &r) //��ֹ�Կ���
        return *this;
    if (stop) //���������ɾ��
    {
        delete[] stop;
    }
    (int&)numb = r.numb;
    (int&)nofs = r.nofs;
    (int*&)stop = r.stop;
    (int&)r.numb = 0;
    (int&)r.nofs = 0;
    (int*&)r.stop = 0;
    return *this;
}
//�Ƿ����վ����s�������򷵻ذ����ڼ�վ�Σ�-1����δ����
int LINE::has(int s) const
{
    for (int x = 0; x < nofs; x++)
        if (stop[x] == s)
            return x;
    return -1;
}
//�ж�������·�ཻ���ཻ����1
int LINE::cross(const LINE& b) const
{ //��·�ཻ�򷵻�1
    if (this == &b)
        return 0;
    for (int x = 0; x < nofs; x++)
        if (b.has(stop[x]) != -1)
            return 1;
    return 0;
}

//ȡ��·���
LINE::operator int() const
{
    return numb;
}
//ȡ������վ������
int LINE::NOFS() const
{
    return nofs;
}
//��·��վd��վa�ľ���
double LINE::dist(int b, int e) const
{
    double d;
    int bi, ei; //����վ��Ŀ�ʼ���������
    int x1, y1, x2, y2, w;
    if ((bi = has(b)) == -1)
        throw "Wrong stop number while calculating distance!";
    if ((ei = has(e)) == -1)
        throw "Wrong stop number while calculating distance!";
    if (bi > ei)
    {
        b = ei;
        ei = bi;
        bi = b;
    }
    d = 0;
    for (b = bi; b < ei; b++)
    {
        x1 = GIS::st[stop[b]].X();
        y1 = GIS::st[stop[b]].Y();
        x2 = GIS::st[stop[b + 1]].X();
        y2 = GIS::st[stop[b + 1]].Y();
        d += sqrt(1.0*((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1)));
    }
    return d;
}
//ȡ����xվ��վ����
int& LINE::operator[](int x)
{
    if (x >= 0)
        return stop[x];
}
//��������
LINE::~LINE() noexcept
{
    if (stop != 0)
    {
        delete[] stop;
    }
    (int*&)stop = 0;
    (int&)numb = 0;
    (int&)nofs = 0;
}
//######################TRAN���ʵ��###########################

//���캯��
TRAN::TRAN(int from, int to, int stop) : from(from), to(to), stop(stop) 
{

}
//�ж��Ƿ����
int TRAN::operator==(const TRAN& t) const
{
    if(from == t.from && to == t.to && stop == t.stop)
        return 1;
    return 0;
}
int& TRAN::F() { return from; }
int& TRAN::T() { return to; }
int& TRAN::S() { return stop; }

//######################ROUTE���ʵ��###########################

//���캯��
ROUTE::ROUTE(TRAN* tran, int noft) : tran(noft ? new TRAN[noft] : nullptr), noft(noft)
{
    if (this->tran == 0 && noft != 0)
        throw "memory allocation error!";
    for (int i = 0; i < noft; i++)
        this->tran[i] = tran[i];
}
//�õ���ת�˹���ROUTE
ROUTE::ROUTE(const TRAN& t) : tran(new TRAN[1]), noft(0)
{
    if (tran == 0)
        throw "memory allocation error!";
    if (tran)
        *tran = t;
}
//�������
ROUTE::ROUTE(const ROUTE& r) : tran(r.noft ? new TRAN[r.noft] : 0), noft(r.noft)
{
    if (tran == 0 && r.noft != 0)
        throw "memory allocation error!";
    for (int i = 0; i < noft; i++)
        tran[i] = r.tran[i];
}
//�ƶ�����
ROUTE::ROUTE(ROUTE&& r) noexcept : tran(r.tran), noft(r.noft)
{
    (TRAN*&)(r.tran) = 0;
    (int&)(r.noft) = 0;
}
//��������
ROUTE::~ROUTE() noexcept
{
    if (tran)
    {
        delete[] tran;
        *(int*)&noft = 0;
        (TRAN*&)tran = 0;
    }
}
//��ӡת��·��,����ת�˴���
int ROUTE::print() const
{
    for (int i = 0; i < noft; i++)
    {
        if (tran[i].S() == -1)
            cout << "���� ����" << tran[i].F() + 1 << "· ֱ��" << tran[i].S() << endl;
        else
            cout << "���� ����" << tran[i].F() + 1 << "· ��" << tran[i].S() << "���� " << tran[i].T() << "· ������" << endl;
    }
    return noft;
}
//ת�˴���
ROUTE::operator int() const
{
    return noft;
}
//ȥ���ظ�ת��
ROUTE ROUTE::operator*() const
{
    int nn = noft;
    TRAN* t = new TRAN[noft];//��ʱ����
    if (t == 0)
        throw "memory allocation error!";
    for (int i = 0; i < nn; i++)
        t[i] = tran[i];
    for (int i = 0; i < nn - 1; i++)
        for (int j = i + 1; j < nn; j++)
        {
            if (t[i].S() == t[j].S() && t[i].F() == t[j].T())//������·����ѭ��,�ص�֮ǰ�����ĳ�
            {
                for (int k = i, n = j + 1; n < nn; n++, k++)
                    t[k] = t[n];//ȫ��ǰ��
                nn -= (j + 1 - i);//������·֮����ǻ��ߵ���ɲ���,ȫ��ɾ��
                j = i;//ȥ���������Ҫ���¿�ʼ��ֹ�������ɵ�ѭ��δ������
            }
        }
    ROUTE r(t, nn);
    delete[] t;
    return r;
}

//ת��·������ת�˴���
int ROUTE::operator-(const ROUTE& n) const
{
    return noft - n.noft;
}
//�ж�����·���Ƿ���ͬ
bool ROUTE::operator==(const ROUTE& r)
{
    if (r.noft == noft)
    {
        for (int i = 0; i < noft; i++)
        {
            if (!(tran[i] == r.tran[i]))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
//һ��·�������еĻ���վ��
TRAN& ROUTE::operator[](int x)
{
    if (x < 0 || x > noft)
        throw "out of index!";
    return tran[x];
}
//ת��·������
ROUTE ROUTE::operator+(const ROUTE& r) const
{
    if (noft == 0)
        return *this;
    if (r.noft == 0)
        return r;
    if (tran[noft - 1].T() != r.tran[0].F())//��β������
        throw "can not be connected!";
    ROUTE s;
    (TRAN*&)(s.tran) = new TRAN[noft + r.noft];
    if(!s.tran)
        throw "memory allocation error!";
    (int&)(s.noft) = noft + r.noft;
    int x,y;
    for (x = 0; x < noft; x++)
        s.tran[x] = tran[x];
    for (y = 0; y < r.noft; y++)
        s.tran[x++] = r.tran[y];
    return *s;
}
//�����ֵ
ROUTE& ROUTE::operator=(const ROUTE& r)
{
    if (this == &r)
        return *this;
    if (tran)
        delete[] tran;
    (TRAN*&)tran = new TRAN[r.noft];
    if(!tran)
        throw "Memory allocation error!";
    (int&)noft = r.noft ;
    for (int i = 0; i < noft; i++)
        tran[i] = r.tran[i];
    return *this;
}
//�ƶ���ֵ
ROUTE& ROUTE::operator=(ROUTE&& r) noexcept
{
    if (this == &r)
        return *this;
    if (tran)
        delete[] tran;
    (TRAN*&)tran = r.tran;
    (int&)noft = r.noft;
    (TRAN*&)(r.tran) = 0;
    (int&)(r.noft) = 0;
    return *this;
}
//·������
ROUTE& ROUTE::operator+=(const ROUTE& r)
{
    return *this = *this + r;
}


//######################ROUTE���ʵ��###########################

//���캯��
ELEM::ELEM(ROUTE* p, int n) : p(n > 0 ? new ROUTE[n] : 0), n(n)
{
    if (this->p == 0 && n != 0)
        throw "memory allocation error!";
    for (int i = 0; i < n; i++)
        this->p[i] = p[i];
}
//ȱʡת�˷����Ĺ���
ELEM::ELEM(int n) : p(n>0 ? new ROUTE[n] : 0), n(n)
{
    if (p == 0 && n != 0)
        throw "memory allocation error!";
}
//�������
ELEM::ELEM(const ELEM& n) : p(n.n > 0 ? new ROUTE[n.n] : 0), n(n.n)
{
    if (p == 0 && n.n != 0)
        throw "memory allocation error!";
    for (int i = 0; i < this->n; i++)
        p[i] = n.p[i];
}
//�ƶ�����
ELEM::ELEM(ELEM&& n) noexcept : p(n.p), n(n.n)
{
    (ROUTE*&)(n.p) = 0;
    (int&)n.n = 0;
}
//ȥ��ת���еĻ�
ELEM ELEM::operator*() const
{
    int nn = this->n;
    if (nn == 0)
        return *this;
    ROUTE* t = new ROUTE[nn];
    if (t == 0)
        throw "memory allocation error!";
    for (int x = 0; x < nn; x++)
        t[x] = p[x];
    for (int x = 0; x < nn - 1; x++)
        for (int y = x + 1; y < nn; y++)
        {
            if (t[x] == t[y])//���ڻ�
            {
                for (int k = x + 1, j = y + 1; j < nn - 1; j++, k++)
                    t[k] = t[j];
                nn -= (y - x);//�����ϵ�����Ԫ��ȥ��
                y = x;
            }
        }
    ELEM r(t, nn);
    if (t != 0)//ɾ����ʱ����
        delete[] t;
    t = 0;
    return r;
}
//Ԫ������·��
ELEM ELEM::operator+(const ROUTE& n) const
{
    ELEM r(this->n + 1);
    for (int i = 0; i < this->n; i++)
        r.p[i] = *p[i];
    r.p[this->n] = n;
    return *r;
}
//Ԫ��·������
ELEM ELEM::operator+(const ELEM& n) const
{
    if (this->n == 0)
        return n;
    if (n.n == 0)
        return *this;
    ELEM r(this->n + n.n);//�����µ�NODE,·��Ϊ�������
    for (int i = 0; i < this->n; i++)
        r.p[i] = *p[i];//���������
    for (int i = 0; i < n.n; i++)
        r.p[i + this->n] = *n.p[i];//��n�е�Ԫ�ؿ���
    return *r;
}
//Ԫ��·��ת������
ELEM ELEM::operator*(const ELEM& n) const
{
    if (this->n == 0)
        return *this;
    if (n.n == 0)
        return n;
    ELEM r(this->n * n.n);
    int cnt = 0;
    for (int i = 0; i < this->n; i++)
        for (int j = 0; j < n.n; j++)
        {
            if (p[i][-1 + p[i]].T() != n.p[j][0].F())
                throw "be a circle!";
            try
            {
                r.p[cnt] = p[i] + n.p[j]; //·������
            }
            catch (const char* e)
            {
                const char* p = e;
            }
            cnt++;
        }
    return *r;
}
//�����ֵ
ELEM& ELEM::operator=(const ELEM& n)
{
    if (this == &n)//��ֹ�Ը�ֵ
        return *this;
    if (p)
        delete[] p;
    (ROUTE*&)p = new ROUTE[n.n];
    if (p == 0)
        throw "memory allocation error!";
    (int&)(this->n) = p ? n.n : 0;
    for (int i = 0; i < n.n; i++)//ֵ����
        p[i] = n.p[i];
    return *this;
}
//�ƶ���ֵ
ELEM& ELEM::operator=(ELEM&& n) noexcept
{
    if (this == &n)//��ֹ�Ը�ֵ
        return *this;
    if (p)
        delete[] p;
    (ROUTE*&)p = n.p;
    (int&)(this->n) = n.n;
    (ROUTE*&)(n.p) = 0;
    (int&)(n.n) = 0;
    return *this;
}
//Ԫ������·��
ELEM& ELEM::operator+=(const ROUTE& n)
{
    return *this = *this + n;
}
//Ԫ��·��ת������
ELEM& ELEM::operator+=(const ELEM& n)
{
    return *this = *this + n;
}
//�ƶ���ֵ
ELEM& ELEM::operator*=(const ELEM& n)
{
    return *this = *this * n;
}
//��õ�x��ת��·��
ROUTE& ELEM::operator[](int x)
{
    if (x < 0 || x >= n)
        throw "error input of index!";
    return p[x];
}
//ȡ����������
ELEM::operator int& ()
{
    return n;
}
//��������
ELEM::~ELEM() noexcept
{
    if (p)
    {
        delete[] p;
        (ROUTE*&)p = 0;
        (int&)n = 0;
    }
}
//�������
void ELEM::print() const
{
    for (int i = 0; i < n; i++)
    {
        p[i].print();
    }
}


//######################TMAP���ʵ��###########################

//���캯��
MAT::MAT(int r, int c) : p((r > 0 && c > 0) ? new ELEM[r * c] : 0), r( r), c(c)
{
    if (this->p == 0 && r > 0 && c > 0)
        throw "memory allocation error!";
}
//�������
MAT::MAT(const MAT& a) : p((a.r > 0 && a.c > 0) ? new ELEM[a.r * a.c] : 0), r(a.r), c(a.c)
{
    if (p == 0 && a.r > 0 && a.c > 0)
        throw "memory allocation error!";
    for (int i = r * c - 1; i >= 0; i--)
        p[i] = a.p[i];
}
//�ƶ�����
MAT::MAT(MAT&& a) noexcept : p(a.p), r(a.r), c(a.c)
{
    (ELEM*&)(a.p) = 0;
    (int&)(a.r) = (int&)(a.c) = 0;
}
//��������
MAT::~MAT()
{
    if (p)
    {
        delete[] p;
        (ELEM*&)p = 0;
        (int&)r = (int&)c = 0;
    }
}
//���ɴ�㷵��false
bool MAT::notZero() const
{
    for (int i = r * c - 1; i >= 0; i--)
        if (p[i].operator int& () == 0)
            return 0;
    return 1;
}
//��r��c��ת��·����Ŀ
int& MAT::operator()(int x, int y)
{
    if (x < 0 || x >= r||y<0||y>=c)
        throw "out of index!";
    return p[x * c + y];
}
//ȡ����r�е��׵�ַ
ELEM* MAT::operator[](int r)
{
    if (r < 0 || r >= this->r)
        throw "out of index!";
    return p + r * c;
}
//�����ֵ
MAT& MAT::operator=(const MAT& a)
{
    if (this == &a)//��ֹ�Ը�ֵ
        return *this;
    if (p)
        delete[] p;
    (ELEM*&)p = new ELEM[a.r * a.c];
    if (p == 0)
        throw "memory allocation error!";
    (int&)r = a.r;
    (int&)c = a.c;
    for (int i = r * c - 1; i >= 0; i--)
        p[i] = a.p[i];
    return *this;
}
//�ƶ���ֵ
MAT& MAT::operator=(MAT&& a)
{
    if (this == &a)//��ֹ�Ը�ֵ
        return *this;
    if (p)//������ɾ��
        delete[] p;
    (ELEM*&)p = a.p;
    (int&)r = a.r;
    (int&)c = a.c;
    (ELEM*&)(a.p) = 0;
    (int&)(a.r) = 0;
    (int&)(a.c) = 0;
    return *this;
}
//�հ��˷�
MAT MAT::operator*(const MAT& a) const
{
    if (c != a.r)
        throw "can not mutiply!";
    int t=0;
    MAT s(r, a.c);//��Ҫ���صı���,�ɳ˷�������Ӧ��ֵ
    for (int h = 0; h < r; h++)//���վ���˷�
    {
        for (int j = a.c - 1; j >= 0; j--)
        {
            if (h == j) 
                continue;
            t = h * s.c + j;
            for (int k = 0; k < c; k++)
                if (k != h && k != j)
                    s.p[t] += p[h * c + k] * a.p[k * a.c + j];
        }
    }
    return s;
}
//�հ��ӷ�
MAT MAT::operator+(const MAT& a) const
{
    if (r != a.r && c != a.c)
        throw "can not not add!";
    MAT s(*this);
    for (int i = r * c - 1; i >= 0; i--)
        s.p[i] += a.p[i];
    return s;
}
//�ӷ�
MAT& MAT::operator+=(const MAT& a)
{ 
    return *this = *this + a;
}
//�˷�
MAT& MAT::operator*=(const MAT& a)
{ 
    return *this = *this * a;
}
//��·��a�������Ԫ��
MAT& MAT::operator()(int ro, int co, const ROUTE& a)
{
    p[ro * c + co] += a;
    return *this;
}
//���b�յ�e��������ת�˵�·����
int MAT::miniTran(int s, int t, int& notf, ROUTE(&r)[100]) const
{
    int b = 0, e = 0;//�����յ�
    int num_tran[100];//��Ӧ��ת��·����
    int bls[20], els[20];//blsΪ��������·����,elsΪ�����յ��·����
    ELEM rou;
    for (int i = 0; i < GIS::nl; i++)//Ѱ�Ұ��������յ���ع�����·�±�
    {
        if (GIS::ls[i].has(s) != -1)//����
            if (b < 20)
                bls[b++] = i;
        if (GIS::ls[i].has(t) != -1)
            if (e < 20)
                els[e++] = i;
    }
    int z = 0,u,v,w;
    for (int i = 0; i < b; i++)
        for (int j = 0; j < e; j++)
        {
            rou = GIS::tra[bls[i]][els[j]];//�õ�����·������ת����·
            w = GIS::tra(bls[i], els[j]);
            if (w == 0)//ת����·����Ϊ0
                continue;
            for (v = 0; v < w; v++)
            {
                u = rou[v]; //�õ�ת�˴���
                if (z == 0 || u < num_tran[0])//��nottΪ�գ���ת�˴�����nott[0]С
                {
                    num_tran[0] = u;
                    r[0] = rou[v];
                    z = 1;
                }
                if (u == num_tran[0])//��������·ת�˴�����ͬʱ������
                {
                    if (z == 100)
                        return z;
                    num_tran[z] = u;
                    r[z] = rou[v];
                    z++;
                }
            }
        }
    notf = num_tran[0];
    return z;
}
//��b��e����·��r�ľ���
double MAT::getDist(int b, int e, ROUTE& r)
{
    double d = 0;
    if ((int)r==1 && r[0].F() == r[0].T())//һ��ת�˻������ڳ���������Ҫת�˵�,����Ҫ����
    {
        d = GIS::ls[r[0].F()].dist(b, e);//һ������ֱ�Ӽ������
        return d;
    }
    d = GIS::ls[r[0].F()].dist(b, r[0].S());
    int y = (int)r - 1;//��¼ת�˵Ĵ���
    for (int i = 0; i < y; i++)
        d += GIS::ls[r[i].T()].dist(r[i].S(), r[i + 1].S());//�����ߵķֶξ���֮��
    d += GIS::ls[r[y].T()].dist(r[y].S(), e);
    return d;
}
//���b�յ�e������̾����·����
int MAT::miniDist(int s, int t, double& dist, ROUTE(&r)[100]) const
{
    int b = 0, e = 0;//�����յ�
    int dis[100];//��Ӧ��ת�˾���
    int bls[20], els[20];//blsΪ��������·����,elsΪ�����յ��·����
    ELEM rou;
    for (int i = 0; i < GIS::nl; i++)//Ѱ�Ұ��������յ���ع�����·�±�
    {
        if (GIS::ls[i].has(s) != -1)//����
            if (b < 20)
                bls[b++] = i;
        if (GIS::ls[i].has(t) != -1)
            if (e < 20)
                els[e++] = i;
    }
    int z = 0, u, w;
    for (int i = 0; i < b; i++)
        for (int j = 0; j < e; j++)
        {
            rou = GIS::tra[bls[i]][els[j]];//�õ�����·������ת����·
            w = GIS::tra(bls[i], els[j]);
            if (w == 0)//ת����·����Ϊ0
                continue;
            for (int i = 0; i < w; i++)
            {
                u = MAT::getDist(s, t, rou[i]);//ת�˾���
                if (z == 0 || u < dis[0])//��nottΪ�գ���ת�˴�����nott[0]С
                {
                    dis[0] = u;
                    r[0] = rou[i];
                    z = 1;
                }
                if (u == dis[0])//��������·ת�˴�����ͬʱ������
                {
                    if (z == 100)//����
                        return z;
                    dis[z] = u;
                    r[z] = rou[i];
                    z++;
                }
            }
        }
    dist = dis[0];
    return z;
}
//���ת�˾���
void MAT::print() const
{
    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            cout << "��" << i << "��" << j << "��" << (int)p[i * c + j] << "��·��" << endl;
        }
    }
}

//######################Organization���ʵ��###########################
//���캯��
Organization::Organization(QString& loc1, int x1, int y1):orgname(loc1), X(x1), Y(y1)
{
    //�յ�
}
//ȡ������
const QString& Organization::org_name()
{
    return orgname;
}
//ȡx����
int& Organization::x()
{ 
    return X;
}
//ȡy����
int& Organization::y()
{
    return Y;
}


//######################GIS���ʵ��###########################

//���캯��
GIS::GIS()
{ 
    obs++; 
}
//��վ�㼰��·�ļ����ص�ͼ
GIS::GIS(const char* flstop, const char* flline, const char* florg)
{
    FILE* fs, * fl;
    fs = fopen(flstop, "r");
    fl = fopen(flline, "r");
    if (fs == 0 || fl == 0)
        throw "File open error!";
    fscanf(fs, "%d", &ns);
    st = new STOP[ns];
    int m, n, p, q, r;
    int* s, * t;
    for (m = 0; m < ns; m++)
    {
        fscanf(fs, "%d%d", &st[m].X(), &st[m].Y());
        st[m].N() = m + 1;//������·�ı�Ŵ�1��ʼ
    }
    fclose(fs);
    fscanf(fl, "%d", &nl);
    s = new int[nl];//ÿ����·��վ����
    t = new int[100];//ÿ����·��վ����������Ϊ100
    for (m = 0; m < nl; m++)//����վ��
    {
        fscanf(fl, "%d", &s[m]);
    }
    *(LINE**)&ls = new LINE[nl];
    for (m = 0; m < nl; m++)//����վ����
    {
        for (n = 0; n < s[m]; n++)
        {
            fscanf(fl, "%d", &t[n]);
            t[n]--;
        }
        ls[m] = LINE(m + 1, s[m], t);
    }
    fclose(fl);
    //��ȡ��֯
    QFile f(florg);
    if (!f.open(QIODevice::ReadOnly))  //���ļ�ʧ��
        throw "error";
    QTextCodec* code = QTextCodec::codecForName("utf8");//��ֹ����
    QTextStream fino(&f);
    fino.setCodec(code);
    while (!fino.atEnd()) {
        QString s, t;
        int x, y;
        fino >> s >> x >> t >> y;
        if (s.isEmpty()) { org_num = orgs.size(); break; }
        orgs.push_back({ s,x,y });
    }
    f.close();
    for (m = 0; m < nl; m++)//����raw
    {
        for (p = 0,n = 0; n < nl; n++)
            if (m != n)
                p += GIS::ls[m].cross(GIS::ls[n]);
        if (p == 0)
        {
            throw "there is an independent line";
        }
    }
    MAT ra(nl, nl);
    ROUTE a;
    TRAN* u = new TRAN[100];//��ʱ����
    for (m = 0; m < nl; m++)
        for (n = 0; n < nl; n++)
        {
            if (m == n)
            {
                u[0] = TRAN(m, n, -1);//�����������һ��ת��
                a = ROUTE(&u[0], 1);//ֻ��һ��ת��
                ra(m, n, a);
                continue;
            }
            p = 0; //������·�������
            for (q = GIS::ls[m].NOFS() - 1; q >= 0; q--)
            {
                r = GIS::ls[m][q];
                if (GIS::ls[n].has(r) != -1)//ÿ���������һ����·�߼�ת��
                {
                    u[p] = TRAN(m, n, r);
                    a = ROUTE(&u[p++], 1);//ֻ��һ��ת��
                    ra(m, n, a);
                }
            }
        }
    tra = raw = ra;
    for (n = 2; n < nl; n++)//�������հ�
    {
        raw *= ra;
        tra += raw;
    }
    raw = ra;
    delete[]s;
    delete[]t;
    delete[] u;
    obs++;
}

//����ת��
int GIS::miniTran(int fx, int fy, int tx, int ty, int& f, int& t, int& n, ROUTE(&r)[100])
{
    int m;
    f = 0; //������������վ��Ϊf
    double df = sqrt((st[0].X() - fx) * (st[0].X() - fx) + (st[0].Y() - fy) * (st[0].Y() - fy));
    t = 0; //�����յ������վ��Ϊt
    double dt = sqrt((st[0].X() - tx) * (st[0].X() - tx) + (st[0].Y() - ty) * (st[0].Y() - ty));
    double tf, tt;
    for (m = 1; m < GIS::ns; m++)//�����������յ������վ��
    {
        tf = sqrt((st[m].X() - fx) * (st[m].X() - fx) + (st[m].Y() - fy) * (st[m].Y() - fy));
        if (df > tf)//����������
        {
            df = tf;
            f = m;
        }
        tt = sqrt((st[m].X() - tx) * (st[m].X() - tx) + (st[m].Y() - ty) * (st[m].Y() - ty));
        if (dt > tt)
        {
            dt = tt;
            t = m;
        }
    }
    if (f == t)//�����յ���ͬ�����˳�
        return 0;
    return GIS::tra.miniTran(f, t, n, r);
}
//��̾���
int GIS::miniDist(int fx, int fy, int tx, int ty, int& f, int& t, double& d, ROUTE(&r)[100])
{
    int m;
    double df, tf, dt, tt;
    f = 0;
    df = sqrt((st[0].X() - fx) * (st[0].X() - fx) + (st[0].Y() - fy) * (st[0].Y() - fy));
    t = 0;
    dt = sqrt((st[0].X() - tx) * (st[0].X() - tx) + (st[0].Y() - ty) * (st[0].Y() - ty));
    for (m = 1; m < GIS::ns; m++)
    {
        tf = sqrt((st[m].X() - fx) * (st[m].X() - fx) + (st[m].Y() - fy) * (st[m].Y() - fy));
        if (df > tf)
        {
            df = tf;
            f = m;
        }
        tt = sqrt((st[m].X() - tx) * (st[m].X() - tx) + (st[m].Y() - ty) * (st[m].Y() - ty));
        if (dt > tt)
        {
            dt = tt;
            t = m;
        }
    }
    if (f == t)//�����յ���ͬ�����˳�
        return 0;
    return GIS::tra.miniDist(f, t, d, r);
}
//��������
GIS::~GIS()
{
    obs--;
    if (obs)//�Ѿ�û�ж�����
        return;
    if (st)
    {
        delete[] st;
        *(STOP**)&st = 0;
    }
    if (ls)
    {
        delete[] ls;
        *(LINE**)&ls = 0;
    }
}