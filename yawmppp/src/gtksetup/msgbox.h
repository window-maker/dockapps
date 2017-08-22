/* GPL'd (C) 1999-2000 Felipe Bergo -- bergo@seul.org */

/* 

   This module is built up of the following files:
   
   msgbox.h
   msgbox.cc _OR_ msgbox.c
   question.xpm
   bong.xpm
   exclamation.xpm
   info.xpm

*/

/* return values */

#ifndef FB_MSGBOX_H
#define FB_MSGBOX_H

#include <gtk/gtk.h>

/* ENUMS */

typedef enum
{
  MSGBOX_ICON_NONE,
  MSGBOX_ICON_ERROR,
  MSGBOX_ICON_INFO,
  MSGBOX_ICON_EXCLAMATION,
  MSGBOX_ICON_QUESTION
} MsgBoxIcon;

typedef enum
{
  MSGBOX_OK,
  MSGBOX_OKCANCEL,
  MSGBOX_YESNO,
  MSGBOX_YESNOCANCEL
} MsgBoxType;

typedef enum
{
  MSGBOX_R_OK,
  MSGBOX_R_YES,
  MSGBOX_R_NO,
  MSGBOX_R_CANCEL
} MsgBoxResult;

/* prototype */

/* 
   obs 1: this call blocks until the user picks a choice.
   obs 2: parent can be NULL.
   obs 3: you must have a gtk_main loop running before calling it.
*/

MsgBoxResult message_box(GtkWindow *parent,
			 char *txt,
			 char *title,
			 MsgBoxType mbt,
			 MsgBoxIcon mbi);

/* 
   excerpt from my 900-page book

   "The Ultimate Guide to Message Box Icons
    Felipe Bergo - 0th edition
    Prentice Room publishers,    
    ISBN 6666-6666-6666"

   MSGBOX_ICON_NONE          No icon.

   MSGBOX_ICON_ERROR         Should be used on errors, i.e.,
                             something wrong really happened.
                             Currently included icon is a white
                             cross in a red circle. "bong.xpm"

   MSGBOX_ICON_QUESTION      Should be used on any questions, like
                             confirmations. Currently included
                             icon is a white question mark in a 
                             green circle. "question.xpm"

   MSGBOX_ICON_EXCLAMATION   Should be used on warnings, like when
                             your code tries to allocate a resource
                             and it is busy (and being busy is "normal").
                             Currently included icon is a black exclamation
                             mark in an yellow triangle.

   MSGBOX_ICON_INFO          Should be used on general informative
                             messages, like "Operation Finished",
                             "Scan finished: 999999 blocks,
                              999998 bad, 1 good.", and the like.
                             Currently included icon is a white "i"
                             in a blue square.

   The return value is an enum of MsgBoxResult type.
*/

#endif
