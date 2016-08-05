/*

  Spectrum display widget

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#include <stdint.h>
#include <QPainter>
#include <QFontDatabase>
#include "spectrumwidget.h"

SpectrumWidget::SpectrumWidget(QWidget *parent)
    : QWidget(parent), m_bkbuffer(0)
{
    m_dbmin = -65.0f;
    m_dbmax = 5.0f;
    m_fmin  = 0.0f;
    m_fmax  = 8000.0f;

    const QFont smallFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    setFont(smallFont);
}


void SpectrumWidget::paintEvent(QPaintEvent *event)
{
    (event);

    // create a new back buffer if the widget got
    // resized or if there isn't one
    if ((m_bkbuffer == 0) || (m_bkbuffer->width() != width())
            || (m_bkbuffer->height() != height()))
    {
        if (m_bkbuffer != 0)
            delete m_bkbuffer;

        m_bkbuffer = new QImage(rect().size(), QImage::Format_RGB32);
        QPainter bpainter(m_bkbuffer);

        bpainter.fillRect(rect(), Qt::black);

        // draw the horizontal divisions
        QString string;
        QFontMetrics fm(font());
        uint32_t fontHeight = fm.height();

        float db_start = floor(m_dbmin / 10.0f)*10.0f;
        float db_end   = floor(m_dbmax / 10.0f)*10.0f;

        if (db_end-db_start > 10.0f)
        {
            for(float db=db_start; db<=db_end; db+=10.0f)
            {
                int32_t ypos = db2pix(db);
                bpainter.setPen(Qt::gray);
                bpainter.drawLine(0, ypos, width()-1, ypos);
                string = QString("%1dB").arg(db,3,'d',0);
                uint32_t fontWidth  = fm.width(string)+2;
                QRect textRect(1,ypos-fontHeight/2,fontWidth,fontHeight);
                bpainter.fillRect(textRect,Qt::black);
                bpainter.setPen(Qt::white);
                bpainter.drawText(textRect,Qt::AlignCenter, string);
            }
        }

        // draw the x-axis
        const int32_t steps[] = {1,2,5,10,20,50,100,200,500,1000,2000,5000,10000,20000,50000,0};
        uint32_t idx = 0;
        uint32_t labelWidth  = fm.width("XXXXXXXX");
        int maxLabels = width()/labelWidth;
        while((m_fmax-m_fmin)/steps[idx] > maxLabels && steps[idx+1]>0)
            idx++;

        int32_t step = steps[idx];

        int32_t start = -step*(static_cast<int32_t>(-m_fmin/step+1000)-1000);
        QRect textRect;
        for(int32_t i=start; i<=m_fmax; i+=step)
        {
            int32_t x = static_cast<int32_t>(0.5f+(i-m_fmin)/(m_fmax-m_fmin)*width());
            bpainter.setPen(Qt::gray);
            bpainter.drawLine(x, 0, x, height()-1);

            bpainter.setPen(Qt::white);
            if (step>=1000)
            {
                string = QString("%1 kHz").arg(i/1000.0f,3,'d',0);
            }
            else
            {
                string = QString("%1 Hz").arg((double)i,3,'d',0);
            }
            int32_t txtWidth  = fm.width(string)+2;
            if ((x-txtWidth/2.0f) < 0)
            {
                // adjust for off-screen label left side
                textRect = QRect(0,height()-fontHeight,txtWidth,fontHeight);
            }
            else if ((x+txtWidth/2.0f) > width())
            {
                // adjust for off-screen label right side
                textRect = QRect(width()-txtWidth,height()-fontHeight,txtWidth,fontHeight);
            }
            else
            {
                textRect = QRect(x-txtWidth/2,height()-fontHeight,txtWidth,fontHeight);
            }
            bpainter.fillRect(textRect, Qt::black);
            bpainter.drawText(textRect,Qt::AlignCenter, string);
        }
    }
    QPainter painter(this);
    painter.drawImage(rect(), *m_bkbuffer);
}

int32_t SpectrumWidget::db2pix(float db)
{
   return static_cast<int32_t>((m_dbmax - db) / (m_dbmax-m_dbmin) * height());
}