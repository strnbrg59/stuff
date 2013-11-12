package net.trhj.androidflashcards;

import android.os.Bundle;
import android.app.Activity;
import android.widget.TextView;
import android.net.Uri;
import android.content.Intent;
import android.util.Log;
import android.view.View;
import java.io.InputStream;
import java.io.FileNotFoundException;

public class DbbkActivity extends Activity {

    LanguageSpinner language_spinner_;
    TextView info_text_;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_dbbk);

        language_spinner_ = new LanguageSpinner(this);
        info_text_ = (TextView) findViewById(R.id.info);

    }

    public void go(View view)
    {
        /*
         * Grab the attachment
         */

        Intent intent = getIntent();
        Uri intent_data = intent.getData();

        if (! intent.getScheme().equals("content")) {
            Log.e("Cardation", "getScheme() != 'content'");
            info_text_.setText("Error: getScheme() != 'content'");
        }

        InputStream attachment = null;
        try {
            attachment = getContentResolver().openInputStream(intent_data);
        }
        catch (FileNotFoundException e) {
            Log.e("Cardation", "FileNotFoundException");
            info_text_.setText("FileNotFoundException");
            return;
        }

        CardDb.restoreFromInputStream(attachment, this);

        info_text_.setText("Done");
    }
}
