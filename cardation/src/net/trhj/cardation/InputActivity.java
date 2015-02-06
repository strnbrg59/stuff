package net.trhj.cardation;

import android.os.Bundle;
import android.app.Activity;
import android.text.*;
import android.widget.Toast;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.CheckBox;
import android.util.Log;

public class InputActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_input);
        EditText recto = (EditText) findViewById(R.id.recto);
        EditText verso = (EditText) findViewById(R.id.verso);
        final Button save_button = (Button) findViewById(R.id.saveButton);
        save_button.setEnabled(false);

        getActionBar().setTitle("Input/" + CardDb.getCurrLanguage());

        // Activate the Save button as soon as there's text in both recto
        // and verso.  Not really necessary, as we elsewhere check this
        // condition before actually saving a new word.  Also, good to have this
        // code as an example.
        recto.addTextChangedListener(new MyTextWatcher(verso, save_button));
        verso.addTextChangedListener(new MyTextWatcher(recto, save_button));
    }

    /** Called when the user clicks the Save button */
    public void saveCard(View view)
    {
        EditText recto = (EditText) findViewById(R.id.recto);
        String recto_str = recto.getText().toString();
        recto_str = recto_str.trim();
        EditText verso = (EditText) findViewById(R.id.verso);
        String verso_str = verso.getText().toString();
        verso_str = verso_str.trim();
        if (recto_str.matches("") || verso_str.matches("")) {
            Log.w("Cardation", "Rejected empty or blank string");
            return;
        }
        EditText quote = (EditText) findViewById(R.id.quote);
        String quote_str = quote.getText().toString();
        quote_str = quote_str.trim();

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
            new CardDb.Card(recto_str, verso_str,
                            fwd_streak, bkwd_streak,
                            due,
                            importance, quote_str);
        try {
            long newRowId = CardDb.saveCard(getBaseContext(), card);
            Log.w("Flashcards", "newRowId = " + newRowId);

            // Prepare to receive the next card.
            recto.setText("");
            verso.setText("");
            quote.setText("");
            checkbox.setChecked(true);
        }
        // This doesn't catch the exception, dammit.  Why not?!
        catch (Exception e) {
            Toast.makeText(this, "Duplicate -- rejected",
                           Toast.LENGTH_LONG).show();
        }
        Button save_button = (Button) findViewById(R.id.saveButton);
        save_button.setEnabled(false);
    }
}
