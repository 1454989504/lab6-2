#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "dwgline.h"
#include "dwgcircle.h"
#include "dwgrectangle.h"
#include "dwgpoint.h"
#include <QWheelEvent>
#include <QDebug>
#include <dwgobject.h>>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_trans.reset();                    // 先把系统的变换矩阵置为空
    m_trans.translate(0,this->height());       // 再平移坐标原点到窗口左下角
    m_trans.scale(1,-1);                    // 对坐标系的Y轴实施反向变换
    this->setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    // 初始化 mObjVec
    mObjVec = QVector<DwgObject*>();
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QMessageBox::information(this,"Open Menu Clicked!","OK");
}

bool isfirst = true;
QPoint firstpt,lastpt;
void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter *mpainter = new QPainter(this);
    if(!(firstpt.isNull() || lastpt.isNull())  &&  ! isfirst)
    {
        QPen pen;							// 由于橡皮线一般是虚线，因此，需要先构建一支虚线的笔
        pen.setWidth(0);						// 为提高速度，把笔的线宽设置为0
        pen.setColor(QColor(255,0,0));			// 设置一个红色的线
        pen.setStyle(Qt::DashLine);              // 把线型设置为虚线
        m_painter->setPen(pen);				// 把定义好的虚线赋值给当前绘图工具

        if(curDrawType == DLine)			// 判断是否正绘制直线
            m_painter->drawLine(firstpt,lastpt);    // 绘制一条虚线
    }
    m_painter->end();						// 一定不要忘记，先结束临时的画笔样式

    // 下面开始绘制主顺序链表中的已添加图元
    m_painter->begin(this);					// 重新开始绘图
    if(!m_mainlist.empty())
        for(DwgObject *i :m_mainlist)
            i->Draw(m_painter);
    m_painter->end();
    mpainter->setWorldTransform(m_trans,true);
    foreach (auto i, mObjVec) {
        i->Draw(mpainter);
    }
    for(auto& i : m_mainlist)
        i->Draw(mpainter);
    delete mpainter;
}

void MainWindow::on_actionLine_triggered()
{
    curDrawType = DLine;
    setCursor(QCursor(Qt::CrossCursor));    // 设置光标类型为十字交叉
}
void MainWindow::on_actionCircle_triggered()
{
    curDrawType = DCircle;
    setCursor(QCursor(Qt::CrossCursor));    // 设置光标类型为十字交叉
}
void MainWindow::on_actionRectangle_triggered()
{
    curDrawType = DRectangle;
    setCursor(QCursor(Qt::CrossCursor));    // 设置光标类型为十字交叉
}
void MainWindow::on_actionPoint_triggered()
{
    curDrawType = DPoint;
    setCursor(QCursor(Qt::CrossCursor));    // 设置光标类型为十字交叉
}
void MainWindow::on_actionNew_triggered()
{
    qDeleteAll(m_mainlist);     // 删除所有的图元对象
    m_mainlist.clear();		 //  清空m_mainlist
    update();
    ui->statusbar->showMessage("开始绘制一张新的图纸：",3000);
}
void MainWindow::on_actionExit_triggered()
{
    qDeleteAll(m_mainlist);     // 删除所有的图元对象
    m_mainlist.clear();		 //  清空m_mainlist
    this->close();
}

bool isFirst=true;                  // 用于判断鼠标点击行为是第一点还是第二点
QPoint temppt;
            // 用于临时保存鼠标点击坐标点

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)					// 判断是否是鼠标左键按下
    {
    if(curDrawType == DPoint)                    // 判断当前绘制对象为点
    {
        DwgPoint* temppt = new DwgPoint;
        temppt->mPoint = event->pos(); // 或者使用 SetData 方法
        m_mainlist.append(temppt);
    }
    if(curDrawType == DLine)					// 判断当前绘制对象为直线
    {
        if(isFirst)
        {   firstpt = event->pos();
            isFirst = false;
        }
        else
        {   lastpt = event->pos();
            DwgLine *templine = new DwgLine;
            // 将坐标映射回原始坐标系
            QPoint pt1 = m_trans.inverted().map(firstpt);
            QPoint pt2 = m_trans.inverted().map(event->pos());
            templine->SetData(pt1, pt2); // 使用映射后的坐标设置私有成员变量
            m_mainlist.append(templine);
            isFirst = true;
        }
    }
    if(curDrawType == DCircle)
    {
        if(isFirst)
        {
            firstpt = event->pos();
            isFirst = false;
        }
        else
        {
            lastpt = event->pos();
            DwgCircle *circle = new DwgCircle;            // 创建并初始化一个直线对象
            QPoint pt1 = m_trans.inverted().map(firstpt);
            QPoint pt2 = m_trans.inverted().map(event->pos());
            circle->SetData(pt1,pt2);		// 使用SetData函数设置私有成员变量
            m_mainlist.append(circle);                     // append函数用于把对象添加到数组mLines中
            update();                                   // 该函数用于刷新窗口
            isFirst = true;
        }
    }
    if(curDrawType == DRectangle)
    {
        if(isFirst)
        {
            temppt = event->pos();
            isFirst = false;
        }
        else
        {
            DwgRectangle *Rectangle = new DwgRectangle;            // 创建并初始化一个直线对象
            Rectangle->SetData(temppt,event->pos());		// 使用SetData函数设置私有成员变量
            m_mainlist.append(Rectangle);                     // append函数用于把对象添加到数组mLines中
            update();                                   // 该函数用于刷新窗口
            isFirst = true;
        }
    }
    }
    update();
}
void MainWindow::wheelEvent(QWheelEvent *event)
{
    m_winScale = m_winScale +(event->angleDelta().y()/120.0/10);

    // 下面的代码用于计算m_trans
    qreal winScale = 1+event->angleDelta().y()/120.0/10;
    QTransform trans1,trans2,trans3;
    trans1.translate(-event->position().x(),-event->position().y());
    trans2.scale(winScale,winScale);
    trans3.translate(event->position().x(),event->position().y());
    m_trans = m_trans*trans1*trans2*trans3;

    qDebug() <<  m_trans;
    update();
}
QPoint movept ;   // 用于平移窗口时的临时参考点
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{

    if(!movept.isNull())
        if(event->buttons() == Qt::MiddleButton)      // 判断鼠标中键是否按下
            m_trans.translate(event->pos().x() - movept.x()  ,
                              movept.y() - event->pos().y() );
    movept = event->pos();
    if(!isfirst)
    {   lastpt = event->pos();
        update();
    }

    QMainWindow::mouseMoveEvent(event);			// 子部件处理完，把事件传递给父类
    update();
}
