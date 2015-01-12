/************************************************************************/
/* InControl Android X10 home automation app, v0.1 beta.                */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet (code@lookout.no) January 2011.             */
/************************************************************************/

package no.lookout.incontrol;

import android.content.Context;
import android.view.View;
import android.widget.TableLayout;

public class ModuleTable extends TableLayout {

	public ModuleTable(Context context) {
		super(context);
	}
	
	public ModuleRow getModuleRow(Module module) {
		for(int i = 0; i < getChildCount(); i++) {
			ModuleRow row = (ModuleRow)getChildAt(i);
			if(row.getId() == module.getId()) return row;
		}
		return null;
	}
	
	public void deleteModuleRow(Module module) {
		ModuleRow row = getModuleRow(module);
		if(row instanceof View) removeView((View)row);
	}
	
	public DeviceRow addDeviceRow(Module module) {
		DeviceRow row = new DeviceRow(super.getContext(), module);
		row.setLayoutParams(new TableLayout.LayoutParams(
            android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
            android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
		this.addView(row);
		return row;
	}
	
	public DeviceRow getDeviceRow(Module module) {
		ModuleRow row = getModuleRow(module);
		if(row instanceof DeviceRow) return (DeviceRow)row;
		return null;
	}
}