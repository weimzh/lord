#
# Copyright (c) 2009, Wei Mingzhi. All rights reserved.
#
# Use, redistributions and modifications of this file are
# unrestricted provided the above copyright notice is
# preserved.
#

PREFIX=/usr/
BINDIR=${PREFIX}/bin/
SHAREDIR=${PREFIX}/share/

OBJ = \
	src/bot.o src/card.o src/compress.o src/font.o src/game.o \
	src/general.o src/ini.o src/localplr.o src/main.o src/player.o \
	src/sound.o src/text.o src/ui.o src/util.o src/hzk.o

CC = gcc
CXX = g++
INSTALL = install

TARGET = lord

ifeq ($(CONF),debug)
BASEFLAGS = -g3 -D_DEBUG=1
else
BASEFLAGS = -s -O3 -DRELEASE=1
endif

CFLAGS = ${BASEFLAGS} `sdl-config --cflags`
LDFLAGS = ${BASEFLAGS} `sdl-config --libs` -lSDL_image

all: ${TARGET}

${TARGET}: ${OBJ}
	${CXX} ${LDFLAGS} -o ${TARGET} ${OBJ}

clean:
	-rm -rf src/*.o ${TARGET}
	-rm -rf .objs lord.layout lord.depend

distclean: clean

install:
	${INSTALL} -m755 -D lord ${DESTDIR}/${BINDIR}/lord
	${INSTALL} -m644 -D data/lord.ini ${DESTDIR}/${SHAREDIR}/lord/data/lord.ini
	${INSTALL} -m644 -D data/titleschs.txt ${DESTDIR}/${SHAREDIR}/lord/data/titleschs.txt
	${INSTALL} -m644 -D fonts/brush.fnt ${DESTDIR}/${SHAREDIR}/lord/fonts/brush.fnt
	${INSTALL} -m644 -D fonts/hzk16.fnt ${DESTDIR}/${SHAREDIR}/lord/fonts/hzk16
	${INSTALL} -m644 -D images/1pc.png ${DESTDIR}/${SHAREDIR}/lord/images/1pc.png
	${INSTALL} -m644 -D images/1pdc.png ${DESTDIR}/${SHAREDIR}/lord/images/1pdc.png
	${INSTALL} -m644 -D images/2pc.png ${DESTDIR}/${SHAREDIR}/lord/images/2pc.png
	${INSTALL} -m644 -D images/2pdc.png ${DESTDIR}/${SHAREDIR}/lord/images/2pdc.png
	${INSTALL} -m644 -D images/3pc.png ${DESTDIR}/${SHAREDIR}/lord/images/3pc.png
	${INSTALL} -m644 -D images/abortc.png ${DESTDIR}/${SHAREDIR}/lord/images/abortc.png
	${INSTALL} -m644 -D images/back.jpg ${DESTDIR}/${SHAREDIR}/lord/images/back.jpg
	${INSTALL} -m644 -D images/call.png ${DESTDIR}/${SHAREDIR}/lord/images/call.png
	${INSTALL} -m644 -D images/cards.png ${DESTDIR}/${SHAREDIR}/lord/images/cards.png
	${INSTALL} -m644 -D images/discardc.png ${DESTDIR}/${SHAREDIR}/lord/images/discardc.png
	${INSTALL} -m644 -D images/left.png ${DESTDIR}/${SHAREDIR}/lord/images/left.png
	${INSTALL} -m644 -D images/right.png ${DESTDIR}/${SHAREDIR}/lord/images/right.png
	${INSTALL} -m644 -D images/panel.png ${DESTDIR}/${SHAREDIR}/lord/images/panel.png
	${INSTALL} -m644 -D images/local.png ${DESTDIR}/${SHAREDIR}/lord/images/local.png
	${INSTALL} -m644 -D images/lord.png ${DESTDIR}/${SHAREDIR}/lord/images/lord.png
	${INSTALL} -m644 -D images/notcallc.png ${DESTDIR}/${SHAREDIR}/lord/images/notcallc.png
	${INSTALL} -m644 -D sound/bomb.wav ${DESTDIR}/${SHAREDIR}/lord/sound/bomb.wav
	${INSTALL} -m644 -D sound/call.wav ${DESTDIR}/${SHAREDIR}/lord/sound/call.wav
	${INSTALL} -m644 -D sound/card1.wav ${DESTDIR}/${SHAREDIR}/lord/sound/card1.wav
	${INSTALL} -m644 -D sound/card2.wav ${DESTDIR}/${SHAREDIR}/lord/sound/card2.wav
	${INSTALL} -m644 -D sound/hint.wav ${DESTDIR}/${SHAREDIR}/lord/sound/hint.wav
	${INSTALL} -m644 -D sound/hint2.wav ${DESTDIR}/${SHAREDIR}/lord/sound/hint2.wav
	${INSTALL} -m644 -D sound/lost.wav ${DESTDIR}/${SHAREDIR}/lord/sound/lost.wav
	${INSTALL} -m644 -D sound/pass.wav ${DESTDIR}/${SHAREDIR}/lord/sound/pass.wav
	${INSTALL} -m644 -D sound/win.wav ${DESTDIR}/${SHAREDIR}/lord/sound/win.wav

%.o:    %.cpp
	${CXX} ${CFLAGS} -c $< -o $@

%.o:    %.c
	${CC} ${CFLAGS} -c $< -o $@
