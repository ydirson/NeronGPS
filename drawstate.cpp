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

#include <QtGlobal>
#include <QtDebug>

#ifdef Q_WS_QWS
#include <QAction>
#include <QMenu>
#include <QtopiaApplication>
#else
#include <QApplication>
#endif

#include "include/drawstate.h"
#include "include/converter.h"
#include "include/trace.h"
#include "include/tilehttpwheel.h"
#include "include/tilehttpname.h"
#include "include/global.h"

TDrawState::TDrawState()
{
	_width = -1;
	_height = -1;
	_magnification = 0;
	_magnificate = true;
	_zoom = 0;
	_centerX = 0;
	_centerY = 0;
	_auto = true;
	_validAngle = false;
	_validPos = false;
	_fix = false;
	_moving = false;
	_displayAlwaysOn = false;
	_server = NULL;

	connect(&_map, SIGNAL(sendUpdate()), this, SLOT(slotRefresh()));
}

TDrawState::~TDrawState()
{
	_map.setServer(NULL);
}

void TDrawState::loadDefault(TSettings &settings, const QString &section)
{
	QString defaultServer = TTileHttpName::defaultServer();

	settings.beginGroup(section);
	_magnification = settings.getValue("magnification", 1).toInt();
	_dontMagnificate = settings.getValue("nomagnification", "").toStringList();
	_zoom = settings.getValue("defaultzoom", 0).toInt() + magnification();
	_centerX = TConverter::prepareX(settings.getValue("defaultlongitude", 5.71).toDouble());
	_centerY = TConverter::prepareY(settings.getValue("defaultlatitude", 45.234444).toDouble());
	_displayAlwaysOn = settings.getValue("displayalwayson", false).toBool();
	QString httpName = settings.getValue("defaultserver", defaultServer).toString();
	settings.endGroup();

	if(_displayAlwaysOn) {
		QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
	} else {
		QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);
	}

	slotSwitchHttpServer(httpName);
}

void TDrawState::setTileServer(TTileServer *server)
{
	_server = server;

	_map.setServer(server);

	reloadTiles();
	emit signalUpdate();
}

void TDrawState::draw(QPainter &painter)
{
	_map.draw(painter, *this);
}

void TDrawState::setSize(int width, int height)
{
	_width = width;
	_height = height;

	_map.setSurface(_width * 3, _height * 3, _width, _height, Qt::black);

	reloadTiles();
}

void TDrawState::slotSetZoom(int zoom)
{
	_zoom = zoom;
	reloadTiles();
	emit signalUpdate();
	tMessage((void *)this, 1000) << (_zoom - magnification());
}

void TDrawState::slotZoomPlus()
{
	if(_zoom < NUMLEVELS) {
		_zoom += 1;
		reloadTiles();
		emit signalUpdate();
		tMessage((void *)this, 1000) << (_zoom - magnification());
	}
}

void TDrawState::slotZoomMinus()
{
	if(_zoom > 0) {
		_zoom -= 1;
		reloadTiles();
		emit signalUpdate();
		tMessage((void *)this, 1000) << (_zoom - magnification());
	}
}

void TDrawState::slotSetMagnification(int magnification)
{
	if(_magnification == magnification) {
		return;
	}

	_magnification = magnification;

	if(_magnificate) {
		reloadTiles();
		emit signalUpdate();
	}
}

void TDrawState::slotMove(int offsetX, int offsetY)
{
	_centerX += offsetX;
	_centerY += offsetY;

	moveTiles();
	emit signalUpdate();
}

void TDrawState::slotAutoOn()
{
	if(!_auto) {
		_auto = true;

		if(_validPos) {
			_centerX = _currentX;
			_centerY = _currentY;
			moveTiles();
			emit signalUpdate();
		}

		emit signalActionState("Auto center", false, false);
	}
}

void TDrawState::slotAutoOff()
{
	if(_auto) {
		_auto = false;
		emit signalActionState("Auto center", true, true);
	}
}

void TDrawState::slotGpsState(bool fix)
{
	if(!fix) {
		_fix = false;
		emit signalUpdate();
	}
}

void TDrawState::slotGpsData(int x, int y, qreal angle, bool angleValid)
{
	if(_auto && ((_currentX != x) || (_currentY != y))) {
		_centerX = x;
		_centerY = y;
		moveTiles();
	}

	_currentX = x;
	_currentY = y;
	_validPos = true;
	_validAngle = angleValid;
	_currentAngle = angle;
	_fix = true;

	emit signalUpdate();
}

void TDrawState::slotCenterTo(int xmin, int xmax, int ymin, int ymax)
{
	_centerX = (xmin + xmax) / 2;
	_centerY = (ymin + ymax) / 2;
	_zoom = TConverter::autoZoom(xmax - xmin, ymax - ymin, _width, _height);

	reloadTiles();
	emit signalUpdate();
}

void TDrawState::slotTriggerBatchLoading()
{
	int x = (TConverter::convert(_centerX, _zoom) - _width / 2) >> magnification();
	int y = (TConverter::convert(_centerY, _zoom) - _height / 2) >> magnification(); 
	int w = _width >> magnification();  
	int h = _height >> magnification(); 
	int z = _zoom - magnification();

	emit signalBatchLoading(x, y, w, h, z);
}

void TDrawState::slotRefresh()
{
	emit signalUpdate();
}

void TDrawState::slotMoving(bool moving)
{
	_moving = moving;

	if(_moving) {
		_map.flush();
	}
}

void TDrawState::slotDisplayAlwaysOn(bool alwaysOn)
{
	_displayAlwaysOn = alwaysOn;

	if(_displayAlwaysOn) {
		QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
	} else {
		QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);
	}
}

void TDrawState::slotSwitchHttpServer(const QString &name)
{
	_httpName = name;

	_magnificate = _dontMagnificate.indexOf(name) == -1;

	reloadTiles();
	emit signalUpdate();
}

void TDrawState::reloadTiles()
{
	if((!_moving) && (_server != NULL) && (_width != -1)) {
		_map.loadTiles(_httpName, _centerX, _centerY, _zoom, magnification());
	}
}

void TDrawState::moveTiles()
{
	if((!_moving) && (_server != NULL) && (_width != -1)) {
		_map.moveTo(_centerX, _centerY);
	}
}

