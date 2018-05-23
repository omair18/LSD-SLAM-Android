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

#include "preferences.h"

Preferences::Preferences()
{
    // [castedflake] Set box-style color picker as default.
    // [castedflake] Set canvas scaling by mouse-scrolling as default.

    QSettings set;
    smartStroke = set.value("smartStroke", false).toBool();
    colorPickerStyle = set.value("colorPickerStyle", 1).toInt();
    //wheelAction = (WheelAction)set.value("wheelAction", WheelScalesCanvas).toInt();
    settingsVersion = set.value("settingsVersion", 0).toInt();
	for(int i=0; i<DRAGSTATE_COUNT; i++) {
		dragKeys[i] = set.value(QString("dragKey_%1").arg(QString::number(i)), -1).toInt();
	}
	for(int i=0; i<WheelCOUNT; i++) {
		wheelKeys[i] = set.value(QString("wheelKey_%1").arg(QString::number(i)), -1).toInt();
	}
}

void Preferences::save()
{
	QSettings set;
	set.setValue("smartStroke", smartStroke);
	set.setValue("colorPickerStyle", colorPickerStyle);
	//set.setValue("wheelAction", (int)wheelAction);
	set.setValue("settingsVersion", settingsVersion);
	for(int i=0; i<DRAGSTATE_COUNT; i++) {
		set.setValue(QString("dragKey_%1").arg(QString::number(i)), dragKeys[i]);
	}
	for(int i=0; i<WheelCOUNT; i++) {
		set.setValue(QString("wheelKey_%1").arg(QString::number(i)), wheelKeys[i]);
	}
}

Preferences& Preferences::Self()
{
	static Preferences instance;
	return instance;
}
