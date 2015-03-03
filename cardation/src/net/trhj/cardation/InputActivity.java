package net.trhj.cardation;

import android.os.Bundle;
import android.app.Activity;
import android.text.*;
import android.widget.Toast;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.util.Log;

public class InputActivity extends Activity {

    EditText recto_;
    EditText verso_;
    EditText quote_;
    Button save_button_;
    String recto_str_;
    String verso_str_;
    String quote_str_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_input);

        recto_str_ = verso_str_ = quote_str_ = null;
        recto_ = (EditText) findViewById(R.id.recto);
        verso_ = (EditText) findViewById(R.id.verso);
        quote_ = (EditText) findViewById(R.id.quote);

        // Activate the Save button as soon as there's text in both recto
        // and verso.  Not really necessary, as we elsewhere check this
        // condition before actually saving a new word.  Also, good to have this
        // code as an example.
        save_button_ = (Button) findViewById(R.id.saveButton);
        save_button_.setEnabled(false);
        recto_.addTextChangedListener(new MyTextWatcher(verso_, save_button_));
        verso_.addTextChangedListener(new MyTextWatcher(recto_, save_button_));
    }

    public void onResume() {
        super.onResume();

        Log.i("Cardation", "InputActivity.onResume()");

        if (CardDb.getCurrLanguage() == null) {
            Log.e("Cardation", "InputActivity.onResume(): curr lang = null");
            System.exit(0);
        }
        getActionBar().setTitle("Input " + CardDb.getCurrLanguage());

        if (recto_str_ != null) {
            recto_.setText(recto_str_);
        }
        if (verso_str_ != null) {
            verso_.setText(verso_str_);
        }
        if (quote_str_ != null) {
            quote_.setText(quote_str_);
        }

        ListView listview = (ListView) findViewById(R.id.full_list);
        ListViewCommon.doit(listview, recto_, this, getIntent());
    }

    public void onPause() {
        super.onPause();

        Log.i("Cardation", "InputActivity.onPause()");
        recto_str_ = recto_.getText().toString();        
        verso_str_ = verso_.getText().toString();        
        quote_str_ = quote_.getText().toString();        
    }

    public void onStop() {
        super.onStop();
        Log.i("Cardation", "InputActivity.onStop()");
    }

    /** Called when the user clicks the Save button */
    public void saveCard(View view)
    {
        recto_str_ = recto_.getText().toString();
        recto_str_ = recto_str_.trim();
        verso_str_ = verso_.getText().toString();
        verso_str_ = verso_str_.trim();
        if (recto_str_.matches("") || verso_str_.matches("")) {
            Log.w("Cardation", "Rejected empty or blank string");
            return;
        }
        quote_str_ = quote_.getText().toString();
        quote_str_ = quote_str_.trim();

        CheckBox checkbox = (CheckBox) findViewById(R.id.activeCheckBox);
        int importance;
        if (checkbox.isChecked()) {
            importance = 1;
        } else {
            importance = 0;
        }
        int fwd_streak = LearnActivity.getMinStreak(this);
        int bkwd_streak = LearnActivity.getMinStreak(this);
        int due = CardationUtils.epochNow();

        // Save to DB
        CardDb.Card card =
            new CardDb.Card(recto_str_, verso_str_,
                            fwd_streak, bkwd_streak,
                            due,
                            importance, quote_str_);
        try {
            long newRowId = CardDb.saveCard(getBaseContext(), card);
            Log.w("Flashcards", "newRowId = " + newRowId);

            // Prepare to receive the next card.
            recto_.setText("");
            verso_.setText("");
            quote_.setText("");
            checkbox.setChecked(true);
        }
        // XXX This doesn't catch the exception, dammit.  Why not?!
        catch (Exception e) {
            Toast.makeText(this, "Duplicate -- rejected",
                           Toast.LENGTH_LONG).show();
        }
        Button save_button_ = (Button) findViewById(R.id.saveButton);
        save_button_.setEnabled(false);
    }
}
