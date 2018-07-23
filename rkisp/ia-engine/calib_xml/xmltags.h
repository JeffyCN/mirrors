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
 * @file xmltags.h
 *
 *****************************************************************************/
 #ifndef __XMLTAGS_H__
 #define __XMLTAGS_H__

 #include <tinyxml2.h>

using namespace tinyxml2;
/******************************************************************************
 * class XmlTag
 *****************************************************************************/
class XmlTag
{
public:
    enum TagType_e
    {
        TAG_TYPE_INVALID    = 0,
        TAG_TYPE_CHAR,
        TAG_TYPE_DOUBLE,
        TAG_TYPE_STRUCT,
        TAG_TYPE_CELL,
        TAG_TYPE_MAX
    };

    XmlTag( const XMLElement *e );
    
    int Size();
    const char* Value();
    unsigned int ValueToUInt( bool *ok );

    TagType_e Type();
    bool isType( const TagType_e type );

protected:
    const XMLElement* m_Element;
};


/******************************************************************************
 * class CellTag
 *****************************************************************************/
class XmlCellTag: public XmlTag
{
public:
    XmlCellTag( const XMLElement *e );

    int Index();
};

#endif /* __XMLTAGS_H__ */

