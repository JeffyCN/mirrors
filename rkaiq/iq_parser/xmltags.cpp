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
 * @file        xmltags.cpp
 *
 *****************************************************************************/

#include "calibtags.h"
#include "xmltags.h"
#include <string>
#include <iostream>

/******************************************************************************
 * class XmlTag
 *****************************************************************************/

/******************************************************************************
 * XmlTag::XmlTag
 *****************************************************************************/
XmlTag::XmlTag( const XMLElement* e )
    : m_Element( e )
{
}



/******************************************************************************
 * XmlTag::XmlTag
 *****************************************************************************/
int XmlTag::Size()
{
    const XMLAttribute* pattr = m_Element->FindAttribute(CALIB_ATTRIBUTE_SIZE);
    const char* c_string = pattr->Value();

    int col;
    int row;

    int res = sscanf(c_string, CALIB_ATTRIBUTE_SIZE_FORMAT, &row, &col);
    if (res != CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS)
    {
        return (0);
    }

    return ((col * row));
}

int XmlTag::SizeRow()
{
    const XMLAttribute* pattr = m_Element->FindAttribute(CALIB_ATTRIBUTE_SIZE);
    const char* c_string = pattr->Value();

    int col;
    int row;

    int res = sscanf(c_string, CALIB_ATTRIBUTE_SIZE_FORMAT, &row, &col);
    if (res != CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS)
    {
        return (0);
    }

    return (row);
}

int XmlTag::SizeCol()
{
    const XMLAttribute* pattr = m_Element->FindAttribute(CALIB_ATTRIBUTE_SIZE);
    const char* c_string = pattr->Value();

    int col;
    int row;

    int res = sscanf(c_string, CALIB_ATTRIBUTE_SIZE_FORMAT, &row, &col);
    if (res != CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS)
    {
        return (0);
    }

    return (col);
}



/******************************************************************************
 * XmlTag::Value
 *****************************************************************************/
const char* XmlTag::Value()
{
    const char* text = m_Element->GetText();
    char* pstr_start = (char*)text;
    int len = 0;
    char* pstr_last = pstr_start;

#if 0
    if (pstr_start) {
        len = strlen(pstr_start);
        pstr_last = (char*)text + len - 1;

        while ((*pstr_start == '\n' || *pstr_start == '\r' || *pstr_start == 0x20 || *pstr_start == 0x09) && (pstr_start != pstr_last)) {
            pstr_start++;
        }

        while ((*pstr_last == '\n' || *pstr_last == '\r' || *pstr_last == 0x20 || *pstr_last == 0x09) && (pstr_start != pstr_last)) {
            pstr_last--;
        }

        len = pstr_last - pstr_start;
    }

    if (len) {
        *(pstr_last + 1) = '\0';
    }
#else
    /* above code cannot process the case: there is only one valide charactor,such as "A  \n\r" */
    if (pstr_start) {
        len = strlen(pstr_start);
        pstr_last = (char*)text + len - 1;

        while ((*pstr_start == '\n' || *pstr_start == '\r' || *pstr_start == 0x20 || *pstr_start == 0x09) && (pstr_start != pstr_last)) {
            pstr_start++;
        }

        while ((*pstr_last == '\n' || *pstr_last == '\r' || *pstr_last == 0x20 || *pstr_last == 0x09) && (pstr_start != pstr_last)) {
            *pstr_last = '\0';
            pstr_last--;
        }
    }
#endif

    return pstr_start ;
}


/******************************************************************************
 * XmlTag::ValueTonUInt
 *****************************************************************************/
unsigned int XmlTag::ValueToUInt( bool *ok )
{
    const char* value = Value();
    unsigned int v = 0;

    //*ok = XMLUtil::ToUnsigned(value, &v);
    if(sscanf(value, "%x", &v) == 1)
        *ok = true;
    else
        *ok = false;

    return ( v );
}



/******************************************************************************
 * XmlTag::Type
 *****************************************************************************/
XmlTag::TagType_e XmlTag::Type()
{
    const XMLAttribute *pattr = m_Element->FindAttribute( CALIB_ATTRIBUTE_TYPE );
    std::string s_value(pattr->Value());

    if ( s_value == CALIB_ATTRIBUTE_TYPE_CHAR )
    {
        return ( TAG_TYPE_CHAR );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_DOUBLE )
    {
        return ( TAG_TYPE_DOUBLE );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_STRUCT )
    {
        return ( TAG_TYPE_STRUCT );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_CELL )
    {
        return ( TAG_TYPE_CELL );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_INT )
    {
        return ( TAG_TYPE_INT );
    }
    else
    {
        return ( TAG_TYPE_INVALID );
    }

    return ( TAG_TYPE_INVALID );
}



/******************************************************************************
 * XmlTag::isType
 *****************************************************************************/
bool XmlTag::isType
(
    const XmlTag::TagType_e type
)
{
    const XMLAttribute* pattr = m_Element->FindAttribute( CALIB_ATTRIBUTE_TYPE );
    std::string s_value(pattr->Value());

    if ( s_value == CALIB_ATTRIBUTE_TYPE_CHAR )
    {
        return ( (bool)(TAG_TYPE_CHAR == type) );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_DOUBLE )
    {
        return ( (bool)(TAG_TYPE_DOUBLE == type) );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_STRUCT )
    {
        return ( (bool)(TAG_TYPE_STRUCT == type) );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_CELL )
    {
        return ( (bool)(TAG_TYPE_CELL == type) );
    }
    else if ( s_value == CALIB_ATTRIBUTE_TYPE_INT )
    {
        return ( (bool)(TAG_TYPE_INT == type) );
    }
    else
    {
        return ( (bool)(TAG_TYPE_INVALID == type) );
    }

    return ( false );
}



/******************************************************************************
 * class XmlCellTag
 *****************************************************************************/

/******************************************************************************
 * XmlCellTag::XmlCellTag
 *****************************************************************************/
XmlCellTag::XmlCellTag( const XMLElement *e )
    : XmlTag( e )
{
}



/******************************************************************************
 * XmlCellTag::XmlTag
 *****************************************************************************/
int XmlCellTag::Index()
{
    int value = 0;

    const XMLAttribute *pattr = m_Element->FindAttribute( CALIB_ATTRIBUTE_INDEX );
    if ( pattr )
    {
        pattr->QueryIntValue(&value);
    }

    return ( value );

#if 0
    int res = 0;
    res = m_Element.QueryIntAttribute(CALIB_ATTRIBUTE_INDEX, &value);
    if(res == XML_NO_ATTRIBUTE)
        return err;
    else
        return ( value );
#endif

}


