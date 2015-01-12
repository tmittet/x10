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
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.http.client.ClientProtocolException;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.AsyncTask;

public class ModuleHost {

	public interface OnGetListener {
		public abstract void onRequestStart();
		public abstract void onRequestComplete();
		public abstract void onRequestError(Exception e);
		public abstract void onAdd(Module module);
		public abstract void onChange(Module module);
		public abstract void onDelete(Module module);
	}
	
	private HttpParams httpParams;
	private String uri = "";
	private int port = 80;
	private String userName;
	private String password;
	private Comparator<Module> orderModulesBy;
	
	private OnGetListener onGetListener = null;
	
	private Map<Integer, Module> modules = new HashMap<Integer, Module>();
	
	public int getConnectionTimeoutMs() {
		return HttpConnectionParams.getConnectionTimeout(httpParams);
	}
    public void setConnectionTimeoutMs(int connectionTimeoutMs) {
    	HttpConnectionParams.setConnectionTimeout(httpParams, connectionTimeoutMs);
	}
	public int getResponseTimeoutMs() {
		return HttpConnectionParams.getSoTimeout(httpParams);
	}
	public void setResponseTimeoutMs(int responseTimeoutMs) {
		HttpConnectionParams.setSoTimeout(httpParams, responseTimeoutMs);
	}
	public String getDomainOrIP() {
		return ModuleUtil.getDomainFromUri(uri);
	}
	public void setDomainOrIP(String domain) {
		uri = "http://" + ModuleUtil.getDomainFromUri(domain.replace(" ", "")) + ":" + port;
	}
	public int getPortNumber() {
		return port;
	}
	public void setPortNumber(int port) {
		this.port = port;
		uri = "http://" + ModuleUtil.getDomainFromUri(uri);
		if(port != 80) uri += ":" + port;
	}
	public String getUserName() {
		return userName;
	}
	public void setUserName(String userName) {
		this.userName = userName;
	}
	public String getPassword() {
		return password;
	}
	public void setPassword(String password) {
		this.password = password;
	}
	
	public void setOrderModulesBy(Comparator<Module> orderBy) {
		this.orderModulesBy = orderBy;
	}
	public Comparator<Module> getOrderModulesBy() {
		return orderModulesBy;
	}
	
	public void setOnModuleGetListener(OnGetListener listener) {
		onGetListener = listener;
	}

	public ModuleHost(String domainOrIP, int portNumber, String userName, String password, int connectionTimeoutMs, int responseTimeoutMs, Comparator<Module> orderModulesBy) {
		httpParams = new BasicHttpParams();
		setDomainOrIP(domainOrIP);
		setPortNumber(portNumber);
		setUserName(userName);
		setPassword(password);
		setConnectionTimeoutMs(connectionTimeoutMs);
        setResponseTimeoutMs(responseTimeoutMs);
        setOrderModulesBy(orderModulesBy);
	}
	
	public void refresh() {
		new GetModulesDataTask().execute();
	}
	
	public Collection<Module> getModules() {
		if(modules.isEmpty()) {
			refresh();
		}
		List<Module> sorted = new ArrayList<Module>(modules.values());
		Collections.sort(sorted, orderModulesBy);
		return sorted;
	}
	
	public Module getModuleById(int id) {
		return modules.get(id);
	}
	
	public void addModule(Module module) {
		if(!modules.containsKey(module.getId())) {
			module.setHttpParams(httpParams);
			module.setBaseUrl(uri);
			module.setUserName(userName);
			module.setPassword(password);
			modules.put(module.getId(), module);
			onAdd(module);
		}
		else if(modules.get(module.getId()).update(module)) {
			onChange(modules.get(module.getId()));
		}
	}
	
	public void deleteModule(Module module) {
		if(modules.containsKey(module.getId())) {
			onDelete(modules.get(module.getId()));
			modules.remove(module.getId());
		}
		new DeleteModulesDataTask().execute(module);
	}

	private void onRequestStart() {
		if(onGetListener != null) onGetListener.onRequestStart();
	}
	
	private void onRequestComplete() {
		if(onGetListener != null) onGetListener.onRequestComplete();
	}
	
	private void onRequestError(Exception e) {
		if(onGetListener != null) onGetListener.onRequestError(e);
	}
	
	private void onAdd(Module module) {
		if(onGetListener != null) onGetListener.onAdd(module);
	}
	
	private void onChange(Module module) {
		if(onGetListener != null) onGetListener.onChange(module);
	}
	
	private void onDelete(Module module) {
		if(onGetListener != null) onGetListener.onDelete(module);
	}

	private class GetModulesDataTask extends AsyncTask<Void, Exception, List<Module>> {
    	@Override
    	protected List<Module> doInBackground(Void... params) {
			try {
				JSONObject jsonData = ModuleUtil.getModuleData(httpParams, uri, userName, password);
	    		JSONArray jsonModules;
	    		jsonModules = jsonData.getJSONArray("module");
	    		List<Module> result = new ArrayList<Module>(jsonModules.length());
	    		for(int i = 0; i < jsonModules.length(); i++) {
	    			result.add(new Module(
    					jsonModules.getJSONObject(i), httpParams, uri, userName, password)); 
	            }
	    		Collections.sort(result, orderModulesBy);
	    		return result;
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
    	protected void onPostExecute(List<Module> result) {
    		if(result != null) {
    			List<Integer> deleteKeys = new ArrayList<Integer>();
    			for(int key : modules.keySet())
	            {
	            	if(!result.contains(modules.get(key))) {
	            		onDelete(modules.get(key));
	            		deleteKeys.add(key);
	            	}
	            }
    			for(int key : deleteKeys) {
    				modules.remove(key);
    			}
	            for (Module module : result) {
	            	int key = module.getId();
	            	if(modules.containsKey(key)) {
	            		if(modules.get(key).update(module)) {
	            			onChange(modules.get(key));
	            		}
	            	}
	            	else {
	            		modules.put(key, module);
	            		onAdd(modules.get(key));
	            	}
	            }
	            onRequestComplete();
    		}
        }
    }
	
	private class DeleteModulesDataTask extends AsyncTask<Module, Exception, Boolean> {
    	@Override
    	protected Boolean doInBackground(Module... params) {
			try {
				ModuleUtil.deleteModuleData(httpParams, uri + params[0].getUrl(), userName, password);
				return true;
			} catch (ClientProtocolException e) {
				publishProgress(e);
			} catch (IOException e) {
				publishProgress(e);
			}
    		return false;
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
    	protected void onPostExecute(Boolean result) {
    		if(result) onRequestComplete();
        }
    }
}