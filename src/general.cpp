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

#include "main.h"

CGeneral *gpGeneral = NULL;

const char *soundfile[NUM_SOUND + 1] = {
   "card1",
   "bomb",
   "call",
   "pass",
   "win",
   "lost",
   "hint",
   "hint2",
   "",
};

const char *imgfile[NUM_IMG + 1] = {
   "1p",
   "1pd",
   "2p",
   "2pd",
   "3p",
   "notcall",
   "discard",
   "abort",
   "panel",
   "local",
   "left",
   "right",
   "lord",
   "call",
   "chk_in",
   "chk_out",
   "",
};

CGeneral::CGeneral():
m_fntBrush(FONTS_DIR "brush.fnt"),
m_fnt(FONTS_DIR "hzk16.fnt")
{
   memset(m_img, 0, sizeof(m_img));
   m_fSndLoaded = false;
   m_imgCards = NULL;
   m_imgBack = NULL;
   LoadImages();
   LoadSound();
}

CGeneral::~CGeneral()
{
   FreeImages();
   FreeSound();
}

void CGeneral::ScreenFade(int duration, SDL_Surface *s)
{
   SDL_Surface *pNewFadeSurface = SDL_CreateRGBSurface(gpScreen->flags & (~SDL_HWSURFACE),
      gpScreen->w, gpScreen->h, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);

   if (!pNewFadeSurface) {
      // cannot create surface, just blit the surface to the screen
      if (s != NULL) {
         SDL_BlitSurface(s, NULL, gpScreen, NULL);
         SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
      }
      return;
   }

   if (s == NULL) {
      // make black screen
      SDL_FillRect(pNewFadeSurface, NULL,
         SDL_MapRGB(pNewFadeSurface->format, 0, 0, 0));
   } else {
      SDL_BlitSurface(s, NULL, pNewFadeSurface, NULL);
   }

   if (SDL_MUSTLOCK(gpScreen)) {
      if (SDL_LockSurface(gpScreen) < 0) {
         // cannot lock screen, just blit the surface to the screen
         if (s != NULL) {
            SDL_BlitSurface(s, NULL, gpScreen, NULL);
            SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
         }
         return;
      }
   }

   const unsigned long size = gpScreen->pitch * gpScreen->h;
   unsigned char *fadeFromRGB = (unsigned char *)calloc(size, 1);
   unsigned char *fadeToRGB = (unsigned char *)calloc(size, 1);
   if (fadeFromRGB == NULL || fadeToRGB == NULL) {
      TerminateOnError("Memory allocation error !");
   }

   memcpy(fadeFromRGB, gpScreen->pixels, size);
   memcpy(fadeToRGB, pNewFadeSurface->pixels, size);

   int first = SDL_GetTicks(), now = first;

   do {
      // The +50 is to allow first frame to show some change
      float ratio = (now - first + 50) / (float)duration;
      const unsigned char amount = (unsigned char)(ratio * 255);
      const unsigned char oldamount = 255 - amount;
      unsigned char *pw = (unsigned char *)gpScreen->pixels;
      unsigned char *stop = pw + size;
      unsigned char *from = fadeFromRGB;
      unsigned char *to = fadeToRGB;

      do {
         //dividing by 256 instead of 255 provides huge optimization
         *pw = (oldamount * *(from++) + amount * *(to++)) / 256;
      } while (++pw != stop);

      now = SDL_GetTicks();
      SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);
   } while (now - first + 50 < duration);

   free(fadeFromRGB);
   free(fadeToRGB);

   SDL_BlitSurface(pNewFadeSurface, NULL, gpScreen, NULL);
   SDL_UpdateRect(gpScreen, 0, 0, gpScreen->w, gpScreen->h);

   if (SDL_MUSTLOCK(gpScreen))
      SDL_UnlockSurface(gpScreen);

   SDL_FreeSurface(pNewFadeSurface);
}

int CGeneral::ReadKey(bool fWait)
{
   SDL_Event event;
   int ret;

   if (!fWait) {
      // clear the event queue
      while (SDL_PollEvent(&event)) {
      }
   }

   ret = fWait ? SDL_WaitEvent(&event) : SDL_PollEvent(&event);
   do {
      if (event.type == SDL_KEYDOWN) {
         break;
      } else if (event.type == SDL_MOUSEBUTTONDOWN) {
         if (event.button.button == SDL_BUTTON_RIGHT) {
            return SDLK_RIGHT;
         } else if (event.button.button == SDL_BUTTON_LEFT) {
            CCheckBox::Clicked(event.button.x, event.button.y);
            CTextBox::SetActive(CTextBox::GetTextBoxId(event.button.x, event.button.y));
            int id = CButton::GetButtonId(event.button.x, event.button.y);
            if (id < 0) {
               return SDLK_RETURN;
            }
            return SDLK_LAST + id;
         }
      }
      ret = fWait ? SDL_WaitEvent(&event) : SDL_PollEvent(&event);
   } while (ret);

   CTextBox::InKey(event.key.keysym);
   return event.key.keysym.sym;
}

void CGeneral::UpdateScreen(int x, int y, int w, int h)
{
   SDL_UpdateRect(gpScreen, x, y, w, h);
}

void CGeneral::ClearScreen(bool fadein, bool fadeout, bool bg)
{
   SDL_Surface *r = SDL_CreateRGBSurface(gpScreen->flags,
      gpScreen->w, gpScreen->h, gpScreen->format->BitsPerPixel,
      gpScreen->format->Rmask, gpScreen->format->Gmask,
      gpScreen->format->Bmask, gpScreen->format->Amask);

   if (bg) {
      int w = r->w;
      while (w > 0) {
         int h = r->h;
         while (h > 0) {
            SDL_Rect dstrect;
            dstrect.x = r->w - w;
            dstrect.y = r->h - h;
            dstrect.w = m_imgBack->w;
            dstrect.h = m_imgBack->h;
            SDL_BlitSurface(m_imgBack, NULL, r, &dstrect);
            h -= m_imgBack->h;
         }
         w -= m_imgBack->w;
      }
   } else {
      UTIL_FillRect(r, 0, 0, gpScreen->w, gpScreen->h, 30, 130, 100);
   }

   UTIL_RectShade(r, 0, 0, gpScreen->w, gpScreen->h, 196, 196,
      0, 0, 196, 196, 196, 0, 196);

   if (fadeout) {
      ScreenFade(300);
   }

   ScreenFade(fadein ? 300 : 0, r);
   SDL_FreeSurface(r);
}

void CGeneral::EraseArea(int x, int y, int w, int h, bool bg)
{
   if (bg) {
      // display the background image
      SDL_Rect dstrect;
      dstrect.x = x;
      dstrect.y = y;
      dstrect.w = w;
      dstrect.h = h;
      SDL_BlitSurface(m_imgBack, &dstrect, gpScreen, &dstrect);
   } else {
      UTIL_FillRect(gpScreen, x, y, w, h, 30, 130, 100);
   }
}

void CGeneral::DrawTextBrush(const char *t, int x, int y, int r, int g, int b, int size)
{
   SDL_Surface *s = m_fntBrush.Render(t, r, g, b, size, ((size < 32) ? false : true));

   SDL_Rect dstrect;
   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = s->w;
   dstrect.h = s->h;

   SDL_BlitSurface(s, NULL, gpScreen, &dstrect);
   SDL_FreeSurface(s);

   UpdateScreen(x, y, dstrect.w, dstrect.h);
}

void CGeneral::DrawText(const char *t, int x, int y, int r, int g, int b)
{
   unsigned char c[2], *glyph;
   int w = 8, i, j, oldx = x;
   unsigned int l = SDL_MapRGBA(gpScreen->format, r, g, b, 0);

   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_LockSurface(gpScreen);
   }

   while ((c[0] = *(t++)) != '\0') {
      if (c[0] < 0xa1 || c[0] > 0xfe || !*t) {
         // ASCII character
         glyph = m_fnt.ReadASCGlyph(c[0]);
         w = 8;
      } else {
         // GB character
         c[1] = *t++;
         glyph = m_fnt.ReadGBGlyph(c);
         w = 16;
      }
      if (glyph == NULL) {
         continue; // cannot read such a character
      }
      if (w == 8) {
         for (i = 0; i < 16; i++) {
            for (j = 0; j < 8; j++) {
               if (glyph[i] & (1 << j)) {
                  UTIL_PutPixel(gpScreen, x + j, y + i, l);
               }
            }
         }
      } else {
         for (i = 0; i < 32; i++) {
            for (j = 0; j < 8; j++) {
               if (glyph[i] & (1 << (7 - j))) {
                  UTIL_PutPixel(gpScreen, x + j + 8 * (i & 1), y + i / 2, l);
               }
            }
         }
      }
      x += w;
   }

   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_UnlockSurface(gpScreen);
   }

   UpdateScreen(oldx, y, x - oldx, 16);
}

void CGeneral::DrawTextVerticalUp(const char *t, int x, int y, int r, int g, int b)
{
   unsigned char c[2], *glyph;
   int w = 8, i, j, oldy = y;
   unsigned int l = SDL_MapRGBA(gpScreen->format, r, g, b, 0);

   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_LockSurface(gpScreen);
   }

   while ((c[0] = *(t++)) != '\0') {
      if (c[0] < 0xa1 || c[0] > 0xfe || !*t) {
         // ASCII character
         glyph = m_fnt.ReadASCGlyph(c[0]);
         w = 8;
      } else {
         // GB character
         c[1] = *t++;
         glyph = m_fnt.ReadGBGlyph(c);
         w = 16;
      }
      if (w == 8) {
         for (i = 0; i < 16; i++) {
            for (j = 0; j < 8; j++) {
               if (glyph[i] & (1 << j)) {
                  UTIL_PutPixel(gpScreen, x + i, y - j, l);
               }
            }
         }
      } else {
         for (i = 0; i < 32; i++) {
            for (j = 0; j < 8; j++) {
               if (glyph[i] & (1 << (7 - j))) {
                  UTIL_PutPixel(gpScreen, x + i / 2, y - j - 8 * (i & 1), l);
               }
            }
         }
      }
      y -= w;
   }

   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_UnlockSurface(gpScreen);
   }

   UpdateScreen(x, y, 16, oldy - y);
}

void CGeneral::DrawTextVerticalDown(const char *t, int x, int y, int r, int g, int b)
{
   unsigned char c[2], *glyph;
   int w = 8, i, j, oldy = y;
   unsigned int l = SDL_MapRGBA(gpScreen->format, r, g, b, 0);

   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_LockSurface(gpScreen);
   }

   while ((c[0] = *(t++)) != '\0') {
      if (c[0] < 0xa1 || c[0] > 0xfe || !*t) {
         // ASCII character
         glyph = m_fnt.ReadASCGlyph(c[0]);
         w = 8;
      } else {
         // GB character
         c[1] = *t++;
         glyph = m_fnt.ReadGBGlyph(c);
         w = 16;
      }
      if (w == 8) {
         for (i = 0; i < 16; i++) {
            for (j = 0; j < 8; j++) {
               if (glyph[i] & (1 << j)) {
                  UTIL_PutPixel(gpScreen, x + 16 - i, y + j, l);
               }
            }
         }
      } else {
         for (i = 0; i < 32; i++) {
            for (j = 0; j < 8; j++) {
               if (glyph[i] & (1 << (7 - j))) {
                  UTIL_PutPixel(gpScreen, x + 16 - i / 2, y + j + 8 * (i & 1), l);
               }
            }
         }
      }
      y += w;
   }

   if (SDL_MUSTLOCK(gpScreen)) {
      SDL_UnlockSurface(gpScreen);
   }

   UpdateScreen(x, oldy, 16, y - oldy);
}

static int color = -1;

SDL_Surface *CGeneral::RenderCard(const CCard &c)
{
   SDL_Surface *s;
   int pw = m_imgCards->w / 13, ph = m_imgCards->h / 5;

   if (color == -1) {
      color = (RandomLong(1, 100) > 50) ? 2 : 3;
   }

   s = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE,
      pw, ph, gpScreen->format->BitsPerPixel,
      gpScreen->format->Rmask, gpScreen->format->Gmask,
      gpScreen->format->Bmask, gpScreen->format->Amask);

   SDL_SetColorKey(s, SDL_SRCCOLORKEY, SDL_MapRGBA(s->format, 0, 0, 255, 0));
   UTIL_FillRect(s, 0, 0, pw, ph, 0, 0, 255);

   SDL_Rect dstrect;

   if (!c.IsValid()) {
      dstrect.x = pw * color;
      dstrect.y = ph * 4;
   } else {
      dstrect.x = (c.GetNum() - 1) * pw;
      dstrect.y = c.GetType() * ph;
   }

   dstrect.w = pw;
   dstrect.h = ph;

   SDL_BlitSurface(m_imgCards, &dstrect, s, NULL);
   return s;
}

void CGeneral::DrawCard(const CCard &c, int x, int y, bool update)
{
   SDL_Rect dstrect, dstrect2;
   int pw = m_imgCards->w / 13, ph = m_imgCards->h / 5;

   if (color == -1) {
      color = (RandomLong(1, 100) > 50) ? 2 : 3;
   }

   if (!c.IsValid()) {
      dstrect.x = pw * color;
      dstrect.y = ph * 4;
   } else {
      dstrect.x = (c.GetNum() - 1) * pw;
      dstrect.y = c.GetType() * ph;
   }
   dstrect.w = dstrect2.w = pw;
   dstrect.h = dstrect2.h = ph;

   dstrect2.x = x;
   dstrect2.y = y;

   SDL_BlitSurface(m_imgCards, &dstrect, gpScreen, &dstrect2);

   if (update) {
      UpdateScreen(x, y, dstrect2.w, dstrect2.h);
   }
}

void CGeneral::DrawImage(int num, int x, int y, bool update)
{
   if (num < 0 || num >= NUM_IMG) {
      TerminateOnError("CGeneral::DrawImage(): num < 0 || num >= NUM_IMG");
   }
   SDL_Rect dstrect;
   dstrect.x = x;
   dstrect.y = y;
   dstrect.w = m_img[num]->w;
   dstrect.h = m_img[num]->h;
   SDL_BlitSurface(m_img[num], NULL, gpScreen, &dstrect);
   if (update) {
      UpdateScreen(x, y, dstrect.w, dstrect.h);
   }
}

void CGeneral::LoadImages()
{
   int i;
   FreeImages();
   m_imgCards = LoadBitmapFile(IMAGES_DIR "cards.png");
   m_imgBack = LoadBitmapFile(IMAGES_DIR "back.jpg");
   for (i = 0; i < NUM_IMG; i++) {
      m_img[i] = LoadBitmapFile(va("%s%s", IMAGES_DIR, msg(va("img_%s", imgfile[i]))));
   }
}

void CGeneral::FreeImages()
{
   int i;
   if (m_imgCards != NULL) {
      SDL_FreeSurface(m_imgCards);
   }
   m_imgCards = NULL;
   if (m_imgBack != NULL) {
      SDL_FreeSurface(m_imgBack);
   }
   m_imgBack = NULL;
   for (i = 0; i < NUM_IMG; i++) {
      if (m_img[i] != NULL) {
         SDL_FreeSurface(m_img[i]);
      }
   }
}

void CGeneral::LoadSound()
{
   if (m_fSndLoaded) {
      return;
   }

   m_fSndLoaded = true;

   int i;

   for (i = 0; i < NUM_SOUND; i++) {
      assert(*soundfile[i]);
      m_snd[i] = LoadSoundFile(va("%s%s.wav", SOUND_DIR, soundfile[i]));
   }
}

void CGeneral::FreeSound()
{
   if (!m_fSndLoaded) {
      return;
   }

   int i;

   for (i = 0; i < NUM_SOUND; i++) {
      if (m_snd[i] != NULL) {
         SOUND_FreeWAV(m_snd[i]);
      }
   }
}

void CGeneral::PlaySound(int num)
{
   assert(num >= 0 && num < NUM_SOUND);
   if (g_fNoSound) {
      return;
   }
   SOUND_PlayWAV(m_snd[num]);
}

SDL_Surface *CGeneral::LoadBitmapFile(const char *filename)
{
   SDL_Surface *pic = IMG_Load(filename);

   if (pic == NULL) {
      TerminateOnError("Cannot load Bitmap file %s: %s", filename, SDL_GetError());
   }

   return pic;
}

SDL_AudioCVT *CGeneral::LoadSoundFile(const char *filename)
{
   SDL_AudioCVT *s = SOUND_LoadWAV(filename);

   if (s == NULL) {
      TerminateOnError("Cannot load sound file %s: %s",
         filename, SDL_GetError());
   }

   return s;
}

CBox::CBox(int x, int y, int w, int h, int r, int g, int b, int a, bool keep)
{
   if (r == 0 && g == 0 && b == 0) {
      m_fFakeBox = true;
      return;
   }

   m_fFakeBox = false;

   if (keep) {
      m_pSavedArea = NULL;
   } else {
      m_pSavedArea = SDL_CreateRGBSurface(gpScreen->flags,
         w, h, gpScreen->format->BitsPerPixel,
         gpScreen->format->Rmask, gpScreen->format->Gmask,
         gpScreen->format->Bmask, gpScreen->format->Amask);

      m_SavedRect.x = x;
      m_SavedRect.y = y;
      m_SavedRect.w = w;
      m_SavedRect.h = h;

      SDL_BlitSurface(gpScreen, &m_SavedRect, m_pSavedArea, NULL);
   }

   UTIL_FillRectAlpha(gpScreen, x, y, w, h, r, g, b, a);
   UTIL_RectShade(gpScreen, x, y, w, h, 255, 255, 255, 0, 0, 0,
      128, 128, 128);

   gpGeneral->UpdateScreen(x, y, w, h);
}

CBox::~CBox()
{
   if (m_fFakeBox || m_pSavedArea == NULL) {
      return;
   }

   SDL_BlitSurface(m_pSavedArea, NULL, gpScreen, &m_SavedRect);
   SDL_FreeSurface(m_pSavedArea);
   gpGeneral->UpdateScreen(m_SavedRect.x, m_SavedRect.y, m_SavedRect.w, m_SavedRect.h);
}

int CButton::bx[MAX_BUTTONS], CButton::by[MAX_BUTTONS];
int CButton::bw[MAX_BUTTONS], CButton::bh[MAX_BUTTONS];
int CButton::bid[MAX_BUTTONS], CButton::num_buttons = 0;

CButton::CButton(int id, int x, int y, int w, int h, int r, int g, int b):
CBox(x, y, w, h, r, g, b, 128, false),
m_iId(id)
{
   if (num_buttons >= MAX_BUTTONS) {
      TerminateOnError("CButton::CButton(): num_buttons > MAX_BUTTONS!");
   }
   bid[num_buttons] = id;
   bx[num_buttons] = x;
   by[num_buttons] = y;
   bw[num_buttons] = w;
   bh[num_buttons] = h;
   num_buttons++;
}

CButton::~CButton()
{
   int i;
   for (i = 0; i < num_buttons; i++) {
      if (bid[i] == m_iId) {
         break;
      }
   }
   if (i >= num_buttons) {
      TerminateOnError("CButton::~CButton(): i >= num_buttons");
   }

   while (i < num_buttons - 1) {
      bid[i] = bid[i + 1];
      bx[i] = bx[i + 1];
      by[i] = by[i + 1];
      bw[i] = bw[i + 1];
      bh[i] = bh[i + 1];
      i++;
   }

   num_buttons--;
}

int CButton::GetButtonId(int x, int y)
{
   int i;

   // search backwards so that newer buttons can supercede
   // older ones
   for (i = num_buttons - 1; i >= 0; i--) {
      if (x > bx[i] && x < bx[i] + bw[i] &&
         y > by[i] && y < by[i] + bh[i]) {
         return bid[i];
      }
   }

   return -1; // not found
}

int CCheckBox::num_checkboxes = 0;
CCheckBox *CCheckBox::cb[MAX_BUTTONS];

CCheckBox::CCheckBox(int x, int y, bool checked)
{
   // save the screen area
   m_SavedRect.x = x;
   m_SavedRect.y = y;
   m_SavedRect.w = 14;
   m_SavedRect.h = 14;
   m_pSavedArea = SDL_CreateRGBSurface(gpScreen->flags,
      14, 14, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);
   SDL_BlitSurface(gpScreen, &m_SavedRect, m_pSavedArea, NULL);

   cb[num_checkboxes] = this;
   num_checkboxes++;

   m_fChecked = checked;
   Repaint();
}

CCheckBox::~CCheckBox()
{
   if (m_pSavedArea != NULL) {
      // restore the original screen image and free the saved surface
      SDL_BlitSurface(m_pSavedArea, NULL, gpScreen, &m_SavedRect);
      SDL_FreeSurface(m_pSavedArea);
   }

   int i = 0;
   while (i < num_checkboxes && cb[i] != this) {
      i++;
   }

   while (i < num_checkboxes - 1) {
      cb[i] = cb[i + 1];
      i++;
   }

   num_checkboxes--;
}

void CCheckBox::Toggle()
{
   m_fChecked = !m_fChecked;
   Repaint();
}

void CCheckBox::Clicked(int x, int y)
{
   int i;
   // search backwards so that newer checkboxes can supercede older ones
   for (i = num_checkboxes - 1; i >= 0; i--) {
      if (x > cb[i]->m_SavedRect.x && x < cb[i]->m_SavedRect.x + 14 &&
         y > cb[i]->m_SavedRect.y && y < cb[i]->m_SavedRect.y + 14) {
         cb[i]->Toggle();
         break;
      }
   }
}

void CCheckBox::Repaint()
{
   gpGeneral->DrawImage(m_fChecked ? IMG_CHK_IN : IMG_CHK_OUT,
      m_SavedRect.x, m_SavedRect.y);
}

CCardImage::CCardImage(CCard c, int x, int y, int buttonid, bool save):
m_pSavedArea(NULL), m_pButton(NULL)
{
   int pw = gpGeneral->m_imgCards->w / 13, ph = gpGeneral->m_imgCards->h / 5;
   if (buttonid >= 0) {
      // create the hot area
      m_pButton = new CButton(buttonid, x, y, pw, ph, 0, 0, 0);
   }
   if (save) {
      // save the screen area
      m_SavedRect.x = x;
      m_SavedRect.y = y;
      m_SavedRect.w = pw;
      m_SavedRect.h = ph;
      m_pSavedArea = SDL_CreateRGBSurface(gpScreen->flags,
         pw, ph, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
         gpScreen->format->Gmask, gpScreen->format->Bmask,
         gpScreen->format->Amask);
      SDL_BlitSurface(gpScreen, &m_SavedRect, m_pSavedArea, NULL);
   }
   // Draw the card. As we may need to draw many cards, don't update
   // the screen at this time.
   gpGeneral->DrawCard(c, x, y, false);
}

CCardImage::~CCardImage()
{
   if (m_pSavedArea != NULL) {
      // restore the original screen image and free the saved surface
      SDL_BlitSurface(m_pSavedArea, NULL, gpScreen, &m_SavedRect);
      SDL_FreeSurface(m_pSavedArea);
   }
   m_pSavedArea = NULL;
   if (m_pButton != NULL) {
      delete m_pButton;
   }
   m_pButton = NULL;
}

CTextLabelBrush::CTextLabelBrush(const char *text, int x, int y, int size):
m_pSavedArea(NULL)
{
   int pw;

   UTIL_StrGetLine(text, 255, pw);
   pw *= size;

   // save the screen area
   m_SavedRect.x = x;
   m_SavedRect.y = y;
   m_SavedRect.w = pw;
   m_SavedRect.h = size;
   m_pSavedArea = SDL_CreateRGBSurface(gpScreen->flags,
      pw, size, gpScreen->format->BitsPerPixel, gpScreen->format->Rmask,
      gpScreen->format->Gmask, gpScreen->format->Bmask,
      gpScreen->format->Amask);
   SDL_BlitSurface(gpScreen, &m_SavedRect, m_pSavedArea, NULL);

   gpGeneral->DrawTextBrush(text, x, y, 255, 255, 20, size);
}

CTextLabelBrush::~CTextLabelBrush()
{
   if (m_pSavedArea != NULL) {
      // restore the original screen image and free the saved surface
      SDL_BlitSurface(m_pSavedArea, NULL, gpScreen, &m_SavedRect);
      SDL_FreeSurface(m_pSavedArea);
   }
   m_pSavedArea = NULL;
}

int CTextBox::num_textboxes = 0, CTextBox::current_active = -1;
CTextBox *CTextBox::tb[MAX_BUTTONS];

CTextBox::CTextBox(int x, int y, int length, const char *szDefaultText)
{
   m_pBox = NULL;
   m_iId = num_textboxes;

   tb[num_textboxes] = this;
   num_textboxes++;

   m_iX = x;
   m_iY = y;
   m_iLength = length;

   m_szText = new char[length];
   if (szDefaultText != NULL) {
      strncpy(m_szText, szDefaultText, length - 1);
      m_szText[length - 1] = '\0';
   } else {
      m_szText[0] = '\0';
   }

   Repaint();
}

CTextBox::~CTextBox()
{
   delete m_pBox;
   delete []m_szText;
   if (current_active == m_iId) {
      current_active = -1;
   } else if (current_active > m_iId) {
      current_active--;
   }

   int i = m_iId;

   while (i < num_textboxes - 1) {
      tb[i] = tb[i + 1];
      i++;
   }

   num_textboxes--;
}

int CTextBox::GetTextBoxId(int x, int y)
{
   int i;

   // search backwards so that newer textboxes can supercede older ones
   for (i = num_textboxes - 1; i >= 0; i--) {
      if (x > tb[i]->m_iX && x < tb[i]->m_iX + tb[i]->m_iLength * 8 + 5 &&
         y > tb[i]->m_iY && y < tb[i]->m_iY + 20) {
         return i;
      }
   }

   return -1; // not found
}

void CTextBox::InKey(SDL_keysym keysym)
{
   if (current_active < 0) {
      return;
   }
   if (keysym.sym == SDLK_BACKSPACE) {
      tb[current_active]->Backspace();
   } else if (keysym.sym >= 'a' && keysym.sym <= 'z') {
      tb[current_active]->AddChar((keysym.mod & KMOD_SHIFT) ? keysym.sym - 'a' + 'A' : keysym.sym);
   } else if (keysym.sym >= '0' && keysym.sym <= '9' || keysym.sym == '.' || keysym.sym == ' ') {
      tb[current_active]->AddChar(keysym.sym);
   }
}

void CTextBox::SetActive(int id)
{
   int old = current_active;

   if (id >= 0 && id < num_textboxes) {
      current_active = id;
      tb[id]->Repaint();
   } else {
      current_active = -1;
   }

   if (old >= 0 && old < num_textboxes) {
      tb[old]->Repaint();
   }
}

void CTextBox::AddChar(unsigned char c)
{
   int l = strlen(m_szText);
   if (l < m_iLength - 1) {
      m_szText[l] = c;
      m_szText[l + 1] = '\0';
   }
   Repaint();
}

void CTextBox::Backspace()
{
   int l = strlen(m_szText);
   if (l > 0) {
      m_szText[l - 1] = '\0';
   }
   Repaint();
}

void CTextBox::Repaint()
{
   if (m_pBox != NULL) {
      delete m_pBox;
   }
   m_pBox = new CBox(m_iX, m_iY, m_iLength * 8 + 5, 20, 50, 220, 200);
   gpGeneral->DrawText(va("%s%s", m_szText, current_active == m_iId ? "_" : ""),
      m_iX + 2, m_iY + 2, 255, 255, 255);
}
