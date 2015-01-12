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

import android.app.TabActivity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.os.Bundle;
import android.widget.TabHost;

public class InControl extends TabActivity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        Resources res = getResources();
        TabHost tabHost = getTabHost();
        TabHost.TabSpec spec;
        Intent intent;

        intent = new Intent().setClass(this, DevicesActivity.class);
        spec = tabHost.newTabSpec("devices")
        	.setIndicator("Devices", res.getDrawable(R.drawable.ic_tab_devices))
        	.setContent(intent);
        tabHost.addTab(spec);

        intent = new Intent().setClass(this, ScenariosActivity.class);
        spec = tabHost.newTabSpec("scenarios")
        	.setIndicator("Scenarios", res.getDrawable(R.drawable.ic_tab_scenarios))
            .setContent(intent);
        tabHost.addTab(spec);

        intent = new Intent().setClass(this, SensorsActivity.class);
        spec = tabHost.newTabSpec("songs")
        	.setIndicator("Sensors", res.getDrawable(R.drawable.ic_tab_sensors))
            .setContent(intent);
        tabHost.addTab(spec);

        tabHost.setCurrentTab(0);
    }
}