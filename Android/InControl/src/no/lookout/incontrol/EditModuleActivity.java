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

import android.app.Activity;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

public class EditModuleActivity extends Activity {
    
	private boolean edit = false;
	private Spinner houseSpinner;
	private Spinner unitSpinner;
	private Spinner typeSpinner;
	private EditText nameText;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.edit_module);
        Resources resources = getResources();

        TextView heading = (TextView)findViewById(R.id.heading_label);
        
        houseSpinner = (Spinner)findViewById(R.id.houseCode);
        ArrayAdapter<String> houseAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, resources.getStringArray(R.array.house_codes));
        houseAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        houseSpinner.setAdapter(houseAdapter);
        
        unitSpinner = (Spinner)findViewById(R.id.unitCode);
        ArrayAdapter<String> unitAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, resources.getStringArray(R.array.unit_codes));
        unitAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        unitSpinner.setAdapter(unitAdapter);

        typeSpinner = (Spinner)findViewById(R.id.moduleType);
        ArrayAdapter<String> typeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, resources.getStringArray(R.array.module_types));
        typeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        typeSpinner.setAdapter(typeAdapter);

        nameText = (EditText)findViewById(R.id.name);

        Button saveButton = (Button)findViewById(R.id.save_button);
        saveButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
                Intent intent = new Intent();
                intent.putExtra("edit", edit);
                intent.putExtra("house", (char)(houseSpinner.getSelectedItemPosition() + 65));
                intent.putExtra("unit", (byte)(unitSpinner.getSelectedItemPosition() + 1));
                intent.putExtra("type", (byte)typeSpinner.getSelectedItemPosition());
                intent.putExtra("name", nameText.getText().toString());
                setResult(RESULT_OK, intent);
                finish();
            }});
        
        Button cancelButton = (Button)findViewById(R.id.cancel_button);
        cancelButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
            	setResult(RESULT_CANCELED);
                finish();
            }});
        
        Bundle bundle = getIntent().getExtras();
        if(bundle == null || !bundle.containsKey("edit") || !bundle.getBoolean("edit")) {
        	heading.setText(resources.getText(R.string.add_device));
        	edit = false;
        }
        else {
        	heading.setText(resources.getText(R.string.edit_device));
        	houseSpinner.setEnabled(false);
        	unitSpinner.setEnabled(false);
        	edit = true;
        	if(bundle.containsKey("house")) houseSpinner.setSelection((byte)bundle.getChar("house") - 65);
        	if(bundle.containsKey("unit")) unitSpinner.setSelection(bundle.getByte("unit") - 1);
        	if(bundle.containsKey("type")) typeSpinner.setSelection(bundle.getByte("type"));
        	if(bundle.containsKey("name")) nameText.setText(bundle.getString("name"));
        }
    }
}
