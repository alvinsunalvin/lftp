/*
 * lftp - file transfer program
 *
 * Copyright (c) 1999 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Id$ */

#ifndef FILECOPY_H

#include "SMTask.h"
#include "buffer.h"
#include "FileAccess.h"

#define NO_SIZE	     (-1L)
#define NO_SIZE_YET  (-2L)
#define NO_DATE	     ((time_t)-1L)
#define NO_DATE_YET  ((time_t)-2L)
#define FILE_END     (-1L)

class FileCopyPeer : public Buffer
{
protected:
   bool want_size;
   bool want_date;
   long size;
   time_t date;

   long real_pos;
   long seek_pos;
   bool can_seek;
   bool date_set;

public:
   enum direction { GET, PUT };

protected:
   enum direction mode;

public:
   bool CanSeek() { return can_seek; }
   virtual void Seek(long offs) { seek_pos=offs; Empty(); eof=false; }
   virtual long GetRealPos() { return real_pos; }
   virtual long Buffered() { return in_buffer; }
   virtual bool IOReady() { return true; }
   virtual void Skip(int);

   virtual void WantDate() { want_date=true; date=NO_DATE_YET; }
   virtual void WantSize() { want_size=true; size=NO_SIZE_YET; }
   time_t GetDate() { return date; }
   long   GetSize() { return size; }

   void SetDate(time_t d)
      {
	 date=d;
	 if(date==NO_DATE || date==NO_DATE_YET)
	    date_set=true;
	 else
	    date_set=false;
      }
   void SetSize(long s) { size=s; }

   FileCopyPeer(direction m);
   ~FileCopyPeer();
};

class FileCopy : public SMTask
{
   FileCopyPeer *get;
   FileCopyPeer *put;

   enum state_t
      {
	 INITIAL,
	 PUT_WAIT,
	 DO_COPY,
	 CONFIRM_WAIT,
	 GET_DONE_WAIT,
	 ALL_DONE
      } state;

   int max_buf;
   bool cont;

   char *error_text;

public:
   long GetPos();
   int  GetPercentDone();
   float GetRate();
   time_t GetETA();
   bool Done() { return state==ALL_DONE; }
   bool Error() { return error_text!=0; }
   const char *ErrorText() { return error_text; }
   void SetError(const char *str);

   FileCopy(FileCopyPeer *src,FileCopyPeer *dst,bool cont);
   ~FileCopy();
   void Init();

   int Do();
};

class FileCopyPeerFA : public FileCopyPeer
{
   char *file;
   int FAmode;
   FileAccess *session;

   int Get_LL(int size);
   int Put_LL(const char *buf,int size);

public:
   FileCopyPeerFA(FileAccess *s,const char *f,int m);
   ~FileCopyPeerFA();
   int Do();
   bool Done();
   bool IOReady()    { return session->IOReady(); }
   long GetRealPos() { return session->GetRealPos(); }
   void Seek(long pos);

   long Buffered() { return in_buffer+session->Buffered(); }

   void Suspend();
   void Resume();
};

class FileCopyPeerFDStream : public FileCopyPeer
{
   FDStream *stream;

   int Get_LL(int size);
   int Put_LL(const char *buf,int size);

public:
   FileCopyPeerFDStream(FDStream *o,direction m);
   ~FileCopyPeerFDStream();
   int Do();
   bool Done();
   bool IOReady() { return stream->getfd()!=-1; }
   long GetRealPos() { return real_pos; }
   FgData *GetFgData(bool fg);
};

#endif
