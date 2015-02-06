package net.trhj.cardation;

import android.os.Bundle;
import android.app.Activity;
import android.text.*;
import android.view.View;
import android.widget.EditText;
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
    static ArrayAdapter<String> adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);

        listview_ = (ListView) findViewById(R.id.full_list);
        populateListView();
        
        // If we're coming here after an excursion into EditActivity, there'll
        // be a prescribed position to go to.
        Intent intent = getIntent();
        int position = intent.getIntExtra("position", 0);
//      listview_.smoothScrollToPosition(position); (Doesn't work!)
        listview_.setSelection(position);

        listview_.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent,
                                    View view, int position, long id) {
                String item = ((TextView)view).getText().toString();
                EditActivity.curr_recto = item;
                Intent intent = new Intent(getBaseContext(),
                                           EditActivity.class);
                // Save the position; Edit will pass this on when it sends us
                // back to List.
                intent.putExtra("position", position);
                startActivity(intent);
            }
        });

        final EditText search = (EditText) findViewById(R.id.search);
        search.addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable arg0) {
                String text = search.getText().toString();
                adapter.getFilter().filter(text);
                // Matches against beginning of recto text.  To search for
                // recto's that match in the middle too is a big undertaking.
                // On stackoverflow they recommend copying over the source to
                // ArrayAdapter and hacking it.
            }
            @Override
            public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                                          int arg3) {}
            @Override
            public void onTextChanged(CharSequence arg0, int arg1, int arg2,
                                      int arg3) {}
        });
	}

    void populateListView() {
        final LinkedList<String> list = CardDb.listAllRecto(this);
        Log.w("Cardation", "list.size()=" + list.size());
        adapter = new ArrayAdapter<String>(this,
            android.R.layout.simple_spinner_item, list);
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
