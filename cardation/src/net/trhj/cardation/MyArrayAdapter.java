package net.trhj.cardation;

import android.content.Context;
import android.util.Log;
import android.widget.Filter;
import android.widget.ArrayAdapter;
import java.util.*;

class MyArrayAdapter extends ArrayAdapter<String> {

    public List<String> mutable_objects_;
    public ArrayList<String> orig_objects_;
	int constraint_prev_length_;

    public MyArrayAdapter(Context context, int resource, List<String> objects) {
        super(context, resource, objects);
        mutable_objects_ = objects;
        orig_objects_ = new ArrayList<String>(objects);
		constraint_prev_length_ = 0;
    }

    @Override
    public Filter getFilter() {

        Filter filter = new Filter() {
            @Override
            protected FilterResults performFiltering(CharSequence constraint) {

				// This lets the ListView recover from backspacing in the search
				// box:
				if (constraint.length() < constraint_prev_length_) {
					Log.i("Cardation", "erased a char, mutable.len=" +
						mutable_objects_.size() + ", orig.len=" +
						orig_objects_.size());
					mutable_objects_.clear();
					mutable_objects_.addAll(orig_objects_);
				}
				constraint_prev_length_ = constraint.length();

                FilterResults filterResults = new FilterResults();   
                ArrayList<String> tempList=new ArrayList<String>();

                if(constraint != null && mutable_objects_!=null) {
                    int length=mutable_objects_.size();
                    int i=0;
                    while(i<length){
                        String item=mutable_objects_.get(i);
                        if (item.contains(constraint)) {
                            tempList.add(item);
                        }
                        i++;
                    }

                    filterResults.values = tempList;
                    filterResults.count = tempList.size();
                }
                return filterResults;
            }

            @SuppressWarnings("unchecked")
            @Override
            protected void publishResults(CharSequence contraint,
                                          FilterResults results) {
                mutable_objects_ = (ArrayList<String>) results.values;
				Log.i("Cardation", "publishResults(), results.count="
					+ results.count);
                if (results.count > 0) {
                    notifyDataSetChanged();
                } else {
                    notifyDataSetInvalidated();
                }  
            }
        };

        return filter;
    }
}
