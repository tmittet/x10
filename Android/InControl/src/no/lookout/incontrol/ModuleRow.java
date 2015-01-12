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

public interface ModuleRow {
	
	public interface OnChangeListener {
		public abstract void onStateChanged(DeviceRow deviceRow, Module.State state);
		public abstract void onBrightnessChanged(DeviceRow deviceRow, byte brightness);
	}
	
	public void setOnChangeListener(OnChangeListener listener);
	public void setAllProperties(Module module);
	public int getId();
	public void setModuleType(Module.Type type);
	public Module.Type getModuleType();
	public void setModuleState(Module.State state);
	public Module.State getModuleState();
	public void setName(CharSequence name);
	public CharSequence getName();
}
