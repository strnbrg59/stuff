package net.trhj.cardation;


import android.app.Activity;
import android.content.*;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MenuInflater;
import android.view.View;
import android.view.ViewConfiguration;
import java.lang.reflect.Field;

public class MainActivity extends Activity
{
    LanguageSpinner language_spinner_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getOverflowMenu();
        language_spinner_ = new LanguageSpinner(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        String curr_lang = CardDb.getCurrLanguage();
        int n_due = CardDb.getNDue();
        Log.i("Cardation", "MainActivity.onResume(), curr_language="
            + curr_lang + ", n_due=" + n_due);
        language_spinner_.updateNumDue(curr_lang, n_due);
    }

    // Forces the three-dots overflow icon to appear.  (It won't, otherwise,
    // if your device has a hardware menu button.)
    private void getOverflowMenu() {
        try {
            ViewConfiguration config = ViewConfiguration.get(this);
            Field menuKeyField =
              ViewConfiguration.class.getDeclaredField("sHasPermanentMenuKey");
            if(menuKeyField != null) {
                menuKeyField.setAccessible(true);
                menuKeyField.setBoolean(config, false);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu items for use in the action bar
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_activity_actions, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle presses on the action bar items
        switch (item.getItemId()) {
            case R.id.action_search:
                Intent intent = new Intent(this, ListActivity.class);
                startActivity(intent);
                return true;

            case R.id.action_config:
                intent = new Intent(this, ConfigActivity.class);
                startActivity(intent);
                return true;

            case R.id.action_stats:
                intent = new Intent(this, StatsActivity.class);
                startActivity(intent);
                return true;

            case R.id.action_backup:
                Popup dialog = new Popup(this, "Confirm backup...",
                                     new BackupPopupFunctor());
                dialog.show();
                return true;

            case R.id.action_restore:
                dialog = new Popup(this,
                    "Really restore " + CardDb.getCurrLanguage() + "?",
                                   new RestorePopupFunctor());
                dialog.show();
                return true;

            /*
            case R.id.action_hack_db:
                intent = new Intent(this, AndroidDatabaseManager.class);
                startActivity(intent);
                return true;
            */
            case R.id.action_delete:
                dialog = new Popup(this,
                    "Really delete " + CardDb.getCurrLanguage() + "?",
                                   new DeletePopupFunctor(language_spinner_));
                dialog.show();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void inputCard(View view) {
        assert(!CardDb.getCurrLanguage().equals(Globals.new_language));
        Intent intent = new Intent(this, InputActivity.class);
        startActivity(intent);
    }

    public void learn(View view) {
        assert(!CardDb.getCurrLanguage().equals(Globals.new_language));
        Intent intent = new Intent(this, LearnActivity.class);
        startActivity(intent);
    }
}
