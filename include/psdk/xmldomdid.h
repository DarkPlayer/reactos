/*
 * Copyright (C) 2005 Mike McCormack
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __XMLDOMDID_H__
#define __XMLDOMDID_H__

#define DISPID_DOM_BASE 1
#define DISPID_DOM_COLLECTION_BASE 1000000
#define DISPID_DOM_COLLECTION_MAX 2999999

#define DISPID_DOM_NODE 1
#define DISPID_DOM_NODE_NODENAME 2
#define DISPID_DOM_NODE_NODEVALUE 3
#define DISPID_DOM_NODE_NODETYPE 4
#define DISPID_DOM_NODE_NODETYPEENUM 5
#define DISPID_DOM_NODE_PARENTNODE 6
#define DISPID_DOM_NODE_CHILDNODES 7
#define DISPID_DOM_NODE_FIRSTCHILD 8
#define DISPID_DOM_NODE_LASTCHILD 9
#define DISPID_DOM_NODE_PREVIOUSSIBLING 10
#define DISPID_DOM_NODE_NEXTSIBLING 11
#define DISPID_DOM_NODE_ATTRIBUTES 12
#define DISPID_DOM_NODE_INSERTBEFORE 13
#define DISPID_DOM_NODE_REPLACECHILD 14
#define DISPID_DOM_NODE_REMOVECHILD 15
#define DISPID_DOM_NODE_APPENDCHILD 16
#define DISPID_DOM_NODE_HASCHILDNODES 17
#define DISPID_DOM_NODE_OWNERDOC 18
#define DISPID_DOM_NODE_CLONENODE 19

#define DISPID_XMLDOM_NODE 20
#define DISPID_XMLDOM_NODE_STRINGTYPE 21
#define DISPID_XMLDOM_NODE_SPECIFIED 22
#define DISPID_XMLDOM_NODE_DEFINITION 23
#define DISPID_XMLDOM_NODE_TEXT 24
#define DISPID_XMLDOM_NODE_NODETYPEDVALUE 25
#define DISPID_XMLDOM_NODE_DATATYPE 26
#define DISPID_XMLDOM_NODE_XML 27
#define DISPID_XMLDOM_NODE_TRANSFORMNODE 28
#define DISPID_XMLDOM_NODE_SELECTNODES 29
#define DISPID_XMLDOM_NODE_SELECTSINGLENODE 30
#define DISPID_XMLDOM_NODE_PARSED 31
#define DISPID_XMLDOM_NODE_NAMESPACE 32
#define DISPID_XMLDOM_NODE_PREFIX 33
#define DISPID_XMLDOM_NODE_BASENAME 34
#define DISPID_XMLDOM_NODE_TRANSFORMNODETOOBJECT 35
#define DISPID_XMLDOM_NODE__TOP 36

#define DISPID_DOM_DOCUMENT 37
#define DISPID_DOM_DOCUMENT_DOCTYPE 38
#define DISPID_DOM_DOCUMENT_IMPLEMENTATION 39
#define DISPID_DOM_DOCUMENT_DOCUMENTELEMENT 40
#define DISPID_DOM_DOCUMENT_CREATEELEMENT 41
#define DISPID_DOM_DOCUMENT_CREATEDOCUMENTFRAGMENT 42
#define DISPID_DOM_DOCUMENT_CREATETEXTNODE 43
#define DISPID_DOM_DOCUMENT_CREATECOMMENT 44
#define DISPID_DOM_DOCUMENT_CREATECDATASECTION 45
#define DISPID_DOM_DOCUMENT_CREATEPROCESSINGINSTRUCTION 46
#define DISPID_DOM_DOCUMENT_CREATEATTRIBUTE 47
#define DISPID_DOM_DOCUMENT_CREATEENTITY 48
#define DISPID_DOM_DOCUMENT_CREATEENTITYREFERENCE 49
#define DISPID_DOM_DOCUMENT_GETELEMENTSBYTAGNAME 50
#define DISPID_DOM_DOCUMENT_TOP 51

#define DISPID_XMLDOM_DOCUMENT 52
#define DISPID_XMLDOM_DOCUMENT_DOCUMENTNODE 53
#define DISPID_XMLDOM_DOCUMENT_CREATENODE 54
#define DISPID_XMLDOM_DOCUMENT_CREATENODEEX 55
#define DISPID_XMLDOM_DOCUMENT_NODEFROMID 56
#define DISPID_XMLDOM_DOCUMENT_DOCUMENTNAMESPACES 57
#define DISPID_XMLDOM_DOCUMENT_LOAD 58
#define DISPID_XMLDOM_DOCUMENT_PARSEERROR 59
#define DISPID_XMLDOM_DOCUMENT_URL 60
#define DISPID_XMLDOM_DOCUMENT_ASYNC 61
#define DISPID_XMLDOM_DOCUMENT_ABORT 62
#define DISPID_XMLDOM_DOCUMENT_LOADXML 63
#define DISPID_XMLDOM_DOCUMENT_SAVE 64
#define DISPID_XMLDOM_DOCUMENT_VALIDATE 65
#define DISPID_XMLDOM_DOCUMENT_RESOLVENAMESPACE 66
#define DISPID_XMLDOM_DOCUMENT_PRESERVEWHITESPACE 67
#define DISPID_XMLDOM_DOCUMENT_ONREADYSTATECHANGE 68
#define DISPID_XMLDOM_DOCUMENT_ONDATAAVAILABLE 69
#define DISPID_XMLDOM_DOCUMENT_ONTRANSFORMNODE 70
#define DISPID_XMLDOM_DOCUMENT__TOP 71

#define DISPID_DOM_NODELIST 72
#define DISPID_DOM_NODELIST_ITEM 73
#define DISPID_DOM_NODELIST_LENGTH 74

#define DISPID_XMLDOM_NODELIST 75
#define DISPID_XMLDOM_NODELIST_NEXTNODE 76
#define DISPID_XMLDOM_NODELIST_RESET 77
#define DISPID_XMLDOM_NODELIST_NEWENUM 78
#define DISPID_XMLDOM_NODELIST__TOP 79

#define DISPID_DOM_NAMEDNODEMAP 80
#define DISPID_DOM_NAMEDNODEMAP_GETNAMEDITEM 81
#define DISPID_DOM_NAMEDNODEMAP_SETNAMEDITEM 82
#define DISPID_DOM_NAMEDNODEMAP_REMOVENAMEDITEM 83

#define DISPID_XMLDOM_NAMEDNODEMAP 84
#define DISPID_XMLDOM_NAMEDNODEMAP_GETQUALIFIEDITEM 85
#define DISPID_XMLDOM_NAMEDNODEMAP_REMOVEQUALIFIEDITEM 86
#define DISPID_XMLDOM_NAMEDNODEMAP_NEXTNODE 87
#define DISPID_XMLDOM_NAMEDNODEMAP_RESET 88
#define DISPID_XMLDOM_NAMEDNODEMAP_NEWENUM 89
#define DISPID_XMLDOM_NAMEDNODEMAP__TOP 90

#define DISPID_DOM_W3CWRAPPERS 91

#define DISPID_DOM_DOCUMENTFRAGMENT 92
#define DISPID_DOM_DOCUMENTFRAGMENT__TOP 93

#define DISPID_DOM_ELEMENT 94
#define DISPID_DOM_ELEMENT_GETTAGNAME 95
#define DISPID_DOM_ELEMENT_GETATTRIBUTES 96
#define DISPID_DOM_ELEMENT_GETATTRIBUTE 97
#define DISPID_DOM_ELEMENT_SETATTRIBUTE 98
#define DISPID_DOM_ELEMENT_REMOVEATTRIBUTE 99
#define DISPID_DOM_ELEMENT_GETATTRIBUTENODE 100
#define DISPID_DOM_ELEMENT_SETATTRIBUTENODE 101
#define DISPID_DOM_ELEMENT_REMOVEATTRIBUTENODE 102
#define DISPID_DOM_ELEMENT_GETELEMENTSBYTAGNAME 103
#define DISPID_DOM_ELEMENT_NORMALIZE 104
#define DISPID_DOM_ELEMENT__TOP 105

#define DISPID_DOM_DATA 106
#define DISPID_DOM_DATA_DATA 107
#define DISPID_DOM_DATA_LENGTH 108
#define DISPID_DOM_DATA_SUBSTRING 109
#define DISPID_DOM_DATA_APPEND 110
#define DISPID_DOM_DATA_INSERT 111
#define DISPID_DOM_DATA_DELETE 112
#define DISPID_DOM_DATA_REPLACE 113
#define DISPID_DOM_DATA__TOP 114

#define DISPID_DOM_ATTRIBUTE 115
#define DISPID_DOM_ATTRIBUTE_GETNAME 116
#define DISPID_DOM_ATTRIBUTE_SPECIFIED 117
#define DISPID_DOM_ATTRIBUTE_VALUE 118
#define DISPID_DOM_ATTRIBUTE__TOP 119

#define DISPID_DOM_TEXT 120
#define DISPID_DOM_TEXT_SPLITTEXT 121
#define DISPID_DOM_TEXT_JOINTEXT 122
#define DISPID_DOM_TEXT__TOP 123

#define DISPID_DOM_PI 124
#define DISPID_DOM_PI_TARGET 125
#define DISPID_DOM_PI_DATA 126
#define DISPID_DOM_PI__TOP 127

#define DISPID_DOM_DOCUMENTTYPE 128
#define DISPID_DOM_DOCUMENTTYPE_NAME 129
#define DISPID_DOM_DOCUMENTTYPE_ENTITIES 130
#define DISPID_DOM_DOCUMENTTYPE_NOTATIONS 131
#define DISPID_DOM_DOCUMENTTYPE__TOP 132

#define DISPID_DOM_NOTATION 133
#define DISPID_DOM_NOTATION_PUBLICID 134
#define DISPID_DOM_NOTATION_SYSTEMID 135
#define DISPID_DOM_NOTATION__TOP 136

#define DISPID_DOM_ENTITY 137
#define DISPID_DOM_ENTITY_PUBLICID 138
#define DISPID_DOM_ENTITY_SYSTEMID 139
#define DISPID_DOM_ENTITY_NOTATIONNAME 140
#define DISPID_DOM_ENTITY__TOP 141

#define DISPID_DOM_IMPLEMENTATION 142
#define DISPID_DOM_IMPLEMENTATION_HASFEATURE 143
#define DISPID_DOM_IMPLEMENTATION__TOP 144


#define  DISPID_DOM_ERROR              0x000000b0
#define  DISPID_DOM_ERROR_ERRORCODE    0x000000b1
#define  DISPID_DOM_ERROR_URL          0x000000b2
#define  DISPID_DOM_ERROR_REASON       0x000000b3
#define  DISPID_DOM_ERROR_SRCTEXT      0x000000b4
#define  DISPID_DOM_ERROR_LINE         0x000000b5
#define  DISPID_DOM_ERROR_LINEPOS      0x000000b6
#define  DISPID_DOM_ERROR_FILEPOS      0x000000b7
#define  DISPID_DOM_ERROR__TOP         0x000000b8


#endif /* __XMLDOMDID_H__ */
