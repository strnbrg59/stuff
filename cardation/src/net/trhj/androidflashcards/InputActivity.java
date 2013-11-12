package net.trhj.androidflashcards;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.widget.Toast;
import android.view.View;
import android.widget.EditText;
import android.widget.CheckBox;
import android.util.Log;

public class InputActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_input);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu); // R.menu.input?
        return true;
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
    }
}
