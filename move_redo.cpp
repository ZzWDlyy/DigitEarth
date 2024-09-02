#include "earth_map_demo.h"
#include <QMouseEvent>
#define MARGIN 2 

//��갴���¼�
/*
 *���ã�
 *1.�ж��Ƿ�ʱ������ _isleftpressed
 *2.��ȡ�������Ļ�е�λ�� _plast
 *3.�������ʱ����������� _curpos
 */
void earth_map_demo::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (event->button() == Qt::LeftButton)
    {
        this->_isleftpressed = true;
        QPoint temp = event->globalPos();
        _plast = temp;
        _curpos = countFlag(event->pos(), countRow(event->pos()));
    }
}

//����ͷ��¼�
/*
 *���ã�
 *1.��_isleftpressed ��Ϊfalse
 *2.�������ʽ�ָ�ԭ��ʽ  setCursor(Qt::ArrowCursor);
 */
void earth_map_demo::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (_isleftpressed)
        _isleftpressed = false;
    setCursor(Qt::ArrowCursor);
}

//����ƶ��¼� 
void earth_map_demo::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (this->isFullScreen()) return;	//��������ȫ����ֱ�ӷ��أ������κβ���
    int poss = countFlag(event->pos(), countRow(event->pos()));
    setCursorType(poss);
    if (_isleftpressed)//�Ƿ����
    {
        QPoint ptemp = event->globalPos();
        ptemp = ptemp - _plast;
        if (_curpos == 22)//�ƶ�����
        {
            ptemp = ptemp + pos();
            move(ptemp);
            subW->move(ptemp.x() + 310,ptemp.y() + 100);
        }
        else
        {
            QRect wid = geometry();
            switch (_curpos)//�ı䴰�ڵĴ�С
            {
            case 11:wid.setTopLeft(wid.topLeft() + ptemp); break;//���Ͻ�
            case 13:wid.setTopRight(wid.topRight() + ptemp); break;//���Ͻ�
            case 31:wid.setBottomLeft(wid.bottomLeft() + ptemp); break;//���½�
            case 33:wid.setBottomRight(wid.bottomRight() + ptemp); break;//���½�
            case 12:wid.setTop(wid.top() + ptemp.y()); break;//���Ͻ�
            case 21:wid.setLeft(wid.left() + ptemp.x()); break;//�����
            case 23:wid.setRight(wid.right() + ptemp.x()); break;//���ҽ�
            case 32:wid.setBottom(wid.bottom() + ptemp.y()); break;//���½�
            }
            setGeometry(wid);
        }
        _plast = event->globalPos();//����λ��
    }
}

//��ȡ����ڴ������������ ��  ������������
int earth_map_demo::countFlag(QPoint p, int row)//�����������һ�к���һ��
{
    if (p.y() < MARGIN)
        return 10 + row;
    else if (p.y() > this->height() - MARGIN)
        return 30 + row;
    else
        return 20 + row;
}

//��ȡ����ڴ������������ ��   ��������
int earth_map_demo::countRow(QPoint p)
{
    return (p.x() < MARGIN) ? 1 : (p.x() > (this->width() - MARGIN) ? 3 : 2);
}

//�����������λ�øı����ָ����״
void earth_map_demo::setCursorType(int flag)
{
    switch (flag)
    {
    case 11:
    case 33:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case 13:
    case 31:
        setCursor(Qt::SizeBDiagCursor); break;
    case 21:
    case 23:
        setCursor(Qt::SizeHorCursor); break;
    case 12:
    case 32:
        setCursor(Qt::SizeVerCursor); break;
    case 22:
        setCursor(Qt::ArrowCursor);
        QApplication::restoreOverrideCursor();//�ָ����ָ����״
        break;
    }
}
