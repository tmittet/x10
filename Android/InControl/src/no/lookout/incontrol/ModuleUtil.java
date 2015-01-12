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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.List;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.HttpParams;
import org.json.JSONException;
import org.json.JSONObject;

public class ModuleUtil {
	
	public static JSONObject getModuleData(HttpParams httpParams, String uri, String userName, String password) throws ClientProtocolException, IOException, JSONException {
        HttpClient httpClient = new DefaultHttpClient(httpParams);
        HttpGet httpGet = new HttpGet(uri);
        if(userName != null && userName.length() > 0 && password != null && password.length() > 0) {
        	httpGet.addHeader("Authorization", "Basic " + Base64.encodeBytes((userName + ":" + password).getBytes()));
        }
        HttpResponse response;
            response = httpClient.execute(httpGet);
            HttpEntity entity = response.getEntity(); 
            if(entity != null) {
                InputStream instream = entity.getContent();
                JSONObject jsonObject = new JSONObject(streamToString(instream));
                instream.close();
                return jsonObject;
            }
        return null;
    }
    
    public static JSONObject postModuleData(HttpParams httpParams, String uri, String userName, String password, List<BasicNameValuePair> fields) throws ClientProtocolException, IOException, JSONException {
    	
        DefaultHttpClient httpClient = new DefaultHttpClient(httpParams);
        httpClient.removeRequestInterceptorByClass(org.apache.http.protocol.RequestExpectContinue.class);
        HttpPost httpPost = new HttpPost(uri);
        if(userName != null && userName.length() > 0 && password != null && password.length() > 0) {
        	httpPost.addHeader("Authorization", "Basic " + Base64.encodeBytes((userName + ":" + password).getBytes()));
        }
		try {
			httpPost.setEntity(new UrlEncodedFormEntity(fields));
			HttpResponse response = httpClient.execute(httpPost);
	        HttpEntity entity = response.getEntity(); 
	        if (entity != null) {
	            InputStream instream = entity.getContent();
	            JSONObject jsonObject = new JSONObject(streamToString(instream));
	            instream.close();
	            return jsonObject;
	        }
		}
		catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		return null;
    }
    
    public static void deleteModuleData(HttpParams httpParams, String uri, String userName, String password) throws ClientProtocolException, IOException {
    	DefaultHttpClient httpClient = new DefaultHttpClient(httpParams);
        HttpDelete httpDelete = new HttpDelete(uri);
        if(userName != null && userName.length() > 0 && password != null && password.length() > 0) {
        	httpDelete.addHeader("Authorization", "Basic " + Base64.encodeBytes((userName + ":" + password).getBytes()));
        }
		httpClient.execute(httpDelete);
    }
    
    public static String getDomainFromUri(String domain) {
		if(domain.indexOf("//") > -1) domain = domain.substring(domain.indexOf("//") + 2);
		if(domain.endsWith("/")) domain = domain.substring(0, domain.indexOf("/"));
		if(domain.contains(":")) domain = domain.substring(0, domain.indexOf(":"));
		return domain;
	}
    
    private static String streamToString(InputStream stream) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
        StringBuilder sb = new StringBuilder();
        try {
        	String line = null;
            while((line = reader.readLine()) != null) sb.append(line + "\n");
        }
        finally {
        	stream.close();
        }
        return sb.toString();
    }
}