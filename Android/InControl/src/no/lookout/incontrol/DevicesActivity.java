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

import java.util.Comparator;

import no.lookout.incontrol.Module.State;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ScrollView;

public class DevicesActivity extends Activity {

	private static final String LOG_TAG = "DevicesActivity";
	private static final int EDIT_MODULE_REQUEST_CODE = 1;

	private boolean blockModuleHostRefresh = false;
	private SharedPreferences preferences;
	private Resources resources;
	private ModuleTable moduleTable;
	private ProgressDialog waitDialog;
	private ModuleHost moduleHost;
	
    @Override
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        preferences = PreferenceManager.getDefaultSharedPreferences(this);
        preferences.registerOnSharedPreferenceChangeListener(onPreferenceChangeListener);
        resources = getResources();
        moduleTable = new ModuleTable(this);
        moduleTable.setLayoutParams(new LinearLayout.LayoutParams(
            android.view.ViewGroup.LayoutParams.FILL_PARENT,
            android.view.ViewGroup.LayoutParams.FILL_PARENT));
        ScrollView scroll = new ScrollView(this);
        scroll.setLayoutParams(new LinearLayout.LayoutParams(
            android.view.ViewGroup.LayoutParams.FILL_PARENT,
            android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
        scroll.addView(moduleTable);
        scroll.setVerticalScrollBarEnabled(false);
        setContentView(scroll);
        moduleHost = new ModuleHost(
    		preferences.getString("domainOrIP", ""),
    		getIntegerPref("portNumber", 80),
    		preferences.getString("userName", ""),
    		preferences.getString("password", ""),
    		getIntegerPref("connectionTimeout", 6000),
    		getIntegerPref("responseTimeout", 6000),
    		getModuleComparatorFromPref());
        moduleHost.setOnModuleGetListener(onModuleGetListener);
    }
    
	@Override
	public void onResume() {
		super.onResume();
		if(moduleHost.getDomainOrIP().length() == 0)
		{
			SharedPreferences.Editor prefEditor = preferences.edit();
			prefEditor.putString("domainOrIP", "www");
			prefEditor.commit();
			startActivity(new Intent(this, SettingsActivity.class));
		}
		else
		{
			if(!blockModuleHostRefresh) moduleHost.refresh();
		}		
	}
	
	@Override
	public void onDestroy() {
		preferences.unregisterOnSharedPreferenceChangeListener(onPreferenceChangeListener);
		preferences = null;
		resources = null;
		moduleTable = null;
		waitDialog = null;
		moduleHost = null;
		super.onDestroy();
	}

	private SharedPreferences.OnSharedPreferenceChangeListener onPreferenceChangeListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
		@Override
		public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
			if(key.equals("domainOrIP")) {
				moduleHost.setDomainOrIP(preferences.getString("domainOrIP", "www"));
			}
			else if(key.equals("portNumber")) {
		        moduleHost.setPortNumber(getIntegerPref("portNumber", 80));
			}
			else if(key.equals("connectionTimeout")) {
		        moduleHost.setConnectionTimeoutMs(getIntegerPref("connectionTimeout", 6000));
			}
			else if(key.equals("responseTimeout")) {
		        moduleHost.setResponseTimeoutMs(getIntegerPref("responseTimeout", 6000));
			}
			else if(key.equals("userName")) {
				moduleHost.setUserName(preferences.getString("userName", ""));
			}
			else if(key.equals("password")) {
				moduleHost.setPassword(preferences.getString("password", ""));
			}
			else if(key.equals("deviceOrder")) {
				moduleHost.setOrderModulesBy(getModuleComparatorFromPref());
				moduleTable.removeAllViews();
				for(Module module : moduleHost.getModules()) {
					moduleTable.addDeviceRow(module);
				}
			}
		}
	};
	
	private ModuleHost.OnGetListener onModuleGetListener = new ModuleHost.OnGetListener() {
		@Override
		public void onRequestStart() {
			waitDialog = new ProgressDialog(DevicesActivity.this);
    		waitDialog.setCancelable(false);
    		waitDialog.setIndeterminate(true);
    		waitDialog.setMessage(resources.getString(R.string.refreshing_wait));
    		waitDialog.show();			
		}
		@Override
		public void onRequestComplete() {
			if(waitDialog != null) waitDialog.dismiss();
		}
		@Override
		public void onRequestError(Exception e) {
			if(waitDialog != null) waitDialog.dismiss();
			Log.w(LOG_TAG, e.getMessage());
			showAlert(
				DevicesActivity.this,
				resources.getString(R.string.request_failed),
				e.getMessage());			
		}
		@Override
		public void onAdd(Module module) {
			DeviceRow row = moduleTable.addDeviceRow(module);
			row.setOnChangeListener(onDeviceRowChangeListener);
			registerForContextMenu(row);
			module.setOnModulePostListener(onModulePostListener);
        	module.setOnModuleChangeListener(onModuleChangeListener);			
		}
		@Override
		public void onChange(Module module) { }
		@Override
		public void onDelete(Module module) {
			moduleTable.deleteModuleRow(module);
		}
	};
	
	private Module.OnPostListener onModulePostListener = new Module.OnPostListener() {
		@Override
		public void onRequestStart(Module module) {
			blockModuleHostRefresh = true;
			waitDialog = new ProgressDialog(DevicesActivity.this);
    		waitDialog.setCancelable(false);
    		waitDialog.setIndeterminate(true);
    		waitDialog.setMessage(resources.getString(R.string.executing_wait));
    		waitDialog.show();
		}
		@Override
		public void onRequestComplete(Module module) {
    		if(waitDialog != null) waitDialog.dismiss();
    		blockModuleHostRefresh = false;
		}
		@Override
		public void onRequestError(Module module, Exception e) {
			if(waitDialog != null) waitDialog.dismiss();
			Log.w(LOG_TAG, e.getMessage());
			showAlert(
				DevicesActivity.this,
				resources.getString(R.string.request_failed),
				e.getMessage());
			blockModuleHostRefresh = false;
		}
	};
    
    private Module.OnChangeListener onModuleChangeListener = new Module.OnChangeListener() {
    	@Override
    	public void onTypeChanged(Module module, Module.Type type)
    	{
    		ModuleRow row = moduleTable.getModuleRow(module);
    		if(row != null) row.setModuleType(type);
    	}
    	@Override
    	public void onNameChanged(Module module, String name) {
    		ModuleRow row = moduleTable.getModuleRow(module);
    		if(row != null) row.setName(name);
    	}
    	@Override
    	public void onStateChanged(Module module, Module.State state) {
    		ModuleRow row = moduleTable.getModuleRow(module);
    		if(row != null) row.setModuleState(state);
    	}
    	@Override
    	public void onBrightnessChanged(Module module, byte brightness) {
    		DeviceRow row = moduleTable.getDeviceRow(module);
    		if(row != null) row.setBrightness(brightness);
    	}
    };
    
    private DeviceRow.OnChangeListener onDeviceRowChangeListener = new DeviceRow.OnChangeListener() {
		
    	@Override
		public void onStateChanged(DeviceRow deviceRow, State state) {
			Module module = moduleHost.getModuleById(deviceRow.getId());
			if(module != null) {
				module.setState(state);
				module.save();
			}
		}
		
		@Override
		public void onBrightnessChanged(DeviceRow deviceRow, byte brightness) {
			Module module = moduleHost.getModuleById(deviceRow.getId());
			if(module != null) {
				module.setBrightness(brightness);
				module.save();
			}
		}
	};
    
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.devices_menu, menu);
	    return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	    case R.id.settings:
	    	startActivity(new Intent(this, SettingsActivity.class));
	        return true;
	    case R.id.add_device:
	    	startEditModuleActivity(null);
	    	return true;
	    case R.id.quit:
	    	finish();
	        return true;
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}
	
	@Override
	public void onCreateContextMenu(ContextMenu menu, View view, ContextMenu.ContextMenuInfo menuInfo)
	{
		super.onCreateContextMenu(menu, view, menuInfo);
		Module module = moduleHost.getModuleById(view.getId());
		if(module != null) {
			menu.setHeaderTitle(module.getName());
			menu.add(0, view.getId(), 0, R.string.edit);
			menu.add(0, view.getId(), 0, R.string.delete);
		}
		else {
			menu.setHeaderTitle(R.string.tab_devices);
		}
		menu.add(0, view.getId(), 0, R.string.refresh);
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
		CharSequence title = item.getTitle();
		if(title.equals(resources.getString(R.string.refresh))) {
			if(!blockModuleHostRefresh) moduleHost.refresh();
			return true;
		}
		else if(title.equals(resources.getString(R.string.edit))) {
			Module module = moduleHost.getModuleById(item.getItemId());
			if(module != null) startEditModuleActivity(module);
			return true;
		}
		else if(title.equals(resources.getString(R.string.delete))) {
			Module module = moduleHost.getModuleById(item.getItemId());
			if(module != null) {
				DeviceDeleteConfirm(module);
			}
			return true;
		}
	    else {
	    	return super.onContextItemSelected(item);
	    }
	}
	
	@Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        if(resultCode == RESULT_OK) {
	        Bundle extras = intent.getExtras();
	        if(extras != null && extras.containsKey("edit"))
	        {
	        	boolean edit = extras.getBoolean("edit");
	        	Module newModule = new Module(extras.getChar("house"), extras.getByte("unit"));
	        	Module curModule = moduleHost.getModuleById(newModule.getId());
	        	// Create
	        	if(curModule == null) {
	        		newModule.setType(extras.getByte("type"));
	        		newModule.setName(extras.getString("name"));
	        		moduleHost.addModule(newModule);
	        		newModule.save();
	        	}
	        	else {
	        		String curName = curModule.getName();
	        		if(extras.containsKey("type") && extras.getByte("type") != (byte)curModule.getType().ordinal()) {
	        			curModule.setType(extras.getByte("type"));
		        	}
		        	if(extras.containsKey("name") && !extras.getString("name").equals(curName)) {
		        		curModule.setName(extras.getString("name"));
		        	}
	        		// Overwrite
	        		if(!edit) {
	        			blockModuleHostRefresh = true;
	        			ModuleOverwriteConfirm(curModule, curName);
	        		}
	        		// Edit
	        		else {
	        			curModule.save();
	        		}
	        	}
	        }
        }
        super.onActivityResult(requestCode, resultCode, intent);
    }
	
	private int getIntegerPref(String key, int defValue)
	{
        try {
        	return Integer.parseInt(preferences.getString(key, Integer.toString(defValue)));
        }
        catch(NumberFormatException e) { }
        return defValue;
	}
	
	private Comparator<Module> getModuleComparatorFromPref() {
		int orderBy = getIntegerPref("deviceOrder", 2);
		switch(orderBy) {
		case 0: return Module.NAME;
		case 1: return Module.HOUSE_NAME;
		case 3: return Module.HOUSE_TYPE_NAME;
		case 4: return Module.HOUSE_TYPE_UNIT;
		case 5: return Module.HOUSE_TYPE_DESC_NAME;
		case 6: return Module.HOUSE_TYPE_DESC_UNIT;
		case 7: return Module.TYPE_NAME;
		case 8: return Module.TYPE_HOUSE_NAME;
		case 9: return Module.TYPE_HOUSE_UNIT;
		case 10: return Module.TYPE_DESC_HOUSE_UNIT;
		case 11: return Module.TYPE_DESC_HOUSE_UNIT;
		case 12: return Module.TYPE_DESC_HOUSE_UNIT;
		default: return Module.HOUSE_UNIT;
		}
	}
    
    private void startEditModuleActivity(Module module) {
		Intent intent = new Intent(this, EditModuleActivity.class);
		if(module != null) {
			intent.putExtra("edit", true);
			intent.putExtra("id", module.getId());
			intent.putExtra("house", module.getHouse());
			intent.putExtra("unit", module.getUnit());
			intent.putExtra("url", module.getUrl());
			intent.putExtra("type", (byte)module.getType().ordinal());
			intent.putExtra("name", module.getName());
		}
		startActivityForResult(intent, EDIT_MODULE_REQUEST_CODE);
	}
    
    private static void showAlert(Activity activity, String title, String text) {
    	new AlertDialog.Builder(activity)
    		.setTitle(title)
    		.setMessage(text)
    		.setPositiveButton(R.string.ok, new OnClickListener() {
    			public void onClick(DialogInterface dialog, int which) { }
    			})
    		.show();
    }
    
    private void ModuleOverwriteConfirm(Module module, String moduleName) {
        
    	final Message okMsg = Message.obtain();
        okMsg.setTarget(moduleOverwriteHandler);
        okMsg.what = RESULT_OK;
        okMsg.obj = module;        
        
        final Message cancelMsg = Message.obtain();
        cancelMsg.setTarget(moduleOverwriteHandler);
        cancelMsg.what = RESULT_CANCELED;
        cancelMsg.obj = module;
        
        new AlertDialog.Builder(this)
        	.setTitle(moduleName)
        	.setMessage(R.string.module_exists_overwrite)
			.setPositiveButton(R.string.yes, new OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					okMsg.sendToTarget();
				}
			})
			.setNegativeButton(R.string.no, new OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					cancelMsg.sendToTarget();
				}
			})
			.setOnCancelListener(new OnCancelListener() {
				public void onCancel(DialogInterface dialog) {
					cancelMsg.sendToTarget();
				}				
			})
			.show();
    }
    
    private Handler moduleOverwriteHandler = new Handler() {
        public void handleMessage(Message msg) {
        	Module module = (Module)msg.obj;
        	if(msg.what == RESULT_OK) {
        		module.save();
        	}
        	else {
        		module.revert();
        	}
        	blockModuleHostRefresh = false;
        }
    };
    
    private void DeviceDeleteConfirm(Module module) {
        
    	final Message okMsg = Message.obtain();
        okMsg.setTarget(deviceDeleteHandler);
        okMsg.what = RESULT_OK;
        okMsg.obj = module;        
        
        final Message cancelMsg = Message.obtain();
        cancelMsg.setTarget(deviceDeleteHandler);
        cancelMsg.what = RESULT_CANCELED;
        cancelMsg.obj = module;
        
        new AlertDialog.Builder(this)
        	.setTitle(getResources().getString(R.string.delete) + " " + module.getName())
        	.setMessage(R.string.delete_device_confirm)
			.setPositiveButton(R.string.yes, new OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					okMsg.sendToTarget();
				}
			})
			.setNegativeButton(R.string.no, new OnClickListener() {
				public void onClick(DialogInterface dialog, int which) { }
			})
			.show();
    }
    
    private Handler deviceDeleteHandler = new Handler() {
        public void handleMessage(Message msg) {
        	if(msg.what == RESULT_OK) {
        		moduleHost.deleteModule((Module)msg.obj);
        	}
        }
    };
}