/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Adapted from the Qt examples by Eric MAEKER, <eric.maeker@gmail.com>  *
 ***************************************************************************/
/***************************************************************************
 *   Main Developer : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "imageviewer.h"

#include <utils/global.h>
#include <translationutils/constants.h>
#include <translationutils/trans_menu.h>

#include <QLabel>
#include <QScrollBar>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

using namespace Utils;
using namespace Trans::ConstantTranslations;

/** \todo add gesture for the resize mode and Alt(or Ctrl)+mousewheel to zoom in/out */

ImageViewer::ImageViewer(QWidget *parent) :
    QDialog(parent), m_CurrentIndex(-1)
{
    setObjectName("ImageViewer");
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(false);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_ButBox = new QDialogButtonBox(this);
    //    QAbstractButton *zin = m_ButBox->addButton(tkTr(Trans::Constants::ZOOMIN_TEXT), QDialogButtonBox::ActionRole);
    //    QAbstractButton *zout = m_ButBox->addButton(tkTr(Trans::Constants::ZOOMOUT_TEXT), QDialogButtonBox::ActionRole);

    QAbstractButton *previous = m_ButBox->addButton(tkTr(Trans::Constants::PREVIOUS_TEXT), QDialogButtonBox::ActionRole);
    QAbstractButton *next = m_ButBox->addButton(tkTr(Trans::Constants::NEXT_TEXT), QDialogButtonBox::ActionRole);
//    QAbstractButton *full = m_ButBox->addButton(tkTr(Trans::Constants::FULLSCREEN_TEXT), QDialogButtonBox::ActionRole);
    QAbstractButton *close = m_ButBox->addButton(QDialogButtonBox::Close);

    QVBoxLayout *l = new QVBoxLayout(this);
    setLayout(l);
    l->addWidget(scrollArea);
    l->addWidget(m_ButBox);

    //    connect(zin, SIGNAL(clicked()), this, SLOT(zoomIn()));
    //    connect(zout, SIGNAL(clicked()), this, SLOT(zoomOut()));

    connect(next, SIGNAL(clicked()), this, SLOT(next()));
    connect(previous, SIGNAL(clicked()), this, SLOT(previous()));
    connect(close, SIGNAL(clicked()), this, SLOT(accept()));
//    connect(full, SIGNAL(clicked()), this, SLOT(toggleFullScreen()));

    Utils::resizeAndCenter(this);
}

void ImageViewer::setPixmap(const QPixmap &pixmap)
{
    setPixmaps(QList<QPixmap>() << pixmap);
}

void ImageViewer::setPixmaps(const QList<QPixmap> &pixmaps)
{
    if (pixmaps.count()==0)
        return;
    m_pixmaps = pixmaps;
    imageLabel->setPixmap(pixmaps.at(0));
    normalSize();
    fitToWindow();
    m_CurrentIndex = 0;
    Utils::resizeAndCenter(this);
}

void ImageViewer::showPixmapFile(const QString &absFilePath)
{
    Q_UNUSED(absFilePath);
    /** \todo code here ImageViewer::showPixmapFile(const QString &absFilePath) */
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    //    bool fitToWindow = fitToWindowAct->isChecked();
    //    scrollArea->setWidgetResizable(true);
    //        normalSize();
    //    updateActions();
}

void ImageViewer::next()
{
    if (m_CurrentIndex < (m_pixmaps.count() - 1)) {
        ++m_CurrentIndex;
        imageLabel->setPixmap(m_pixmaps.at(m_CurrentIndex));
        normalSize();
        fitToWindow();
    }
}

void ImageViewer::previous()
{
    if (m_CurrentIndex >= 1) {
        --m_CurrentIndex;
        imageLabel->setPixmap(m_pixmaps.at(m_CurrentIndex));
        normalSize();
        fitToWindow();
    }
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    //     zoomInAct->setEnabled(scaleFactor < 3.0);
    //     zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImageViewer::toggleFullScreen()
{
    Utils::setFullScreen(this, true);
}
