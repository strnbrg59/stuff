package net.trhj.androidflashcards;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.CheckBox;
import android.widget.Toast;

public class EditActivity extends Activity {

    static String curr_recto;
    static CardDb.Card curr_card;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_edit);
        Log.w("Cardation", "EditActivity.curr_recto = " + curr_recto);

        curr_card = CardDb.findCard(this, curr_recto);
        EditText recto = (EditText) findViewById(R.id.clue);
        recto.setText(curr_card.recto_);
        EditText verso = (EditText) findViewById(R.id.answer);
        verso.setText(curr_card.verso_);
        EditText quote = (EditText) findViewById(R.id.quote);
        quote.setText(curr_card.quote_);
        CheckBox active = (CheckBox) findViewById(R.id.activeCheckBox);
        active.setChecked(curr_card.importance_ == 1);
    }

    /* Delete if user has deleted recto */
    public void save(View view) {
        CardDb.Card new_card = (CardDb.Card)curr_card.clone();

        EditText recto = (EditText) findViewById(R.id.clue);
        new_card.recto_ = recto.getText().toString();
        EditText verso = (EditText) findViewById(R.id.answer);
        new_card.verso_ = verso.getText().toString();
        EditText quote = (EditText) findViewById(R.id.quote);
        new_card.quote_ = quote.getText().toString();
        CheckBox active = (CheckBox) findViewById(R.id.activeCheckBox);
        new_card.importance_ = active.isChecked() ? 1 : 0;

        if (new_card.recto_.equals(curr_card.recto_)) {
            CardDb.updateByRecto(this, new_card);
        } else {
            CardDb.deleteCard(this, curr_card);
            if (!new_card.recto_.equals("")) {
                CardDb.saveCard(this, new_card);
            } else {
                Log.w("Cardation", "deleted " + curr_card.recto_);
                Toast.makeText(this, "Deleted " + curr_card.recto_,
                               Toast.LENGTH_LONG).show();
            }
            ListActivity.needs_refresh_ = true;
        }
    }
}
