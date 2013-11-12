package net.trhj.androidflashcards;

import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.app.Activity;
import android.content.*;

public class MainActivity extends Activity
{
    LanguageSpinner language_spinner_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        language_spinner_ = new LanguageSpinner(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    public void inputCard(View view) {
        invalidLanguageHack();
        Intent intent = new Intent(this, InputActivity.class);
        startActivity(intent);
    }

    public void learn(View view) {
        invalidLanguageHack();
        Intent intent = new Intent(this, LearnActivity.class);
        startActivity(intent);
    }

    public void listAll(View view) {
        invalidLanguageHack();
        Intent intent = new Intent(this, ListActivity.class);
        startActivity(intent);
    }

    public void backup(View view) {
        invalidLanguageHack();
        Popup dialog = new Popup(this, "Really backup?",
                                 new BackupPopupFunctor());
        dialog.show();
    }

    public void restore(View view) {
        invalidLanguageHack();
        Popup dialog = new Popup(this, "Really restore?",
                                 new RestorePopupFunctor());
        dialog.show();
    }

    public void delete(View view) {
        invalidLanguageHack();
        Popup dialog = new Popup(this, "Really delete?",
                                 new DeletePopupFunctor(language_spinner_));
        dialog.show();
    }

    public void config(View view) {
        invalidLanguageHack();
        Intent intent = new Intent(this, ConfigActivity.class);
        startActivity(intent);
    }

    // Have to prevent any kind of activities taking place if the current
    // language is "new language".  This is ugly, but effective.
    void invalidLanguageHack() {
        if (CardDb.getCurrLanguage().equals(Globals.new_language)) {
            throw new RuntimeException("This is a crash");
        }
    }
}
