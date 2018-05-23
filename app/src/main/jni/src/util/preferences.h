/* Qaquarelle
 * Copyright (C) 2009 Vasiliy Makarov <drmoriarty.0@gmail.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this software; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtCore>
#include "iKeys.h"

class Preferences: public QObject
{
	Q_OBJECT
 public:
	static Preferences& Self();
	void save();

	bool smartStroke;
	int colorPickerStyle;
	//WheelAction wheelAction;
	int settingsVersion;
	int dragKeys[DRAGSTATE_COUNT];
	int wheelKeys[WheelCOUNT];
	
 private:
	Preferences();
};

#define PREF Preferences::Self()

#endif // PREFERENCES_H
