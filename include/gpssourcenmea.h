/*
 * Copyright 2009, 2010 Thierry Vuillaume
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

#ifndef GPSSOURCENMEA_H
#define GPSSOURCENMEA_H

#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QMutex>

#include "include/gpssourceplugin.h"
#include "include/nmeaparser.h"

class TGpsSourceNmea : public TGpsSourcePlugin
{
	Q_OBJECT
	public:
		TGpsSourceNmea();
		~TGpsSourceNmea();

		QString name() { return QString("nmea file"); }

		void setFile(const QString &fileName);
		void start();
		void stop();

		void startRawRecording(const QString &/*filename*/) { };
		void stopRawRecording() { };

	public slots:
		void slotTimer();

	signals:
		void signalUpdate(TGpsSample sample);

	private:
		QMutex _mutex;
		QTimer _timer;
		TNmeaParser _parser;
		QFile *_file;
		QDateTime _lastTime;
		TGpsSample _sample;

		void parse();
		int interval(QDateTime first, QDateTime next);
};

#endif

