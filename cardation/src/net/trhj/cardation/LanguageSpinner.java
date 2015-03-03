package net.trhj.cardation;

import android.app.Activity;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Toast;
import android.util.Log;
import android.view.View;
import java.util.*;

public class LanguageSpinner
{
    Spinner spinner_;
    ArrayAdapter<LangNumdue> adapter_;
    Activity parent_activity_;
    LinkedList<String> languages_;

    public LanguageSpinner(Activity activity)
    {
        init(activity);
    }

    @SuppressWarnings("unchecked")
    void init(Activity activity)
    {
        parent_activity_ = activity;
        spinner_ = (Spinner) activity.findViewById(R.id.deck_spinner);
        adapter_ = new ArrayAdapter<LangNumdue>(activity,
                        android.R.layout.simple_list_item_1) {
            public int getPosition(LangNumdue item) {
                for (int i=0; i<getCount(); ++i) {
                    if (getItem(i).lang_.equals(item.lang_)) {
                        return i;
                    }
                }
                return -1;
            }};

        languages_ = CardDb.listLanguages(activity);
        LinkedList<String> languages_copy = new LinkedList<String>();
        for (int i=0; i<languages_.size(); ++i) {
            String lang = languages_.get(i);
            if (!ConfigDb.getIsHidden(activity, lang)) {
                languages_copy.add(lang);
            }
        }
        while (languages_copy.size() > 0) {
            String lang = languages_copy.poll();
            int numdue =
                CardDb.getDueCards(activity, Integer.MAX_VALUE, lang).size();
            LangNumdue lang_numdue = new LangNumdue(lang, numdue);
            adapter_.add(lang_numdue);
        }
        adapter_.add(new LangNumdue(Globals.new_language, 0));

        spinner_.setAdapter(adapter_);
        adapter_.sort(new LangNumdueComparator());
        spinner_.setOnItemSelectedListener(new SpinnerActivity());

        String last_lang = ConfigDb.getLastLanguage(activity);
        Log.i("Cardation", "LanguageSpinner.init(), last_lang=|" + last_lang
              + "|");
        if (last_lang.length() > 0) {
            CardDb.setCurrLanguage(last_lang, activity);
            int pos = adapter_.getPosition(new LangNumdue(last_lang,0));
            Log.i("Cardation", "LanguageSpinner adapter_.getPosition(" +
                last_lang + ") = " + pos);
            spinner_.setSelection(pos);
        }
    }

    public void setEnabled(Boolean b)
    {
        spinner_.setEnabled(b);
    }

    // Called from MainActivity.onResume(), so that the information visible
    // stays up to date.
    public void updateNumDue(String lang, int numdue) {
        // Don't think you can edit items in an ArrayAdapter.  So I'll just
        // replace the whole thing, after saving what's there.
        Vector<LangNumdue> new_array = new Vector<LangNumdue>();
        LangNumdue new_item = new LangNumdue(lang, numdue);
        int j = adapter_.getPosition(new_item);
        for (int i=0; i<adapter_.getCount(); ++i) {
            if (i!=j) {
                new_array.addElement(adapter_.getItem(i));
            } else {
                new_array.addElement(new_item);
            }
        }
        adapter_.clear();
        adapter_.addAll(new_array);
    }

    public ArrayAdapter<LangNumdue> getAdapter() {
        return adapter_;
    }

    public void setSelection(int pos) {
        spinner_.setSelection(pos);
    }

    public void remove(String lang)
    {
        // If you leave the current language as is, then when init() calls
        // CardDb.listLanguages(), that thing will call getReadableDatabase()
        // which will call createTableIfNotExists().
        LinkedList<String> langs = CardDb.listLanguages(parent_activity_);
        Log.i("Cardation", "langs.size() = " + langs.size());
        if (langs.size() == 0) {
            // That's if we had only one language, and we deleted it.
            Toast.makeText(parent_activity_, "No languages left exiting now.",
                           Toast.LENGTH_LONG).show();
            System.exit(0);  // Clean enough way to exit.
        }
        CardDb.setCurrLanguage(langs.get(0), parent_activity_);

        // Should work, because we've dropped the corresponding table in the DB.
        init(parent_activity_);
    }

    final class SpinnerActivity extends Activity
        implements OnItemSelectedListener
    {
        public SpinnerActivity() {
            super();
        }

        public void onItemSelected(AdapterView<?> parent, View view, 
            int pos, long id)
        {
            Log.i("Cardation", "SpinnerActivity.onItemSelected()");
            LangNumdue ln = (LangNumdue) parent.getItemAtPosition(pos);

            // The parent activity *is* MainActivity except when we're
            // processing an email attachment, in which case it's the
            // DbbkActivity.  Gotta make sure we don't crash with a bad cast.
            MainActivity main_activity = null;
            try {
                main_activity = (MainActivity)parent_activity_;
            } catch (ClassCastException cce) {
                Log.i("Cardation", "No histogram, fine.");
            }

            String choice = ln.lang_;
            if (choice.equals(Globals.new_language)) {
                if (main_activity != null) {                
                    main_activity.clearHistogram();
                }
                NewLanguagePopup dialog =
                    new NewLanguagePopup(parent_activity_, Globals.new_language,
                                         languages_,
                                         new LanguageAddFunctor());
                dialog.show();
            } else {
                CardDb.setCurrLanguage(choice, parent_activity_);
                if (main_activity != null) {
                    main_activity.showHistogram();
                }
            }
        }

        public void onNothingSelected(AdapterView<?> parent)
        {
            Log.e("Cardation", "nothing selected");
        }
    }


    class LangNumdueComparator implements Comparator<LangNumdue> {
        public int compare(LangNumdue ln1, LangNumdue ln2) {
            if (   ln1.lang_.equals(Globals.new_language)
                && ln2.lang_.equals(Globals.new_language)) {
                return 0;  // To avoid disasters, just in case.
            } else if (ln1.lang_.equals(Globals.new_language)) {
                return 1;
            } else if (ln2.lang_.equals(Globals.new_language)) {
                return -1;
            } else {
                return ln1.lang_.compareTo(ln2.lang_);
            }
        }
    }

    class LanguageAddFunctor implements PopupFunctor {
        public void onPositiveButton(Activity activity, Object extra) {
            String new_lang = (String) extra;
            Log.i("Cardation", "SzpyjnrFanktr.onPositiveButton(), newlang = " +
                  new_lang);
            if (new_lang.contains("_")) {
                Log.w("Cardation", "Languages with underscores are illegal "
                  + "(until I make CardDb.listLanguages() more sophisticated)");
                Toast.makeText(parent_activity_, "No underscores allowed!",
                           Toast.LENGTH_LONG).show();
                return;
            }
            if ((new_lang == null) || (new_lang.equals(""))) {
                if (adapter_.getCount() == 1) { // 1 item -- "New language".
                    System.exit(0);
                }
                return;
            }

            adapter_.add(new LangNumdue(new_lang, 0));
            adapter_.sort(new LangNumdueComparator());

            ConfigDbFields config = new ConfigDbFields(new_lang);
            ConfigDb.update(activity, new_lang, config.rand_reversal_,
                            config.batch_size_, config.initial_streaks_);

            CardDb.createTable(new_lang, activity);
            CardDb.setCurrLanguage(new_lang, activity);
            spinner_.setSelection(adapter_.getPosition(
                new LangNumdue(new_lang,0)));
        }

        public void onNegativeButton(Activity activity) {
            Log.i("Cardation", "LanguageAddFunctor.onNegativeButton()");
            // Gotta make sure spinner doesn't come to rest on "New language".
            if (adapter_.getCount() == 1) { // 1 item -- "New language".
                System.exit(0);
            }
            spinner_.setSelection(adapter_.getPosition(new LangNumdue(
                CardDb.getCurrLanguage(), 0)));
        }
    }
}
