/*
 * Copyright 2004 Martin Fuchs
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


 //
 // Explorer and Desktop clone
 //
 // favorites.cpp
 //
 // Martin Fuchs, 04.04.2004
 //


#include "../utility/utility.h"

#include "../explorer.h"
#include "../globals.h"

#include "startmenu.h"


 /// read .URL file
bool Bookmark::read_url(LPCTSTR path)
{
	char line[BUFFER_LEN];

	tifstream in(path);

	while(in.good()) {
		in.getline(line, BUFFER_LEN);

		const char* p = line;
		while(isspace(*p))
			++p;

		const char* keyword = p;
		const char* eq = strchr(p, '=');

		if (eq) {
			const char* cont = eq + 1;
			while(isspace(*cont))
				++cont;

			if (!strnicmp(keyword, "URL", 3))
				_url = cont;
			else if (!strnicmp(keyword, "IconFile", 8))
				_icon_path = cont;
		}
	}

	return true;
}

 /// convert XBEL bookmark node
bool Bookmark::read_xbel(const_XMLPos& pos)
{
	_url = pos.get("href");

	if (!pos.go_down("title"))
		return false;

	_name = pos->get_content();

	pos.back();

	return true;
}


BookmarkNode::BookmarkNode(const Bookmark& bm)
 :	_type(BMNT_BOOKMARK)
{
	_pbookmark = new Bookmark(bm);
}

BookmarkNode::BookmarkNode(const BookmarkFolder& bmf)
 :	_type(BMNT_FOLDER)
{
	_pfolder = new BookmarkFolder(bmf);
}

BookmarkNode::BookmarkNode(const BookmarkNode& other)
 :	_type(other._type)
{
	if (_type == BMNT_BOOKMARK)
		_pbookmark = new Bookmark(*other._pbookmark);
	else
		_pfolder = new BookmarkFolder(*other._pfolder);
}

BookmarkNode::~BookmarkNode()
{
	if (_type == BMNT_BOOKMARK)
		delete _pbookmark;
	else
		delete _pfolder;
}


 /// read bookmark list from XBEL formated XML tree
void BookmarkList::read(const_XMLPos& pos)
{
	const XMLNode::Children& children = pos->get_children();

	for(XMLNode::Children::const_iterator it=children.begin(); it!=children.end(); ++it) {
		const XMLNode& node = **it;
		const_XMLPos sub_pos(&node);

		if (node == "folder") {
			BookmarkFolder new_folder;

			if (sub_pos.go_down("title")) {
				new_folder._name = sub_pos->get_content();
				sub_pos.back();
			}

			new_folder._bookmarks.read(sub_pos);

			push_back(new_folder);
		} else if (node == "bookmark") {
			Bookmark bookmark;

			if (bookmark.read_xbel(sub_pos))
				push_back(bookmark);
		}
	}
}

 /// write bookmark list into XBEL formated XML tree
void BookmarkList::write(XMLPos& pos) const
{
	for(const_iterator it=begin(); it!=end(); ++it) {
		const BookmarkNode& node = *it;

		if (node._type == BookmarkNode::BMNT_FOLDER) {
			BookmarkFolder& folder = *node._pfolder;

			pos.create("folder");

			pos.create("title");
			pos->set_content(folder._name);
			pos.back();

			folder._bookmarks.write(pos);

			pos.back();
		} else {	// BookmarkNode::BMNT_BOOKMARK
			Bookmark& bookmark = *node._pbookmark;

			if (!bookmark._url.empty()) {
				pos.create("bookmark");

				pos["href"] = bookmark._url;

				pos.create("title");
				pos->set_content(bookmark._name);
				pos.back();

				pos.back();
			}
		}
	}
}


void BookmarkList::fill_tree(HWND hwnd, HTREEITEM parent) const
{
	TV_INSERTSTRUCT tvi;

	tvi.hParent = parent;
	tvi.hInsertAfter = TVI_LAST;

	TV_ITEM& tv = tvi.item;
	tv.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;

	for(const_iterator it=begin(); it!=end(); ++it) {
		const BookmarkNode& node = *it;

		tv.lParam = (LPARAM)&node;

		if (node._type == BookmarkNode::BMNT_FOLDER) {
			const BookmarkFolder& folder = *node._pfolder;

			tv.pszText = (LPTSTR)folder._name.c_str();
			tv.iImage = 3;
			tv.iSelectedImage = 4;
			HTREEITEM hitem = TreeView_InsertItem(hwnd, &tvi);

			folder._bookmarks.fill_tree(hwnd, hitem);
		} else {
			const Bookmark& bookmark = *node._pbookmark;

			tv.pszText = (LPTSTR)bookmark._name.c_str();
			tv.iImage = 0;
			tv.iSelectedImage = 1;
			TreeView_InsertItem(hwnd, &tvi);
		}
	}
}


void BookmarkList::import_IE_favorites(ShellDirectory& dir, HWND hwnd)
{
	TCHAR path[MAX_PATH], ext[_MAX_EXT];

	dir.smart_scan(SCAN_FILESYSTEM);

	for(Entry*entry=dir._down; entry; entry=entry->_next) {
		if (entry->_shell_attribs & SFGAO_HIDDEN)	// hide files like "desktop.ini"
			continue;

		String name;

		if (entry->_etype == ET_SHELL)
			name = dir._folder.get_name(static_cast<ShellEntry*>(entry)->_pidl);
		else
			name = entry->_display_name;

		if (entry->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			BookmarkFolder new_folder;

			new_folder._name = name;

			if (entry->_etype == ET_SHELL) {
				ShellDirectory new_dir(dir._folder, static_cast<ShellEntry*>(entry)->_pidl, hwnd);
				new_folder._bookmarks.import_IE_favorites(new_dir, hwnd);
			} else {
				entry->get_path(path);
				ShellDirectory new_dir(GetDesktopFolder(), path, hwnd);
				new_folder._bookmarks.import_IE_favorites(new_dir, hwnd);
			}

			push_back(new_folder);
		} else {
			Bookmark bookmark;

			bookmark._name = name;

			entry->get_path(path);
			_tsplitpath(path, NULL, NULL, NULL, ext);

			if (!_tcsicmp(ext, TEXT(".url"))) {
				bookmark.read_url(path);
				push_back(bookmark);
			} else {
				///@todo read shell links
				assert(0);
			}
		}
	}
}


 /// read XBEL bookmark file
bool Favorites::read(LPCTSTR path)
{
	XMLDoc xbel;

	if (!xbel.read(path))
		return false;

	const_XMLPos pos(&xbel);

	if (!pos.go_down("xbel"))
		return false;

	super::read(pos);

	pos.back();

	return true;
}

 /// write XBEL bookmark file
void Favorites::write(LPCTSTR path) const
{
	XMLDoc xbel;

	XMLPos pos(&xbel);
	pos.create("xbel");
	super::write(pos);
	pos.back();

	xbel.write(path, XMLNode::FORMAT_SMART, XMLHeader("1.0", "UTF-8", "<!DOCTYPE xbel"
		" PUBLIC \"+//IDN python.org//DTD XML Bookmark Exchange Language 1.0//EN//XML\"\n"
		" \"http://www.python.org/topics/xml/dtds/xbel-1.0.dtd\">"));
}

 /// import Internet Explorer bookmarks from Favorites folder
bool Favorites::import_IE_favorites(HWND hwnd)
{
	WaitCursor wait;

	StartMenuShellDirs dirs;

	try {
		dirs.push_back(ShellDirectory(GetDesktopFolder(), SpecialFolderPath(CSIDL_COMMON_FAVORITES, hwnd), hwnd));
		dirs.push_back(ShellDirectory(GetDesktopFolder(), SpecialFolderPath(CSIDL_FAVORITES, hwnd), hwnd));
	} catch(COMException&) {
	}

	for(StartMenuShellDirs::iterator it=dirs.begin(); it!=dirs.end(); ++it) {
		StartMenuDirectory& smd = *it;
		ShellDirectory& dir = smd._dir;

		try {
			super::import_IE_favorites(dir, hwnd);
		} catch(COMException&) {
		}
	}

	return true;
}
