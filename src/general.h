//
// Copyright (c) 2009 Wei Mingzhi <whistler@openoffice.org>
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see http://www.gnu.org/licenses.
//

#ifndef GENERAL_H
#define GENERAL_H

enum
{
   SOUND_MOVECARD = 0,

   SOUND_BOMB,
   SOUND_BID,
   SOUND_PASS,
   SOUND_WIN,
   SOUND_LOST,
   SOUND_HINT,
   SOUND_HINT2,

   NUM_SOUND,
};

enum
{
   IMG_1P = 0,
   IMG_1PD,
   IMG_2P,
   IMG_2PD,
   IMG_3P,
   IMG_NOTCALL,
   IMG_DISCARD,
   IMG_ABORT,
   IMG_PANEL,
   IMG_LOCAL,
   IMG_LEFT,
   IMG_RIGHT,
   IMG_LORD,
   IMG_BID,
   IMG_CHK_IN,
   IMG_CHK_OUT,

   NUM_IMG,
};

#include "font.h"
#include "card.h"

class CGeneral
{
public:
   CGeneral();
   ~CGeneral();

   void          ScreenFade(int duration = 500, SDL_Surface *s = NULL);
   int           ReadKey(bool fWait = true);
   void          UpdateScreen(int x = 0, int y = 0, int w = 0, int h = 0);

   void          ClearScreen(bool fadein = false, bool fadeout = false, bool bg = false);
   void          EraseArea(int x, int y, int w, int h, bool bg = false);

   void          DrawTextBrush(const char *t, int x, int y, int r = 255, int g = 255, int b = 255, int size = 32);
   void          DrawText(const char *t, int x, int y, int r = 255, int g = 255, int b = 255);
   void          DrawTextVerticalUp(const char *t, int x, int y, int r = 255, int g = 255, int b = 255);
   void          DrawTextVerticalDown(const char *t, int x, int y, int r = 255, int g = 255, int b = 255);
   SDL_Surface  *RenderCard(const CCard &c);
   void          DrawCard(const CCard &c, int x, int y, bool update = true);
   void          DrawImage(int num, int x, int y, bool update = true);

   void          PlaySound(int num);

   void          LoadSound();

   friend class  CCardImage;

private:
   void          LoadImages();
   void          FreeImages();
   void          FreeSound();

   SDL_Surface  *LoadBitmapFile(const char *filename);
   SDL_AudioCVT *LoadSoundFile(const char *filename);

   SDL_Surface  *m_imgCards, *m_imgBack;
   SDL_Surface  *m_img[NUM_IMG];
   SDL_AudioCVT *m_snd[NUM_SOUND];
   bool          m_fSndLoaded;

   CFont         m_fntBrush;
   CHZKFont      m_fnt;
   SDL_Cursor   *m_HandCursor;
};

class CBox
{
public:
   CBox(int x, int y, int w, int h, int r, int g, int b, int a = 160, bool keep = false);
   virtual ~CBox();

protected:
   SDL_Surface  *m_pSavedArea;  // save the area behind this box
   SDL_Rect      m_SavedRect;
   bool          m_fFakeBox;
};

#define MAX_BUTTONS 256

class CButton : public CBox
{
public:
   CButton(int id, int x, int y, int w, int h, int r, int g, int b);
   virtual ~CButton();

   static int bx[MAX_BUTTONS], by[MAX_BUTTONS], bw[MAX_BUTTONS], bh[MAX_BUTTONS];
   static int bid[MAX_BUTTONS], num_buttons;

   static int GetButtonId(int x, int y);

private:
   int m_iId;
};

class CCheckBox
{
public:
   CCheckBox(int x, int y, bool checked = false);
   ~CCheckBox();

   void Toggle();
   inline bool Checked() { return m_fChecked; }

   static int num_checkboxes;
   static CCheckBox *cb[MAX_BUTTONS];
   static void Clicked(int x, int y);

private:
   bool m_fChecked;
   SDL_Surface  *m_pSavedArea;  // save the area behind this box
   SDL_Rect      m_SavedRect;
   void Repaint();
};

class CCardImage
{
public:
   CCardImage(CCard c, int x, int y, int buttonid = -1, bool save = true);
   ~CCardImage();

private:
   SDL_Surface *m_pSavedArea;
   CButton *m_pButton;
   SDL_Rect m_SavedRect;
};

class CTextLabelBrush
{
public:
   CTextLabelBrush(const char *text, int x, int y, int size);
   ~CTextLabelBrush();

private:
   SDL_Surface *m_pSavedArea;
   SDL_Rect m_SavedRect;
};

class CTextBox
{
public:
   CTextBox(int x, int y, int length, const char *szDefaultText);
   ~CTextBox();

   inline const char *Text() { return m_szText; }

   static int num_textboxes, current_active;
   static CTextBox *tb[MAX_BUTTONS];

   static int GetTextBoxId(int x, int y);
   static void InKey(SDL_keysym keysym);
   static void SetActive(int id);

private:
   CBox *m_pBox;
   int m_iX, m_iY, m_iLength;
   char *m_szText;
   int m_iId;

   void AddChar(unsigned char c);
   void Backspace();
   void Repaint();
};

extern CGeneral *gpGeneral;

#endif
