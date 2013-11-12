package net.trhj.androidflashcards;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.content.Intent;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    /** Called when the user clicks the Input button */
    public void inputCard(View view) {
        Intent intent = new Intent(this, InputActivity.class);
        startActivity(intent);
    }

    /** Called when the user clicks the Learn button */
    public void learn(View view) {
        Intent intent = new Intent(this, LearnActivity.class);
        startActivity(intent);
    }

    public void backup(View view) {
        DbInteraction.backup(getBaseContext());
    }

    public void restore(View view) {
        DbInteraction.restore();
    }    
}
