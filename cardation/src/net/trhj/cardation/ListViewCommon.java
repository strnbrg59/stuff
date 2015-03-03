package net.trhj.cardation;

import android.app.Activity;
import android.content.Intent;
import android.text.*;
import android.view.View;
import android.widget.EditText;
import android.widget.Filter;
import android.widget.ListView;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView;
import java.util.*;
import android.util.Log;

public class ListViewCommon {

    public static void doit(ListView listview, final EditText searchbox,
                            final Activity activity, Intent intent) {
        LinkedList<String> list = CardDb.listAllRecto(activity);
        final MyArrayAdapter adapter = new MyArrayAdapter(activity,
            android.R.layout.simple_spinner_item, list);
        listview.setAdapter(adapter);
        
        // If we're coming here after an excursion into EditActivity, there'll
        // be a prescribed position to go to.
        int position = intent.getIntExtra("position", 0);
        listview.smoothScrollToPosition(position); // (Doesn't work!)
        listview.setSelection(position);

        listview.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent,
                                    View view, int position, long id) {
                String item = ((TextView)view).getText().toString();
                EditActivity.curr_recto = item;
                Intent intent = new Intent(activity,
                                           EditActivity.class);
                // Save the position; Edit will pass this on when it sends us
                // back to List.
                intent.putExtra("position", position);
                activity.startActivity(intent);
            }
        });

        searchbox.addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable arg0) {
                String text = searchbox.getText().toString();
                adapter.getFilter().filter(text,
                    new Filter.FilterListener() {
                        @Override
                        public void onFilterComplete(int count) {
							adapter.clear();
							adapter.addAll(adapter.mutable_objects_);
                            Log.i("Cardation", "Filter returned " + count
                              + " values ");
							Log.i("Cardation", "onFilterComplete(), " +
								"adapter.getCount() = " + adapter.getCount());
                        }
                    });
            }
            @Override
            public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                                          int arg3) {}
            @Override
            public void onTextChanged(CharSequence arg0, int arg1, int arg2,
                                      int arg3) {}
        });
    }
}
