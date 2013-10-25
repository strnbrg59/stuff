package net.trhj.androidflashcards;

import android.os.Bundle;
import android.os.Build;
import android.app.Activity;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.support.v4.app.NavUtils;
import android.annotation.TargetApi;
import android.util.Log;

public class LearnActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_learn);

        Log.w("Flashcards", "Entered LearnActivity.onCreate()");

        setTextSizes();
        advanceCard();
	}

	/**
	 * Set up the {@link android.app.ActionBar}, if the API is available.
	 */
	@SuppressWarnings("unused")
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	private void setupActionBar() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
			getActionBar().setDisplayHomeAsUpEnabled(true);
		}
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			// This ID represents the Home or Up button. In the case of this
			// activity, the Up button is shown. Use NavUtils to allow users
			// to navigate up one level in the application structure. For
			// more details, see the Navigation pattern on Android Design:
			//
			// http://developer.android.com/design/patterns/navigation.html#up-vs-back
			//
			NavUtils.navigateUpFromSameTask(this);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

    DbInteraction.Card curr_card_ = null;

    public void setTextSizes() {
        TextView clue = (TextView) findViewById(R.id.clue);
        clue.setTextSize(clue.getTextSize()*2);
        TextView answer = (TextView) findViewById(R.id.answer);
        answer.setTextSize(answer.getTextSize()*2);
    }

    public void advanceCard() {
        curr_card_ = DbInteraction.getRandomCard(getBaseContext());
        assert(curr_card_ != null);

        TextView clue = (TextView) findViewById(R.id.clue);
        clue.setText(curr_card_.recto_);

        TextView answer = (TextView) findViewById(R.id.answer);
        answer.setText("?");
    }

    /** Called when the user clicks the Next button */
    public void showClue(View view) {
        advanceCard();
    }

    /** Called when the user clicks the Show Answer button */
    public void showAnswer(View view) {
        assert(curr_card_ != null);
        TextView answer = (TextView) findViewById(R.id.answer);
        answer.setText(curr_card_.verso_);
    }
}
