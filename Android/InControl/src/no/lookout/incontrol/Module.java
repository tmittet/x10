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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

import org.apache.http.client.ClientProtocolException;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.HttpParams;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.AsyncTask;

public class Module {

	public interface OnPostListener {
		public abstract void onRequestStart(Module module);
		public abstract void onRequestComplete(Module module);
		public abstract void onRequestError(Module module, Exception e);
	}
	
	public interface OnChangeListener {
		public abstract void onTypeChanged(Module module, Module.Type type);
		public abstract void onNameChanged(Module module, String name);
		public abstract void onStateChanged(Module module, Module.State state);
		public abstract void onBrightnessChanged(Module module, byte brightness);
	}
	
	public enum State {
		Unknown,
		On,
		Off,
	};
	
	public enum Type {
		Unknown,
		Appliance,
		Dimmer,
		Sensor,
	};
	
	public static final Comparator<Module> NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getName().compareTo(m2.getName());
				if(comp == 0) comp = HOUSE_UNIT.compare(m1, m1);
				return comp;
			}
		};
	
	public static final Comparator<Module> HOUSE_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getHouse() == m2.getHouse() ? 0 : m1.getHouse() > m2.getHouse() ? 1 : -1;
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
	
	public static final Comparator<Module> HOUSE_TYPE =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getHouse() == m2.getHouse() ? 0 : m1.getHouse() > m2.getHouse() ? 1 : -1;
				if(comp == 0) comp = m1.getType().compareTo(m2.getType());
				return comp;
			}
		};
		
	public static final Comparator<Module> HOUSE_TYPE_DESC =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getHouse() == m2.getHouse() ? 0 : m1.getHouse() > m2.getHouse() ? 1 : -1;
				if(comp == 0) comp = m1.getType().compareTo(m2.getType()) * -1;
				return comp;
			}
		};
	
	public static final Comparator<Module> HOUSE_UNIT =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getHouse() == m2.getHouse() ? 0 : m1.getHouse() > m2.getHouse() ? 1 : -1;
				if(comp == 0) comp = m1.getUnit() == m2.getUnit() ? 0 : m1.getUnit() > m2.getUnit() ? 1 : -1;
				return comp;
			}
		};
	
	public static final Comparator<Module> HOUSE_TYPE_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = HOUSE_TYPE.compare(m1, m2);
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
		
	public static final Comparator<Module> HOUSE_TYPE_DESC_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = HOUSE_TYPE_DESC.compare(m1, m2);
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
	
	public static final Comparator<Module> HOUSE_TYPE_UNIT =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = HOUSE_TYPE.compare(m1, m2);
				if(comp == 0) comp = m1.getUnit() == m2.getUnit() ? 0 : m1.getUnit() > m2.getUnit() ? 1 : -1;
				return comp;
			}
		};
		
	public static final Comparator<Module> HOUSE_TYPE_DESC_UNIT =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = HOUSE_TYPE_DESC.compare(m1, m2);
				if(comp == 0) comp = m1.getUnit() == m2.getUnit() ? 0 : m1.getUnit() > m2.getUnit() ? 1 : -1;
				return comp;
			}
		};
	
	public static final Comparator<Module> TYPE_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getType().compareTo(m2.getType());
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
	
	public static final Comparator<Module> TYPE_DESC_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getType().compareTo(m2.getType()) * -1;
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
	
	public static final Comparator<Module> TYPE_HOUSE =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getType().compareTo(m2.getType());
				if(comp == 0) comp = m1.getHouse() == m2.getHouse() ? 0 : m1.getHouse() > m2.getHouse() ? 1 : -1;
				return comp;
			}
		};
		
	public static final Comparator<Module> TYPE_DESC_HOUSE =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = m1.getType().compareTo(m2.getType()) * -1;
				if(comp == 0) comp = m1.getHouse() == m2.getHouse() ? 0 : m1.getHouse() > m2.getHouse() ? 1 : -1;
				return comp;
			}
		};
	
	public static final Comparator<Module> TYPE_HOUSE_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = TYPE_HOUSE.compare(m1, m2);
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
		
	public static final Comparator<Module> TYPE_DESC_HOUSE_NAME =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = TYPE_DESC_HOUSE.compare(m1, m2);
				if(comp == 0) comp = m1.getName().compareTo(m2.getName());
				return comp;
			}
		};
	
	public static final Comparator<Module> TYPE_HOUSE_UNIT =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = TYPE_HOUSE.compare(m1, m2);
				if(comp == 0) comp = m1.getUnit() == m2.getUnit() ? 0 : m1.getUnit() > m2.getUnit() ? 1 : -1;
				return comp;
			}
		};

	public static final Comparator<Module> TYPE_DESC_HOUSE_UNIT =
        new Comparator<Module>() {
			public int compare(Module m1, Module m2) {
				int comp = TYPE_DESC_HOUSE.compare(m1, m2);
				if(comp == 0) comp = m1.getUnit() == m2.getUnit() ? 0 : m1.getUnit() > m2.getUnit() ? 1 : -1;
				return comp;
			}
		};

	private HttpParams httpParams;
	private String baseUrl;
	private String userName;
	private String password;
	
	private char house = '\0';
	private byte unit = 0;
	private String url = null;
	
	private Type persistedType = Type.Unknown;
	private String persistedName = "";
	private State persistedState = State.Unknown;
	private byte persistedBrightness = 0;
	private Type type = Type.Unknown;
	private String name = "";
	private State state = State.Unknown;
	private byte brightness = 0;

	private OnPostListener onPostListener = null;
	private OnChangeListener onChangeListener = null;
	
	protected void setHttpParams(HttpParams httpParams) {
		this.httpParams = httpParams;
	}
	protected HttpParams getHttpParams() {
		return httpParams;
	}
	
	protected void setBaseUrl(String baseUrl) {
		this.baseUrl = baseUrl;
	}
	protected String getBaseUrl() {
		return baseUrl;
	}

	protected void setUserName(String userName) {
		this.userName = userName;
	}
	protected String getUserName() {
		return userName;
	}

	protected void setPassword(String password) {
		this.password = password;
	}
	protected String getPassword() {
		return password;
	}
	
	public int getId()
	{
		return hashCode();
	}
	
	public char getHouse() {
		return house;
	}

	public byte getUnit() {
		return unit;
	}

	public String getUrl() {
		return url;
	}

	public void setType(Type type) {
		this.type = type;
	}
	public void setType(Byte type) {
		for(Type value : Type.values()) {
			if(value.ordinal() == type) this.type = value;
		}
	}
	public Type getType() {
		return type;
	}

	public void setName(String name) {
		this.name = name;
	}
	public String getName() {
		return name;
	}	

	public void setState(State state) {
		if(state != State.Unknown) this.state = state;
	}
	public State getState() {
		return state;
	}	

	public void setBrightness(byte brightness) {
		this.brightness = brightness;
	}
	public byte getBrightness() {
		return brightness;
	}
	
	public void setOnModulePostListener(OnPostListener listener) {
		onPostListener = listener;
	}
	
	public void setOnModuleChangeListener(OnChangeListener listener) {
		onChangeListener = listener;
	}

	public Module(char house, byte unit) throws IllegalArgumentException {
		house = Character.toUpperCase(house);
		if(house < 'A' || house > 'P') throw new IllegalArgumentException("House must be a character between A and P.");
		if(unit < 1 || unit > 16) throw new IllegalArgumentException("Unit must be a value between 1 and 16.");
		url = "/" + house + "/" + unit + "/";
	}
	
	protected Module(JSONObject module, HttpParams httpParams, String baseUrl, String userName, String password) throws JSONException {
		this.httpParams = httpParams;
		this.baseUrl = baseUrl;
		this.userName = userName;
		this.password = password;
		house = module.getString("house").charAt(0);
    	unit = (byte)module.getInt("unit");
    	url = module.getString("url");
    	if(module.has("type"))
    	{
    		switch(module.getInt("type"))
    		{
    		case 1: persistedType = Type.Appliance; break;
    		case 2: persistedType = Type.Dimmer; break;
    		case 3: persistedType = Type.Sensor; break;
    		default: persistedType = Type.Unknown; break;
    		}
    	}
    	persistedName = module.has("name") ? module.getString("name") : Character.toString(house) + unit;
    	if(module.has("on"))
    	{
    		persistedState = module.getBoolean("on") ? State.On : State.Off; 
    	}
    	if(module.has("brightness"))
    	{
    		persistedBrightness = (byte)module.getInt("brightness");
    	}
    	type = persistedType;
    	name = persistedName;
    	state = persistedState;
    	brightness = persistedBrightness;
	}

	public boolean update(JSONObject updatedModule) throws JSONException
	{
		return update(new Module(updatedModule, httpParams, baseUrl, userName, password));
	}
	
	public boolean update(Module updatedModule)
	{
		boolean updated = false;
		Type updType = updatedModule.getType();
		String updName = updatedModule.getName();
		State updState = updatedModule.getState();
		byte updBrightness = updatedModule.getBrightness();
		if(persistedType != updType) {
			updated = true;
			persistedType = updType;
			type = persistedType;
			onTypeChanged();
		}
		if(!persistedName.equals(updName)) {
			updated = true;
			persistedName = updName;
			name = persistedName;
			onNameChanged();
		}
		if(persistedState != updState) {
			updated = true;
			persistedState = updState;
			state = persistedState;
			onStateChanged();
		}
		if(persistedBrightness != updBrightness) {
			updated = true;
			persistedBrightness = updBrightness;
			brightness = persistedBrightness;
			onBrightnessChanged();
		}
		return updated;
	}
	
	public void save()
	{
		if(httpParams == null || baseUrl == null)
		{
			throw new IllegalStateException(
				"Module is not initialized properly and cannot be saved. " +
				"Please make sure module has been added to the ModuleHost.");
		}
		List<BasicNameValuePair> nameValuePairs = new ArrayList<BasicNameValuePair>();
		if(type != persistedType) nameValuePairs.add(new BasicNameValuePair("type", Integer.toString(type.ordinal())));
		if(name != persistedName) nameValuePairs.add(new BasicNameValuePair("name", "\"" + name + "\""));
		if(state != persistedState) nameValuePairs.add(new BasicNameValuePair("on", Integer.toString(state == State.On ? 1 : 0)));
		if(brightness != persistedBrightness) nameValuePairs.add(new BasicNameValuePair("brightness", Integer.toString(brightness)));
		if(nameValuePairs.size() > 0) {
			BasicNameValuePair[] pairs = new BasicNameValuePair[nameValuePairs.size()];
			new PostModuleDataTask().execute(nameValuePairs.toArray(pairs));
		}
	}
	
	public void revert()
	{
		type = persistedType;
		name = persistedName;
		state = persistedState;
		brightness = persistedBrightness;
	}
	
	@Override
	public int hashCode() {
		return url.hashCode();
	}
	
	@Override
	public boolean equals(Object object) {
		if(object instanceof Module)
		{
			Module module = (Module)object;
			return url.equals(module.url);
		}
		return false;
	}
	
	private void onRequestStart() {
		if(onPostListener != null) onPostListener.onRequestStart(this);
	}
	
	private void onRequestComplete() {
		if(onPostListener != null) onPostListener.onRequestComplete(this);
	}
	
	private void onRequestError(Exception e) {
		if(onPostListener != null) onPostListener.onRequestError(this, e);
	}
	
	private void onTypeChanged() {
		if(onChangeListener != null) onChangeListener.onTypeChanged(this, persistedType);
	}
	
	private void onNameChanged() {
		if(onChangeListener != null) onChangeListener.onNameChanged(this, persistedName);
	}
	
	private void onStateChanged() {
		if(onChangeListener != null) onChangeListener.onStateChanged(this, persistedState);
	}
	
	private void onBrightnessChanged() {
		if(onChangeListener != null) onChangeListener.onBrightnessChanged(this, persistedBrightness);
	}

	private class PostModuleDataTask extends AsyncTask<BasicNameValuePair, Exception, JSONObject> {
    	
    	@Override
    	protected JSONObject doInBackground(BasicNameValuePair... params) {
			try {
				return ModuleUtil.postModuleData(httpParams, baseUrl + url, userName, password, Arrays.asList(params));
			} catch (ClientProtocolException e) {
				publishProgress(e);
			} catch (IOException e) {
				publishProgress(e);
			} catch (JSONException e) {
				publishProgress(e);
			}
    		return null;
        }

    	@Override
        protected void onPreExecute() {
    		onRequestStart();
        }        
        
    	@Override
    	protected void onProgressUpdate(Exception... errors) {
    		onRequestError(errors[0]);
    	}
    	
    	@Override
    	protected void onPostExecute(JSONObject result) {
    		if(result != null) {
    			try {
					update(result);
					onRequestComplete();
				}
    			catch (JSONException e) {
    				onRequestError(e);
				}
            }
        }
    }
}