package net.trhj.androidflashcards;

import android.os.Bundle;
import android.os.Build;
import android.app.Activity;
import android.view.MenuItem;
import android.widget.TextView;
import android.support.v4.app.NavUtils;
import android.annotation.TargetApi;
import android.content.Intent;
import android.util.Log;

/* We're not doing anything with this anymore.  But it's a good example for
 * how to use an Intent.
 */
public class SaveCardActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        // Get the message from the intent.
        Intent intent = getIntent();
        String recto = intent.getStringExtra(InputActivity.RECTO);
        String verso = intent.getStringExtra(InputActivity.VERSO);

        // Save to DB
        long newRowId = DbInteraction.saveCard(recto, verso,
                                               getBaseContext());
        Log.w("Flashcards", "newRowId = " + newRowId);

        // Display something.
        TextView textView = new TextView(this);
        textView.setTextSize(40);
        textView.setText("saved, now go back");
        setContentView(textView);
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
}
