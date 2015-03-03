//
// This processes email attachments.
//

package net.trhj.cardation;

import android.app.Activity;
import android.content.Intent;
import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ArrayAdapter;
import java.io.InputStream;
import java.io.FileNotFoundException;

public class DbbkActivity extends Activity {

    LanguageSpinner language_spinner_;
    String filename_;
    String language_identified_ = Globals.new_language;
    TextView info_text_;
    InputStream attachment_ = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dbbk);

        Button go_button = (Button) findViewById(R.id.go_button);
        go_button.setEnabled(false);

        language_spinner_ = new LanguageSpinner(this);
        info_text_ = (TextView) findViewById(R.id.info);
        ArrayAdapter<LangNumdue> adapter = language_spinner_.getAdapter();

        identifyAttachment();
        String dialog_msg = "abort abort abort";
        int pos;
        Log.i("Cardation",
              "language_identified_=|" + language_identified_ + "|");
        if (language_identified_ != null) {
            LangNumdue lang_numdue = new LangNumdue(language_identified_, 0);
            pos = adapter.getPosition(lang_numdue);
            Log.i("Cardation", "pos=" + pos);
            if (pos != -1) {
                dialog_msg = "Ready to restore " + language_identified_
                           + ", ok?";
            } else {
                dialog_msg = "You do not currently have a deck for the "
                    + language_identified_ + " language.  Select an existing "
                    + "language or create a new one, ok?";
                pos = 0;
            }
        } else {
            dialog_msg = "Attachment is called " + filename_
                + ", an unfamiliar format for such a file name. "
                + "Set language by hand, if you dare to proceed.  OK?";
                pos = adapter.getPosition(
                    new LangNumdue(Globals.new_language, 0));
        }
        Popup dialog = new Popup(this, dialog_msg,
                   new RestoreFromEmailPopupFunctor(language_spinner_, pos));
        dialog.show();
    }

    public void identifyAttachment()
    {
        /*
         * Grab the attachment
         */

        Intent intent = getIntent();
        Uri uri = intent.getData();

        if (! intent.getScheme().equals("content")) {
            Log.e("Cardation", "getScheme() != 'content'");
            info_text_.setText("Error: getScheme() != 'content'");
        }

        try {
            ContentResolver cr = getContentResolver();
            String[] projection = {MediaStore.MediaColumns.DISPLAY_NAME};
            Cursor metaCursor = cr.query(uri, projection, null, null, null);
            if (metaCursor != null) {
                try {
                   if (metaCursor.moveToFirst()) {
                       filename_ = metaCursor.getString(0);
                       Log.i("Cardation", "Attachment filename_=" + filename_); 
                   }
                } finally {
                    metaCursor.close();
                }
            }

            // Figure out the name of the language.  If the attachment doesn't
            // look like it was made by the backup mechanism (i.e. its name is
            // net_trhj_cardation_<language>_number.db) then warn the user he's
            // on his own to choose the language.
            language_identified_ =
                CardDb.extractLanguageTokenFromDbTableName(filename_);

            attachment_ = cr.openInputStream(uri);
        }
        catch (FileNotFoundException e) {
            Log.e("Cardation", "FileNotFoundException");
            info_text_.setText("FileNotFoundException");
            return;
        }
    }

    public void go(View view) {
        RestoreActivity.restore_source_ = "email";
        RestoreActivity.restore_attachment_ = attachment_;
        // Would be more elegant to pass this information through the Intent.
        Intent intent = new Intent(this, RestoreActivity.class);
        startActivity(intent);
    }

    class RestoreFromEmailPopupFunctor implements PopupFunctor {
        LanguageSpinner spinner_;
        int spinner_pos_;

        RestoreFromEmailPopupFunctor(LanguageSpinner spinner, int pos)
        {
            // Passing those args here because if I call
            // LanguageSpinner.setSelection() in the function that calls this,
            // the new-language dialog pops up before the restore-from-email-
            // popup.
            spinner_ = spinner;
            spinner_pos_ = pos;
        }

        public void onPositiveButton(Activity activity, Object extra) {
            Button go_button = (Button) findViewById(R.id.go_button);
            go_button.setEnabled(true);
            spinner_.setSelection(spinner_pos_);
        }

        public void onNegativeButton(Activity activity) {
            info_text_.setText("Aborted");
            spinner_.setEnabled(false);
        }
    }
}
