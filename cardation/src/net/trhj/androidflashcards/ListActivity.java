package net.trhj.androidflashcards;

import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.ListView;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView;
import android.content.Intent;
import java.util.*;
import android.util.Log;

public class ListActivity extends Activity {

    static ListView listview_ = null;
    static boolean needs_refresh_ = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);

        listview_ = (ListView) findViewById(R.id.full_list);
        populateListView();
        listview_.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent,
                                    View view, int position, long id) {
                String item = ((TextView)view).getText().toString();
                EditActivity.curr_recto = item;
                Intent intent = new Intent(getBaseContext(),
                                           EditActivity.class);
                startActivity(intent);
            }
        });
	}

    void populateListView() {
        Log.w("Cardation", "In populateListView()");
        final LinkedList<String> list = CardDb.listAllRecto(this);
        final ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
            android.R.layout.simple_list_item_1, list);
        listview_.setAdapter(adapter);
    }

    protected void onStart() {
        super.onStart();
        if (needs_refresh_) {
            populateListView();
            needs_refresh_ = false;
        }
        Log.w("Cardation", "Starting ListActivity");
    }
}
