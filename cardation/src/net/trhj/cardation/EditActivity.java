package net.trhj.cardation;

import android.app.Activity;
import android.content.Intent;
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

        EditText fwd_streak_txt = (EditText) findViewById(R.id.forwardStreak);
        fwd_streak_txt.setText(Integer.toString(curr_card.fwd_streak_));
        EditText bkwd_streak_txt = (EditText)findViewById(R.id.backwardStreak);
        bkwd_streak_txt.setText(Integer.toString(curr_card.bkwd_streak_));
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
        EditText fwd_streak = (EditText) findViewById(R.id.forwardStreak);
        new_card.fwd_streak_ = new Integer(fwd_streak.getText().toString());
        EditText bkwd_streak = (EditText) findViewById(R.id.backwardStreak);
        new_card.bkwd_streak_ = new Integer(bkwd_streak.getText().toString());

        // Reset due-date.  Only if a streak has been edited.  And respecting
        // a soon-to-come due-date on a high-streak card.
        if (   (new_card.fwd_streak_ != curr_card.fwd_streak_)
            || (new_card.bkwd_streak_ != curr_card.bkwd_streak_)) {
            int new_due = CardationUtils.dueDate(new_card.fwd_streak_,
                                                 new_card.bkwd_streak_,
                                                 new_card.importance_==1);
            int old_due = CardationUtils.dueDate(curr_card.fwd_streak_,
                                                 curr_card.bkwd_streak_,
                                                 curr_card.importance_==1);
            Log.i("Cardation", "curr_due=" + curr_card.due_
                + ", new_due=" + new_due + ", old_due=" + old_due
                + ", diff=" + (new_due - old_due));
            new_card.due_ = curr_card.due_ + (new_due - old_due);
        }

        if (new_card.recto_.equals(curr_card.recto_)) {
            CardDb.saveCard(this, new_card);
        } else {
            CardDb.deleteCard(this, curr_card);
            if (new_card.recto_.equals("")) {
                Log.w("Cardation", "deleted " + curr_card.recto_);
                Toast.makeText(this, "Deleted " + curr_card.recto_,
                               Toast.LENGTH_LONG).show();
            } else {
                CardDb.saveCard(this, new_card);
                Log.w("Cardation", "Changed recto from " + curr_card.recto_
                    +" to " + new_card.recto_);
            }
        }

        // Go back to List
        Intent curr_intent = getIntent();
        int list_position = curr_intent.getIntExtra("position", 0);
        Log.d("Cardation", "Returning to position " + list_position);
        Intent next_intent = new Intent(getBaseContext(), ListActivity.class);
        next_intent.putExtra("position", list_position);
        startActivity(next_intent);
    }
}
