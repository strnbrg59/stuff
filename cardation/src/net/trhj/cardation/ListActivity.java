package net.trhj.cardation;

import android.os.Bundle;
import android.app.Activity;
import android.text.*;
import android.view.View;
import android.widget.EditText;
import android.widget.Filter;
import android.widget.ListView;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView;
import android.content.Intent;
import java.util.*;
import android.util.Log;

public class ListActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);
        getActionBar().setTitle("Search " + CardDb.getCurrLanguage());

        ListView listview = (ListView) findViewById(R.id.full_list);
        EditText searchbox = (EditText) findViewById(R.id.search);
        ListViewCommon.doit(listview, searchbox, this, getIntent());
	}
}
