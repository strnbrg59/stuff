package net.trhj.androidflashcards;

import android.app.Activity;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.util.Log;
import android.view.View;
import java.util.*;

public class LanguageSpinner
{
    Spinner spinner_;
    ArrayAdapter<String> adapter_;
    Activity activity_;
    LinkedList<String> languages_;

    public LanguageSpinner(Activity activity)
    {
        init(activity);
    }

    @SuppressWarnings("unchecked")
    void init(Activity activity)
    {
        activity_ = activity;
        spinner_ = (Spinner) activity.findViewById(R.id.deck_spinner);
        adapter_ = new ArrayAdapter<String>(activity, //getBaseContext(),
                                           android.R.layout.simple_list_item_1);

        languages_ = CardDb.listLanguages(activity);

        LinkedList<String> languages_copy =
            (LinkedList<String>)languages_.clone();
        while (languages_copy.size() > 0) {
            adapter_.add(languages_copy.poll());
        }
        adapter_.add(Globals.new_language);

        spinner_.setAdapter(adapter_);
        adapter_.sort(new LanguageComparator());
        spinner_.setOnItemSelectedListener(new SpinnerActivity());

    }

    public void remove(String lang)
    {
        // If you leave the current language as is, then when init() calls
        // CardDb.listLanguages(), that thing will call getReadableDatabase()
        // which will call createTableIfNotExists().
        CardDb.setCurrLanguage(null, activity_);

        // Should work, because we've dropped the corresponding table in the DB.
        init(activity_);
    }

    final class SpinnerActivity extends Activity
        implements OnItemSelectedListener
    {
        public void onItemSelected(AdapterView<?> parent, View view, 
            int pos, long id)
        {
            String choice = (String) parent.getItemAtPosition(pos);
            if (choice.equals(Globals.new_language)) {
                EditTextPopup dialog =
                    new EditTextPopup(activity_, Globals.new_language,
                                      languages_,
                                      new SpinnerFunctor());
                dialog.show();
            } else {
                CardDb.setCurrLanguage(choice, getBaseContext());
            }
        }

        public void onNothingSelected(AdapterView<?> parent)
        {
            Log.e("Cardation", "nothing selected");
        }
    }

    class LanguageComparator implements Comparator<String> {
        public int compare(String s1, String s2) {
            if (s1.equals(Globals.new_language) && s2.equals(Globals.new_language)) {
                return 0;  // To avoid disasters, just in case.
            } else if (s1.equals(Globals.new_language)) {
                return 1;
            } else if (s2.equals(Globals.new_language)) {
                return -1;
            } else {
                return s1.compareTo(s2);
            }
        }
    }

    class SpinnerFunctor implements PopupFunctor {
        public void doit(Activity activity, Object extra) {
            String new_lang = (String) extra;
            Log.w("Cardation", "SzpyjnrFanktr.doit(), newlang = " +
                  new_lang);
            adapter_.add(new_lang);
            adapter_.sort(new LanguageComparator());
            CardDb.setCurrLanguage(new_lang, activity);
            spinner_.setSelection(adapter_.getPosition(new_lang));
        }
    }
}
