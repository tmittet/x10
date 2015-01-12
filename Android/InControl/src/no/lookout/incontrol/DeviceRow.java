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
import android.content.res.Resources;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.ToggleButton;

public class DeviceRow  extends TableRow implements ModuleRow {
	
	private OnChangeListener onChangeListener = null;
	
	private Module.Type moduleType;
	private Module.State moduleState;
	private ToggleButton toggleButton;
	private TextView textView;
	private SeekBar seekBar;
	
	@Override
	public void setOnChangeListener(OnChangeListener listener) {
		onChangeListener = listener;
	}
	
	@Override
	public void setAllProperties(Module module)
	{
		setId(module.getId());
		setModuleType(module.getType());
		setModuleState(module.getState());
		setName(module.getName());
		setBrightness(module.getBrightness());
	}
	
	@Override
	public void setModuleType(Module.Type type)
	{
		moduleType = type;
		toggleButton.setVisibility(
			moduleType == Module.Type.Appliance ||
			moduleType == Module.Type.Dimmer ?
	    	View.VISIBLE : View.INVISIBLE);
		seekBar.setVisibility(
			moduleType == Module.Type.Dimmer ?
	    	View.VISIBLE : View.INVISIBLE);
	}
	@Override
	public Module.Type getModuleType()
	{
		return moduleType;
	}
	
	@Override
	public void setModuleState(Module.State state)
	{
		moduleState = state;
		if(moduleState != Module.State.Unknown) {
			if(toggleButton.getVisibility() != View.VISIBLE) {
				toggleButton.setVisibility(View.VISIBLE);
			}
			toggleButton.setChecked(moduleState == Module.State.On);
		}
	}
	@Override
	public Module.State getModuleState()
	{
		return moduleState;
	}

	public void setOn(boolean on) {
		setModuleState(on ? Module.State.On : Module.State.Off);
	}
	public boolean getOn() {
		return toggleButton.isChecked();
	}
	
	@Override
	public void setName(CharSequence name) {
		textView.setText(name);
	}
	@Override
	public CharSequence getName() {
		return textView.getText();
	}

	public void setBrightness(byte brightness) {
		seekBar.setProgress((byte)(brightness > 100 ? 99 : brightness > 0 ? brightness - 1 : 0));
	}
	public byte getBrightness() {
		return (byte)(seekBar.getProgress() + 1);
	}
	
	public DeviceRow(Context context, Module module) {
		super(context);
		moduleType = Module.Type.Unknown;
		moduleState = Module.State.Unknown;
		Resources resources = context.getResources();
  		toggleButton = new ToggleButton(context);
  		toggleButton.setLayoutParams(new TableRow.LayoutParams(
			android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
			android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
  		toggleButton.setLayoutParams(new TableRow.LayoutParams(60, 50));
  		toggleButton.setTextOn(resources.getString(R.string.on));
  		toggleButton.setTextOff(resources.getString(R.string.off));
  		toggleButton.setOnClickListener(onClickListener);
  		addView(toggleButton);
  		TableLayout table = new TableLayout(context);
  		table.setPadding(5, 0, 0, 0);
  		table.setLayoutParams(new TableRow.LayoutParams(
			android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
			android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
  		table.setLayoutParams(new TableRow.LayoutParams(240, 50));
    	textView = new TextView(context);
    	textView.setLayoutParams(new LinearLayout.LayoutParams(
            android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
            android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
  		table.addView(textView);
    	seekBar = new SeekBar(context);
		seekBar.setLayoutParams(new TableLayout.LayoutParams(
            android.view.ViewGroup.LayoutParams.FILL_PARENT,
            android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
		seekBar.setMax(99);
		seekBar.setOnSeekBarChangeListener(onSeekBarChangeListener);
		table.addView(seekBar);
		addView(table);
		
		setAllProperties(module);
	}
	
	private View.OnClickListener onClickListener = new View.OnClickListener() {
		
		@Override
		public void onClick(View view) {
			if(onChangeListener != null) {
				ToggleButton button = (ToggleButton)view;
				Module.State state = button.isChecked() ? Module.State.On : Module.State.Off;
				DeviceRow.this.setModuleState(state);
				onChangeListener.onStateChanged(DeviceRow.this, state);
			}
		}
	};
	
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener()
    {
    	@Override
	    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) { }
	    @Override
	    public void onStartTrackingTouch(SeekBar seekBar) { }
	    @Override
	    public void onStopTrackingTouch(SeekBar seekBar) {
	    	if(onChangeListener != null) {
	    		byte brightness = (byte)(seekBar.getProgress() + 1);
	    		DeviceRow.this.setBrightness(brightness);
				onChangeListener.onBrightnessChanged(DeviceRow.this, brightness);
			}
	    }
    };
}
