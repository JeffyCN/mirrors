/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * @file    calibtreewidget.cpp
 *
 *****************************************************************************/
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>

#include "calib_xml/calibtreewidget.h"

#include "calibtags.h"
#include "xmltags.h"


/******************************************************************************
 * local macro definitions
 *****************************************************************************/
#define COLUMN_ID               0
#define COLUMN_ID_TEXT          "ID"

#define COLUMN_VALUE            1
#define COLUMN_VALUE_TEXT       "Value"

#define NOT_ASSIGNED            "N/A"


/******************************************************************************
 * class CalibTreeWidget
 *****************************************************************************/


/******************************************************************************
 * CalibTreeWidget::CalibTreeWidget
 *****************************************************************************/
CalibTreeWidget::CalibTreeWidget
(
    QWidget *parent
)
    : QTreeWidget( parent ),
        m_creationdate( NOT_ASSIGNED ),
        m_creator( NOT_ASSIGNED ),
        m_sensorname( NOT_ASSIGNED ),
        m_samplename( NOT_ASSIGNED ),
        m_generatorversion( NOT_ASSIGNED )
{
    setColumnCount( 2 );

    QStringList labels;
    labels << tr( COLUMN_ID_TEXT ) << tr( COLUMN_VALUE_TEXT );
    setHeaderLabels( labels );

    fileIcon.addPixmap( style()->standardPixmap( QStyle::SP_DirClosedIcon ), QIcon::Normal, QIcon::Off );
    headerIcon.addPixmap( style()->standardPixmap( QStyle::SP_DirClosedIcon ), QIcon::Normal, QIcon::Off );
    sensorIcon.addPixmap( style()->standardPixmap( QStyle::SP_DirClosedIcon ), QIcon::Normal, QIcon::Off );

    setSortingEnabled(false);
}


/******************************************************************************
 * CalibTreeWidget::CalibTreeWidget
 *****************************************************************************/
CalibTreeWidget::~CalibTreeWidget()
{
}



/******************************************************************************
 * CalibTreeWidget::getCreationDate
 *****************************************************************************/
QString CalibTreeWidget::getCreationDate( void )
{
    return ( m_creationdate );
}



/******************************************************************************
 * CalibTreeWidget::getCreator
 *****************************************************************************/
QString CalibTreeWidget::getCreator( void )
{
    return ( m_creator );
}



/******************************************************************************
 * CalibTreeWidget::getCreator
 *****************************************************************************/
QString CalibTreeWidget::getSensorName( void )
{
    return ( m_sensorname );
}



/******************************************************************************
 * CalibTreeWidget::getCreator
 *****************************************************************************/
QString CalibTreeWidget::getSampleName( void )
{
    return ( m_samplename );
}



/******************************************************************************
 * CalibTreeWidget::getCreator
 *****************************************************************************/
QString CalibTreeWidget::getGeneratorVersion( void )
{
    return ( m_generatorversion );
}



/******************************************************************************
 * CalibTreeWidget::parseEntryCell
 *****************************************************************************/
bool CalibTreeWidget::parseEntryCell
(
    const QDomElement   &element,
    QTreeWidgetItem     *parent,
    int                 noElements,
    parseCellContent    func
)
{
    int cnt = 0;

    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item;

    QDomNode child = element.firstChild();
    while ( !child.isNull() && (cnt < noElements) )
    {
        XmlCellTag tag = XmlCellTag( child.toElement() );
        if ( child.toElement().tagName() == CALIB_CELL_TAG )
        {
            bool result = (this->*func)( child.toElement(), parent );
            if ( !result )
            {
                return ( result );
            }
        }
        else
        {
            qDebug()
                << "unknown cell tag: "
                << child.toElement().tagName();

            return ( false );
        }

        child = child.nextSibling();
        cnt ++;
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::readFile
 *****************************************************************************/
bool CalibTreeWidget::readFile
(
    QFile   *device
)
{
    QString errorString;
    int errorLine;
    int errorColumn;

    bool res = true;

    qDebug() << __func__ << " (enter)";

    if ( !doc.setContent( device, false, &errorString, &errorLine, &errorColumn) )
    {
        qDebug()
            << "Error: Parse error at line " << errorLine << ", "
            << "column " << errorColumn << ": "
            << qPrintable( errorString );

        return ( false);
    }

    root = doc.documentElement();
    if ( root.tagName() != CALIB_FILESTART_TAG )
    {
        qDebug()
            << "Error: Not a calibration data file";

        return ( false );
    }

    // 'disable' editing
    disconnect( this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(updateDomElement(QTreeWidgetItem *, int)) );
    disconnect( this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doubleClickedDomElement(QTreeWidgetItem *, int)) );
    disconnect( this, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(clickedDomElement(QTreeWidgetItem *, int)) );

    // make sure we're empty
    clear();

    // create main item
    QTreeWidgetItem *item = createTreeItem( root, NULL );
    item->setIcon( COLUMN_ID    , fileIcon );
    item->setText( COLUMN_ID    , "file" );
    item->setText( COLUMN_VALUE , device->fileName() );

    // parse header section
    QDomElement header = root.firstChildElement( CALIB_HEADER_TAG );
    if ( !header.isNull() )
    {
        res = populateEntryHeader( header.toElement(), item );
        if ( !res )
        {
            return ( res );
        }
    }

    // parse sensor section
    QDomElement sensor = root.firstChildElement( CALIB_SENSOR_TAG );
    if ( !sensor.isNull() )
    {
        res = populateEntrySensor( sensor.toElement(), item );
        if ( !res )
        {
            return ( res );
        }
    }

    // get and parse system section
    QDomElement system = root.firstChildElement( CALIB_SYSTEM_TAG );
    if ( !system.isNull() )
    {
        res = populateEntrySystem( system.toElement(), NULL );
        if ( !res )
        {
            return ( res );
        }
    }

    // resize columns according to content
    expandToDepth(1);
    resizeColumnToContents(COLUMN_ID);
    //resizeColumnToContents(COLUMN_VALUE);

    // 'enable' editing
    connect( this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(updateDomElement(QTreeWidgetItem *, int)) );
    connect( this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doubleClickedDomElement(QTreeWidgetItem *, int)) );
    connect( this, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(clickedDomElement(QTreeWidgetItem *, int)) );

    qDebug() << __func__ << " (exit)";

    return ( res );
}



/******************************************************************************
 * CalibTreeWidget::updateDomElement
 *****************************************************************************/
void CalibTreeWidget::updateDomElement
(
    QTreeWidgetItem *item,
    int             column
)
{
    if ( column == COLUMN_VALUE )
    {
        QDomElement element = domElementForItem.value( item );
        if ( !element.isNull() )
        {
        }
    }
}



/******************************************************************************
 * CalibTreeWidget::doubleClickedDomElement
 *****************************************************************************/
void CalibTreeWidget::doubleClickedDomElement
(
    QTreeWidgetItem *item,
    int             column
)
{
    if ( column == COLUMN_VALUE )
    {
        QDomElement element = domElementForItem.value( item );
        // item->setFlags( item->flags() | Qt::ItemIsEditable );
    }
    else
    {
        // item->setFlags( item->flags() &  ~Qt::ItemIsEditable );
    }
}



/******************************************************************************
 * CalibTreeWidget::clickedDomElement
 *****************************************************************************/
void CalibTreeWidget::clickedDomElement
(
    QTreeWidgetItem *item,
    int             column
)
{
    QDomElement element = domElementForItem.value( item );
    XmlTag tag = XmlTag( element );

    qDebug()
        << "item: "
        << element.tagName()
        << "value: "
        << tag.Value();
}



/******************************************************************************
 * CalibTreeWidget::createTreeItem
 *****************************************************************************/
QTreeWidgetItem *CalibTreeWidget::createTreeItem
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    QTreeWidgetItem *item;
    if ( parentItem )
    {
        item = new QTreeWidgetItem( parentItem );
    }
    else
    {
        item = new QTreeWidgetItem( this );
    }

    domElementForItem.insert( item, element );

    return ( item );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryHeader
 *****************************************************************************/
bool CalibTreeWidget::populateEntryHeader
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setIcon( COLUMN_ID, headerIcon );
    item->setText( COLUMN_ID, "HEADER" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( child.toElement().tagName() == CALIB_HEADER_CREATION_DATE_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );

            m_creationdate = tag.Value();
        }
        else if ( child.toElement().tagName() ==  CALIB_HEADER_CREATOR_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );

            m_creator = tag.Value();
        }
        else if ( child.toElement().tagName() ==  CALIB_HEADER_SENSOR_NAME_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );

            m_sensorname = tag.Value();
        }
        else if ( child.toElement().tagName() ==  CALIB_HEADER_SAMPLE_NAME_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );

            m_samplename = tag.Value();
        }
        else if ( child.toElement().tagName() ==  CALIB_HEADER_GENERATOR_VERSION_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );

            m_generatorversion = tag.Value();
        }
        else if ( child.toElement().tagName() == CALIB_HEADER_RESOLUTION_TAG )
        {
            if ( !parseEntryCell( child.toElement(), item, tag.Size(), &CalibTreeWidget::populateEntryResolution ) )
            {
                qDebug()
                    << "parse error in resolution section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in header section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryResolution
 *****************************************************************************/
bool CalibTreeWidget::populateEntryResolution
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, CALIB_HEADER_RESOLUTION_TAG );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_NAME_TAG)
                || (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_WIDTH_TAG)
                || (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_HEIGHT_TAG)
                || (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_ID_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );
        }
        else if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_FRATE_TAG)
                    && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
                    && (tag.Size() > 0) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );
        }
        else if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_FRATE_TAG)
                    && (tag.isType( XmlTag::TAG_TYPE_CELL ))
                    && (tag.Size() > 0) )
        {
            // TODO: parse and show aFPS framerates
        }
        else
        {
            qDebug()
                << "parse error in resolution section (unknow tag: "
                << child.toElement().tagName();

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntrySensor
 *****************************************************************************/
bool CalibTreeWidget::populateEntrySensor
(
    const QDomElement   &element,
    QTreeWidgetItem     *parent
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parent );
    item->setIcon( 0, sensorIcon );
    item->setText( COLUMN_ID, "SENSOR" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        if ( child.toElement().tagName() == CALIB_SENSOR_AWB_TAG )
        {
            if ( !populateEntryAwb( child.toElement(), item ) )
            {
                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_LSC_TAG )
        {
            XmlTag tag = XmlTag( child.toElement() );

            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_LSC_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryLsc ) )
            {
                qDebug()
                    << "parse error in LSC section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_CC_TAG )
        {
            XmlTag tag = XmlTag( child.toElement() );

            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_CC_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryCc ) )
            {
                qDebug()
                    << "parse error in CC section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AF_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_AF_TAG );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AEC_TAG )
        {
            if ( !populateEntryAec( child.toElement(), item ) )
            {
                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_BLS_TAG )
        {
            XmlTag tag = XmlTag( child.toElement() );

            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_BLS_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryBls ) )
            {
                qDebug()
                    << "parse error in BLS section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_DEGAMMA_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_DEGAMMA_TAG );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_WDR_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_WDR_TAG );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_CAC_TAG )
        {
            XmlTag tag = XmlTag( child.toElement() );

            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_CAC_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryCac ) )
            {
                qDebug()
                    << "parse error in CAC section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_TAG )
        {
            XmlTag tag = XmlTag( child.toElement() );

            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_DPF_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryDpf ) )
            {
                qDebug()
                    << "parse error in DPF section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_DPCC_TAG )
        {
            XmlTag tag = XmlTag( child.toElement() );

            QTreeWidgetItem *subitem = createTreeItem( element, item );
            subitem->setText( 0, CALIB_SENSOR_DPCC_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryDpcc ) )
            {
                qDebug()
                    << "parse error in DPCC section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in header section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}


/******************************************************************************
 * CalibTreeWidget::populateEntryAec
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAec
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, CALIB_SENSOR_AEC_TAG );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_SETPOINT_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_OVER_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_UNDER_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_TAG )
        {
            if ( !parseEntryCell( child.toElement(), item, tag.Size(), &CalibTreeWidget::populateEntryAecEcm ) )
            {
                qDebug()
                    << "parse error in AEC section ("
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in AEC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            //return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAecEcm
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAecEcm
(
    const QDomElement&  element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "EC-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_NAME_TAG )
                && (tag.isType( XmlTag::TAG_TYPE_CHAR ))
                && (tag.Size() > 0) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEMES_TAG )
        {
            if ( !parseEntryCell( child.toElement(), item, tag.Size(), &CalibTreeWidget::populateEntryAecEcmPriorityScheme ) )
            {
                qDebug()
                    << "parse error in ECM section ("
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in ECM section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAecEcmPriorityScheme
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAecEcmPriorityScheme
(
    const QDomElement&  element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "EC-SCHEME" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG)
          || (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in ECM section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            //return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAwb
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAwb
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, CALIB_SENSOR_AWB_TAG );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_TAG )
        {
            if ( !parseEntryCell( child.toElement(), item, tag.Size(), &CalibTreeWidget::populateEntryAwbGlobals ) )
            {
                qDebug()
                    << "parse error in AWB globals ("
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_TAG )
        {
            if ( !parseEntryCell( child.toElement(), item, tag.Size(), &CalibTreeWidget::populateEntryAwbIllumination ) )
            {
                qDebug()
                    << "parse error in AWB illumination ("
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in AWB section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAwbGlobals
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAwbGlobals
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "GLOBALS" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RESOLUTION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_SENSOR_FILENAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_SVDMEANVALUE_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_PCAMATRIX_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CENTERLINE_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_KFACTOR_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RG1_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAXDIST1_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RG2_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAXDIST2_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE1_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST1_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE2_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST2_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_FADE2_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMAX_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMAX_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMAX_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMIN_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMIN_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMIN_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_INDOOR_MIN_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_SKY_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CLIP_OUTDOOR)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_INC)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_DEC)
                )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID    , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE , tag.Value() );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID, CALIB_SENSOR_AWB_GLOBALS_IIR );

            QDomNode subchild = child.toElement().firstChild();
            while ( !subchild.isNull() )
            {
                XmlTag tag = XmlTag( subchild.toElement() );

                if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_ADD)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_SUB)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_FILTER_THRESHOLD)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MIN)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MAX)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_INIT)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_MIDDLE)
                        )
                {
                    QTreeWidgetItem *subsubitem = createTreeItem( child.toElement(), subitem );
                    subsubitem->setText( COLUMN_ID     , subchild.toElement().tagName() );
                    subsubitem->setText( COLUMN_VALUE  , tag.Value() );
                }
                else
                {
                    qDebug()
                        << "parse error in AWB GLOBALS - IIR section (unknow tag: "
                        << subchild.toElement().tagName()
                        << ")";

                    return ( false );
                }

                subchild = subchild.nextSibling();
            }
        }
        else
        {
            qDebug()
                << "parse error in AWB GLOBALS section (unknow tag: "
                << child.toElement().tagName()
                << ")";
            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAwbIllumination
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAwbIllumination
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, CALIB_SENSOR_AWB_ILLUMINATION_TAG );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_WB_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CC_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CTO_TAG)
                )
        {
            QString value = tag.Value().toUpper();

            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID     , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE  , value );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID, CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG );

            QDomNode subchild = child.toElement().firstChild();
            while ( !subchild.isNull() )
            {
                XmlTag tag = XmlTag( subchild.toElement() );

                if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_INV_COV_MATRIX_TAG)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAU_TAG) )
                {
                    QTreeWidgetItem *subsubitem = createTreeItem( child.toElement(), subitem );
                    subsubitem->setText( COLUMN_ID     , subchild.toElement().tagName() );
                    subsubitem->setText( COLUMN_VALUE  , tag.Value() );
                }
                else
                {
                    qDebug()
                        << "parse error in AWB illumination - GMM section (unknow tag: "
                        << subchild.toElement().tagName()
                        << ")";

                    return ( false );
                }

                subchild = subchild.nextSibling();
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID     , CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG );

            QDomNode subchild = child.toElement().firstChild();
            while ( !subchild.isNull() )
            {
                XmlTag tag = XmlTag( subchild.toElement() );

                if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_GAIN_TAG)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_SAT_TAG) )
                {
                    QTreeWidgetItem *subsubitem = createTreeItem( child.toElement(), subitem );
                    subsubitem->setText( COLUMN_ID   , subchild.toElement().tagName() );
                    subsubitem->setText( COLUMN_VALUE, tag.Value() );
                }
                else
                {
                    qDebug()
                        << "parse error in AWB illumination - saturation curve section (unknow tag: "
                        << subchild.toElement().tagName()
                        << ")";
                    return ( false );
                }

                subchild = subchild.nextSibling();
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID, CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG );

            QDomNode subchild = child.toElement().firstChild();
            while ( !subchild.isNull() )
            {
                XmlTag tag = XmlTag( subchild.toElement() );

                if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_GAIN_TAG)
                        || (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_VIG_TAG) )
                {
                    QTreeWidgetItem *subsubitem = createTreeItem( child.toElement(), subitem );
                    subsubitem->setText( COLUMN_ID   , subchild.toElement().tagName() );
                    subsubitem->setText( COLUMN_VALUE, tag.Value() );
                }
                else
                {
                    qDebug()
                        << "parse error in AWB illumination - vignetting curve section (unknow tag: "
                        << subchild.toElement().tagName()
                        << ")";

                    return ( false );
                }

                subchild = subchild.nextSibling();
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID, CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG );
            if ( !parseEntryCell( child.toElement(), subitem, tag.Size(), &CalibTreeWidget::populateEntryAwbIlluminationAlsc ) )
            {
                qDebug()
                    << "parse error in AWB illumination - aLSC section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID, CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG );
            if ( !populateEntryAwbIlluminationAcc( child.toElement(), subitem ) )
            {
                qDebug()
                    << "parse error in AWB illumination - aCC section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in AWB illumination section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAwbIlluminationAlsc
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAwbIlluminationAlsc
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "resolution" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in AWB illumination - aLSC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryAwbIlluminationAcc
 *****************************************************************************/
bool CalibTreeWidget::populateEntryAwbIlluminationAcc
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );
        if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG )
        {
            QTreeWidgetItem *item = createTreeItem( child.toElement(), parentItem );
            item->setText( COLUMN_ID   , child.toElement().tagName() );
            item->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in AWB illumination - aCC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryLsc
 *****************************************************************************/
bool CalibTreeWidget::populateEntryLsc
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "LSC-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );
        if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG) )
        {
            QString value = tag.Value().toUpper();

            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in LSC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryCc
 *****************************************************************************/
bool CalibTreeWidget::populateEntryCc
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "CC-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );
        if ( (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_SATURATION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_WB_TAG ) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in CC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryBls
 *****************************************************************************/
bool CalibTreeWidget::populateEntryBls
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "BLS-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_BLS_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_BLS_RESOLUTION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_BLS_DATA_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in BLS section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryCac
 *****************************************************************************/
bool CalibTreeWidget::populateEntryCac
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "CAC-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_CAC_NAME_TAG )
                || (child.toElement().tagName() == CALIB_SENSOR_CAC_RESOLUTION_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_X_NORMSHIFT_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_X_NORMFACTOR_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_X_OFFSET_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_Y_OFFSET_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_RED_PARAMETERS_TAG)
                || (child.toElement().tagName() == CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in CAC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryDpf
 *****************************************************************************/
bool CalibTreeWidget::populateEntryDpf
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "DPF-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_DPF_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_RESOLUTION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_NLL_COEFF_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_SIGMA_GREEN_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_GRADIENT_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_OFFSET_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPF_NLGAINS_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in DPF section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryDpcc
 *****************************************************************************/
bool CalibTreeWidget::populateEntryDpcc
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "DPCC-PROFILE" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPCC_RESOLUTION_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else if ( child.toElement().tagName() == CALIB_SENSOR_DPCC_REGISTER_TAG )
        {
            if ( !parseEntryCell( child.toElement(), item, tag.Size(), &CalibTreeWidget::populateEntryDpccRegisters ) )
            {
                qDebug()
                    << "parse error in DPCC register section (unknow tag: "
                    << child.toElement().tagName()
                    << ")";

                return ( false );
            }
        }
        else
        {
            qDebug()
                << "parse error in DPCC section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



/******************************************************************************
 * CalibTreeWidget::populateEntryDpccRegisters
 *****************************************************************************/
bool CalibTreeWidget::populateEntryDpccRegisters
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QTreeWidgetItem *item = createTreeItem( element, parentItem );
    item->setText( COLUMN_ID, "REGISTER" );

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );

        if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_REGISTER_NAME_TAG)
                || (child.toElement().tagName() == CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG) )
        {
            QTreeWidgetItem *subitem = createTreeItem( child.toElement(), item );
            subitem->setText( COLUMN_ID   , child.toElement().tagName() );
            subitem->setText( COLUMN_VALUE, tag.Value() );
        }
        else
        {
            qDebug()
                << "parse error in DPCC register section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }


        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}


/******************************************************************************
 * CalibTreeWidget::populateEntrySystem
 *****************************************************************************/
bool CalibTreeWidget::populateEntrySystem
(
    const QDomElement   &element,
    QTreeWidgetItem     *parentItem
)
{
    qDebug() << __func__ << " (enter)";

    QDomNode child = element.firstChild();
    while ( !child.isNull() )
    {
        XmlTag tag = XmlTag( child.toElement() );
        QString value = tag.Value();

        if (child.toElement().tagName() == CALIB_SYSTEM_AFPS_TAG)
        {
            // do nothing, we don't want to see system settings in tree 
        }
        else
        {
            qDebug()
                << "parse error in system section (unknow tag: "
                << child.toElement().tagName()
                << ")";

            return ( false );
        }

        child = child.nextSibling();
    }

    qDebug() << __func__ << " (exit)";

    return ( true );
}



