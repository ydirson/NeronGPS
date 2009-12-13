/*
 * Copyright 2009 Thierry Vuillaume
 *
 * This file is part of NeronGPS.
 *
 * NeronGPS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NeronGPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NeronGPS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LOCATIONPOINTER_H
#define LOCATIONPOINTER_H

#include <QObject>

#include "include/drawstate.h"
#include "include/painter.h"

class TLocationPointer : public QObject
{
	Q_OBJECT
	public:
		TLocationPointer();
		~TLocationPointer();
		
		void configure(TSettings &settings, const QString &section);
		void draw(QPainter &painter, TDrawState &drawState);
		
	private:
		TPainter _locationPaint;
		int _locationSize;
		QColor _fixColor;
		QColor _lostColor;

		TPainter _drivePaint;
		int _driveSize;
		QColor _driveColor;

		int _shadowLevel;
		int _shadowX;
		int _shadowY;

		void drawPointer(QPainter &painter, TPainter &paint, QColor &color, qreal angle, int x, int y, int posX, int posY, int size);
};

#endif

